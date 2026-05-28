import json
import socket
import struct
import threading
import logging
import subprocess
import time
import sqlite3
import os
from langchain_ollama import ChatOllama

logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s | %(levelname)s | %(message)s',
    datefmt='%Y-%m-%d %H:%M:%S'
)
logger = logging.getLogger(__name__)

ollm = None
model_name = "qwen2.5:7b"
server_running = True
clients = []
doctors = []
clients_lock = threading.Lock()
database_path = "smart_medica.db"

def init_database():
    """Initialize SQLite database"""
    conn = sqlite3.connect(database_path)
    cursor = conn.cursor()
    
    cursor.execute('''CREATE TABLE IF NOT EXISTS users (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        username TEXT UNIQUE NOT NULL,
        password TEXT NOT NULL,
        role TEXT NOT NULL DEFAULT 'client'
    )''')
    
    cursor.execute('''CREATE TABLE IF NOT EXISTS sessions (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        client_id TEXT NOT NULL,
        doctor_id TEXT,
        start_time TEXT NOT NULL,
        end_time TEXT
    )''')
    
    cursor.execute('''CREATE TABLE IF NOT EXISTS messages (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        session_id INTEGER,
        sender TEXT NOT NULL,
        content TEXT NOT NULL,
        timestamp TEXT NOT NULL
    )''')
    
    conn.commit()
    conn.close()
    logger.info("[OK] Database initialized")

def add_user(username, password, role='client'):
    """Add user to database"""
    try:
        conn = sqlite3.connect(database_path)
        cursor = conn.cursor()
        cursor.execute('INSERT INTO users (username, password, role) VALUES (?, ?, ?)',
                      (username, password, role))
        conn.commit()
        conn.close()
        return True
    except sqlite3.IntegrityError:
        conn.close()
        return False

def verify_user(username, password, role=None):
    """Verify user credentials"""
    conn = sqlite3.connect(database_path)
    cursor = conn.cursor()
    if role:
        cursor.execute('SELECT * FROM users WHERE username=? AND password=? AND role=?',
                      (username, password, role))
    else:
        cursor.execute('SELECT * FROM users WHERE username=? AND password=?',
                      (username, password))
    result = cursor.fetchone()
    conn.close()
    return result is not None

def user_exists(username, role=None):
    """Check whether a user already exists."""
    conn = sqlite3.connect(database_path)
    cursor = conn.cursor()
    if role:
        cursor.execute('SELECT 1 FROM users WHERE username=? AND role=?', (username, role))
    else:
        cursor.execute('SELECT 1 FROM users WHERE username=?', (username,))
    result = cursor.fetchone()
    conn.close()
    return result is not None

def init_ollama():
    """Initialize ollama model"""
    global ollm
    try:
        logger.info("Checking ollama service status...")
        subprocess.check_output(['curl', '-s', "http://localhost:11434"], shell=True)
        logger.info("[OK] Ollama service is running")
    except:
        logger.warning("[ERROR] Ollama service is not running, starting...")
        subprocess.Popen(['ollama', 'serve'],
                        stdout=subprocess.PIPE,
                        stderr=subprocess.PIPE,
                        stdin=subprocess.PIPE)
        time.sleep(5)
        logger.info("[OK] Ollama service started")

    try:
        logger.info(f"Loading AI model: {model_name}...")
        ollm = ChatOllama(model=model_name, temperature=0.7)
        logger.info(f"[OK] AI model '{model_name}' loaded successfully")
    except Exception as e:
        logger.error(f"[ERROR] AI model loading failed: {e}")
        raise

def process_with_ollama(message: str) -> str:
    """Process message with ollama model"""
    global ollm
    try:
        logger.info("Received user message: %s", message[:100] if len(message) > 100 else message)

        logger.info("[INFO] Model is thinking...")
        response = ollm.invoke(message)
        response_text = response.content if hasattr(response, 'content') else str(response)

        logger.info("[OK] Model thinking completed, output length: %d", len(response_text))
        logger.info("AI response content: %s", response_text[:200] if len(response_text) > 200 else response_text)

        return response_text
    except Exception as e:
        logger.error("[ERROR] Model processing error: %s", e)
        return f"Sorry, an error occurred while processing the message: {str(e)}"

def send_response(client_socket, msg_dict):
    """Send response to client"""
    try:
        json_str = json.dumps(msg_dict, ensure_ascii=False)
        json_bytes = json_str.encode('utf-8')
        header = struct.pack('>I', len(json_bytes))
        client_socket.sendall(header + json_bytes)
    except Exception as e:
        logger.error(f"Failed to send response: {e}")

def get_online_doctors():
    """Get list of online doctors"""
    online_doctors = []
    with clients_lock:
        logger.info(f"[DEBUG] get_online_doctors called, doctors count: {len(doctors)}")
        for doctor in doctors:
            if doctor.get('socket') is None:
                continue
            online_doctors.append({
                'id': id(doctor['socket']),
                'name': doctor.get('name', 'Unknown'),
                'online': True
            })
            logger.info(f"[DEBUG] Adding doctor: {doctor.get('name', 'Unknown')}")
    logger.info(f"[DEBUG] Returning {len(online_doctors)} online doctors")
    return online_doctors

def handle_client(client_socket, addr):
    """Handle single client connection"""
    logger.info(f"New client connected: {addr}")
    buffer = b""
    client_socket.settimeout(120)
    client_info = {'socket': client_socket, 'addr': addr, 'name': None, 'role': 'client'}
    current_doctor = None
    current_client = None

    try:
        while server_running:
            try:
                data = client_socket.recv(4096)
                if not data:
                    logger.info(f"Client disconnected: {addr}")
                    break

                buffer += data

                while True:
                    if len(buffer) < 4:
                        break
                    data_len = struct.unpack('>I', buffer[:4])[0]
                    total_len = 4 + data_len

                    if len(buffer) < total_len:
                        break

                    json_data = buffer[4:total_len]
                    buffer = buffer[total_len:]

                    try:
                        msg_dict = json.loads(json_data.decode('utf-8'))
                        msg_type = msg_dict.get('type', 'unknown')
                        logger.info(f"Received message type from {addr}: {msg_type}")

                        if msg_dict.get('type') == 'login':
                            username = msg_dict.get('username')
                            password = msg_dict.get('password')
                            role = msg_dict.get('role', 'client')
                            logger.info(f"[DEBUG] Login attempt: username={username}, role={role}")
                            
                            authenticated = verify_user(username, password, role)
                            if not authenticated and role == 'client' and password == '' and username:
                                if not user_exists(username, role):
                                    add_user(username, password, role)
                                    logger.info(f"User {username} auto registered as {role}")
                                authenticated = True
                            elif not authenticated and username and not user_exists(username, role):
                                add_user(username, password, role)
                                logger.info(f"User {username} auto registered as {role}")
                                authenticated = verify_user(username, password, role)

                            if authenticated:
                                client_info['name'] = username
                                client_info['role'] = role
                                client_info['current_client'] = None
                                client_info['current_doctor'] = None
                                
                                if role == 'doctor':
                                    with clients_lock:
                                        doctors.append(client_info)
                                    send_response(client_socket, {'type': 'login_success', 'message': 'Doctor logged in', 'name': username})
                                    logger.info(f"Doctor {username} logged in, total doctors: {len(doctors)}")
                                else:
                                    send_response(client_socket, {'type': 'login_success', 'message': 'Client logged in', 'name': username})
                                    logger.info(f"Client {username} logged in")
                            else:
                                send_response(client_socket, {'type': 'login_failed', 'message': 'Invalid credentials'})
                                logger.warning(f"Login failed for {username}")

                        elif msg_dict.get('type') == 'register':
                            username = msg_dict.get('username')
                            password = msg_dict.get('password')
                            role = msg_dict.get('role', 'client')
                            
                            if add_user(username, password, role):
                                send_response(client_socket, {'type': 'register_success', 'message': 'Registration successful'})
                            else:
                                send_response(client_socket, {'type': 'register_failed', 'message': 'Username already exists'})

                        elif msg_dict.get('type') == 'message':
                            user_message = msg_dict.get('data', '')
                            if not user_message:
                                user_message = msg_dict.get('message', '')
                            logger.info(f"Message content: {user_message[:100]}...")

                            ai_response = process_with_ollama(user_message)

                            response_dict = {
                                "type": "ai_response",
                                "data": ai_response
                            }
                            send_response(client_socket, response_dict)
                            logger.info(f"[OK] Sent AI response to {addr}")

                        elif msg_dict.get('type') == 'ping':
                            response_dict = {"type": "pong", "data": "connected"}
                            send_response(client_socket, response_dict)

                        elif msg_dict.get('type') == 'get_doctors':
                            logger.info(f"[DEBUG] get_doctors request received from {addr}")
                            online_doctors = get_online_doctors()
                            send_response(client_socket, {'type': 'doctor_list', 'doctors': online_doctors})

                        elif msg_dict.get('type') == 'connect_doctor':
                            doctor_id = msg_dict.get('doctor_id')
                            logger.info(f"[DEBUG] connect_doctor request: doctor_id={doctor_id}")
                            target_doctor = None
                            
                            with clients_lock:
                                for doctor in doctors:
                                    logger.info(f"[DEBUG] Checking doctor: id={id(doctor['socket'])}, target={doctor_id}")
                                    if id(doctor['socket']) == int(doctor_id):
                                        target_doctor = doctor
                                        break
                            
                            if target_doctor:
                                current_doctor = target_doctor
                                client_info['current_doctor'] = target_doctor
                                target_doctor['current_client'] = client_info
                                
                                send_response(client_socket, {
                                    'type': 'connection_success',
                                    'message': 'Connected to doctor',
                                    'doctor_name': target_doctor.get('name', 'Unknown')
                                })
                                send_response(target_doctor['socket'], {
                                    'type': 'new_client',
                                    'client_name': client_info.get('name', 'Unknown')
                                })
                                logger.info(f"Connected client {client_info.get('name')} with doctor {target_doctor.get('name')}")
                            else:
                                send_response(client_socket, {'type': 'connection_failed', 'message': 'Doctor not available'})
                                logger.warning(f"Doctor not found: {doctor_id}")

                        elif msg_dict.get('type') == 'request_doctor':
                            request_username = msg_dict.get('username')
                            client_info['role'] = 'client'
                            if request_username:
                                client_info['name'] = request_username
                            elif not client_info.get('name'):
                                client_info['name'] = 'Unknown'
                            logger.info(f"[DEBUG] Client {client_info.get('name')} requesting doctor connection")
                            target_doctor = None
                            
                            with clients_lock:
                                for doctor in doctors:
                                    if doctor.get('current_client') is None:
                                        target_doctor = doctor
                                        break
                            
                            if target_doctor:
                                client_info['current_doctor'] = target_doctor
                                target_doctor['current_client'] = client_info
                                
                                send_response(client_socket, {
                                    'type': 'connection_success',
                                    'message': 'Connected to doctor',
                                    'doctor_name': target_doctor.get('name', 'Unknown')
                                })
                                send_response(target_doctor['socket'], {
                                    'type': 'new_client',
                                    'client_name': client_info.get('name', 'Unknown')
                                })
                                logger.info(f"Auto-connected client {client_info.get('name')} with doctor {target_doctor.get('name')}")
                            else:
                                send_response(client_socket, {'type': 'waiting_for_doctor', 'message': 'No doctors available, please wait'})
                                logger.info(f"No doctors available for client {client_info.get('name')}")

                        elif msg_dict.get('type') == 'doctor_message':
                            if client_info['role'] == 'client' and client_info.get('current_doctor'):
                                message = msg_dict.get('message')
                                send_response(client_info['current_doctor']['socket'], {
                                    'type': 'client_message',
                                    'sender': client_info.get('name', 'client'),
                                    'message': message
                                })
                                logger.info(f"Forwarded message from client {client_info.get('name')} to doctor")
                            elif client_info['role'] == 'doctor' and client_info.get('current_client'):
                                message = msg_dict.get('message')
                                send_response(client_info['current_client']['socket'], {
                                    'type': 'client_message',
                                    'sender': client_info.get('name', 'doctor'),
                                    'message': message
                                })
                                logger.info(f"Forwarded message from doctor {client_info.get('name')} to client")
                            else:
                                send_response(client_socket, {
                                    'type': 'connection_failed',
                                    'message': 'No active doctor-client session'
                                })
                                logger.warning(f"Message dropped because no active session exists for {client_info.get('name')}")

                    except json.JSONDecodeError as e:
                        logger.error(f"JSON parsing failed: {e}")
                    except Exception as e:
                        logger.error(f"Message processing error: {e}")

            except socket.timeout:
                continue

    except Exception as e:
        logger.error(f"Client exception {addr}: {e}")
    finally:
        with clients_lock:
            if client_info.get('role') == 'client' and client_info.get('current_doctor'):
                client_info['current_doctor']['current_client'] = None
            elif client_info.get('role') == 'doctor' and client_info.get('current_client'):
                client_info['current_client']['current_doctor'] = None
            
            clients[:] = [c for c in clients if c.get('socket') is not client_socket]
            if client_info in doctors:
                doctors.remove(client_info)
                logger.info(f"Doctor {client_info.get('name')} disconnected, remaining doctors: {len(doctors)}")
        try:
            client_socket.close()
            logger.info(f"Connection closed: {addr}")
        except:
            pass

def start_server(host='0.0.0.0', port=9999):
    """Start TCP server"""
    global server_running

    logger.info("=" * 60)
    logger.info("AI Chat Server Starting...")
    logger.info("=" * 60)

    init_database()
    init_ollama()

    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

    try:
        server_socket.bind((host, port))
        server_socket.listen(10)
        logger.info(f"[OK] Server listening on: {host}:{port}")
        logger.info("=" * 60)
        logger.info("Waiting for client connections...")
        logger.info("=" * 60)

        while server_running:
            try:
                client_socket, addr = server_socket.accept()
                with clients_lock:
                    clients.append({'socket': client_socket, 'addr': addr})
                threading.Thread(target=handle_client, args=(client_socket, addr), daemon=True).start()
            except Exception as e:
                logger.error(f"Accept error: {e}")
                continue

    except KeyboardInterrupt:
        logger.info("\nShutting down server...")
        server_running = False
    finally:
        server_running = False
        try:
            server_socket.close()
        except:
            pass
        with clients_lock:
            for client_info in clients:
                try:
                    client_info['socket'].close()
                except:
                    pass
            for doctor in doctors:
                try:
                    doctor['socket'].close()
                except:
                    pass
        logger.info("[OK] Server closed")

if __name__ == "__main__":
    start_server(port=9999)
