import json
import logging
import socket
import sqlite3
import struct
import subprocess
import threading
import time
import uuid

from langchain_ollama import ChatOllama

"""Smart Medica server entry.

Responsibilities:
1. Provide a TCP long-connection server.
2. Manage login/register and online doctor state.
3. Forward plain AI chat requests to the local Ollama model.
4. Build doctor-client sessions and relay messages between them.
"""


logging.basicConfig(
    level=logging.INFO,
    format="%(asctime)s | %(levelname)s | %(message)s",
    datefmt="%Y-%m-%d %H:%M:%S",
)
logger = logging.getLogger(__name__)


ollm = None
model_name = "qwen2.5:7b"
server_running = True
clients = []
doctors = []
clients_lock = threading.Lock()
database_path = "smart_medica.db"

# clients: lightweight records for all connected sockets.
# doctors: full runtime doctor sessions used for allocation and message routing.


def init_database():
    """Initialize SQLite tables.

    Actively used:
    - users

    Reserved for future persistence:
    - sessions
    - messages
    """

    conn = sqlite3.connect(database_path)
    cursor = conn.cursor()

    cursor.execute(
        """CREATE TABLE IF NOT EXISTS users (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        username TEXT UNIQUE NOT NULL,
        password TEXT NOT NULL,
        role TEXT NOT NULL DEFAULT 'client'
    )"""
    )

    cursor.execute(
        """CREATE TABLE IF NOT EXISTS sessions (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        client_id TEXT NOT NULL,
        doctor_id TEXT,
        start_time TEXT NOT NULL,
        end_time TEXT
    )"""
    )

    cursor.execute(
        """CREATE TABLE IF NOT EXISTS messages (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        session_id INTEGER,
        sender TEXT NOT NULL,
        content TEXT NOT NULL,
        timestamp TEXT NOT NULL
    )"""
    )

    conn.commit()
    conn.close()
    logger.info("[OK] Database initialized")


def add_user(username, password, role="client"):
    """Insert a user into the database."""

    conn = None
    try:
        conn = sqlite3.connect(database_path)
        cursor = conn.cursor()
        cursor.execute(
            "INSERT INTO users (username, password, role) VALUES (?, ?, ?)",
            (username, password, role),
        )
        conn.commit()
        return True
    except sqlite3.IntegrityError:
        return False
    finally:
        if conn:
            conn.close()


def verify_user(username, password, role=None):
    """Check whether the given credentials are valid."""

    conn = sqlite3.connect(database_path)
    cursor = conn.cursor()
    if role:
        cursor.execute(
            "SELECT * FROM users WHERE username=? AND password=? AND role=?",
            (username, password, role),
        )
    else:
        cursor.execute(
            "SELECT * FROM users WHERE username=? AND password=?",
            (username, password),
        )
    result = cursor.fetchone()
    conn.close()
    return result is not None


def user_exists(username, role=None):
    """Check whether a user already exists."""

    conn = sqlite3.connect(database_path)
    cursor = conn.cursor()
    if role:
        cursor.execute("SELECT 1 FROM users WHERE username=? AND role=?", (username, role))
    else:
        cursor.execute("SELECT 1 FROM users WHERE username=?", (username,))
    result = cursor.fetchone()
    conn.close()
    return result is not None


def init_ollama():
    """Ensure Ollama service is available and load the chat model."""

    global ollm
    try:
        logger.info("Checking ollama service status...")
        subprocess.check_output(["curl", "-s", "http://localhost:11434"], shell=True)
        logger.info("[OK] Ollama service is running")
    except Exception:
        logger.warning("[ERROR] Ollama service is not running, starting...")
        subprocess.Popen(
            ["ollama", "serve"],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            stdin=subprocess.PIPE,
        )
        time.sleep(5)
        logger.info("[OK] Ollama service started")

    try:
        logger.info(f"Loading AI model: {model_name}...")
        ollm = ChatOllama(model=model_name, temperature=0.7)
        logger.info(f"[OK] AI model '{model_name}' loaded successfully")
    except Exception as exc:
        logger.error(f"[ERROR] AI model loading failed: {exc}")
        raise


def process_with_ollama(message: str) -> str:
    """Run a plain chat message through the local Ollama model."""

    global ollm
    try:
        logger.info("Received user message: %s", message[:100] if len(message) > 100 else message)
        logger.info("[INFO] Model is thinking...")

        response = ollm.invoke(message)
        response_text = response.content if hasattr(response, "content") else str(response)

        logger.info("[OK] Model thinking completed, output length: %d", len(response_text))
        logger.info(
            "AI response content: %s",
            response_text[:200] if len(response_text) > 200 else response_text,
        )
        return response_text
    except Exception as exc:
        logger.error("[ERROR] Model processing error: %s", exc)
        return f"Sorry, an error occurred while processing the message: {str(exc)}"


def send_response(client_socket, msg_dict):
    """Send a length-prefixed JSON packet.

    Packet format:
    - 4-byte big-endian length header
    - UTF-8 JSON payload
    """

    try:
        json_str = json.dumps(msg_dict, ensure_ascii=False)
        json_bytes = json_str.encode("utf-8")
        header = struct.pack(">I", len(json_bytes))
        client_socket.sendall(header + json_bytes)
    except Exception as exc:
        logger.error(f"Failed to send response: {exc}")


def active_client_count(doctor):
    """Count only currently connected patient sockets."""

    return sum(
        1
        for candidate in doctor.get("connected_clients", [])
        if candidate.get("client", {}).get("socket") is not None
    )


def notify_doctor_client_connected(doctor, client_info, session_id, reconnected=False):
    """Tell the doctor UI to create or refresh the patient conversation."""

    send_response(
        doctor["socket"],
        {
            "type": "new_client",
            "client_name": client_info.get("name", "Unknown"),
            "username": client_info.get("name", "Unknown"),
            "session_id": session_id,
            "reconnected": reconnected,
        },
    )


def bind_client_to_doctor(client_info, target_doctor):
    """Create or refresh a doctor-patient session for this patient connection."""

    client_info["current_doctor"] = target_doctor
    existing_session = None

    for candidate in target_doctor.setdefault("connected_clients", []):
        candidate_client = candidate.get("client", {})
        if candidate_client.get("name") == client_info.get("name"):
            existing_session = candidate
            break

    if existing_session is None:
        session_id = str(uuid.uuid4())
        target_doctor["connected_clients"].append(
            {
                "session_id": session_id,
                "client": client_info,
            }
        )
        reconnected = False
    else:
        session_id = existing_session.get("session_id") or str(uuid.uuid4())
        existing_session["session_id"] = session_id
        existing_session["client"] = client_info
        reconnected = True

    client_info["doctor_session_id"] = session_id
    notify_doctor_client_connected(target_doctor, client_info, session_id, reconnected)
    return session_id


def get_online_doctors():
    """Return the current online doctor list."""

    online_doctors = []
    with clients_lock:
        logger.info(f"[DEBUG] get_online_doctors called, doctors count: {len(doctors)}")
        for doctor in doctors:
            if doctor.get("socket") is None:
                continue
            active_clients = active_client_count(doctor)
            online_doctors.append(
                {
                    "id": id(doctor["socket"]),
                    "name": doctor.get("name", "Unknown"),
                    "online": True,
                    "active_clients": active_clients,
                }
            )
            logger.info(
                f"[DEBUG] Adding doctor: {doctor.get('name', 'Unknown')} with {active_clients} active clients"
            )
    logger.info(f"[DEBUG] Returning {len(online_doctors)} online doctors")
    return online_doctors


def handle_client(client_socket, addr):
    """Handle one connected client.

    Core responsibilities:
    - Receive and unpack TCP packets.
    - Route messages by `type`.
    - Maintain doctor-client session state.
    - Clean up runtime relations when the socket closes.
    """

    logger.info(f"New client connected: {addr}")
    buffer = b""
    client_socket.settimeout(120)

    # Runtime state for the current socket.
    client_info = {
        "socket": client_socket,
        "addr": addr,
        "name": None,
        "role": "client",
        "current_doctor": None,
        "connected_clients": [],
    }

    try:
        while server_running:
            try:
                data = client_socket.recv(4096)
                if not data:
                    logger.info(f"Client disconnected: {addr}")
                    break

                buffer += data

                while True:
                    # Wait until the fixed 4-byte header arrives.
                    if len(buffer) < 4:
                        break

                    data_len = struct.unpack(">I", buffer[:4])[0]
                    total_len = 4 + data_len

                    # Wait until the full payload arrives.
                    if len(buffer) < total_len:
                        break

                    json_data = buffer[4:total_len]
                    buffer = buffer[total_len:]

                    try:
                        msg_dict = json.loads(json_data.decode("utf-8"))
                        msg_type = msg_dict.get("type", "unknown")
                        logger.info(f"Received message type from {addr}: {msg_type}")

                        if msg_type == "login":
                            # Login also supports auto-register for first-time users.
                            username = msg_dict.get("username")
                            password = msg_dict.get("password")
                            role = msg_dict.get("role", "client")
                            logger.info(f"[DEBUG] Login attempt: username={username}, role={role}")

                            authenticated = verify_user(username, password, role)
                            if not authenticated and role == "client" and password == "" and username:
                                if not user_exists(username, role):
                                    add_user(username, password, role)
                                    logger.info(f"User {username} auto registered as {role}")
                                authenticated = True
                            elif not authenticated and username and not user_exists(username, role):
                                add_user(username, password, role)
                                logger.info(f"User {username} auto registered as {role}")
                                authenticated = verify_user(username, password, role)

                            if authenticated:
                                client_info["name"] = username
                                client_info["role"] = role
                                client_info["current_client"] = None
                                client_info["current_doctor"] = None
                                client_info["connected_clients"] = []

                                if role == "doctor":
                                    # Logged-in doctors become visible to patients.
                                    with clients_lock:
                                        doctors.append(client_info)
                                    send_response(
                                        client_socket,
                                        {
                                            "type": "login_success",
                                            "message": "Doctor logged in",
                                            "name": username,
                                        },
                                    )
                                    logger.info(f"Doctor {username} logged in, total doctors: {len(doctors)}")
                                else:
                                    send_response(
                                        client_socket,
                                        {
                                            "type": "login_success",
                                            "message": "Client logged in",
                                            "name": username,
                                        },
                                    )
                                    logger.info(f"Client {username} logged in")
                            else:
                                send_response(
                                    client_socket,
                                    {"type": "login_failed", "message": "Invalid credentials"},
                                )
                                logger.warning(f"Login failed for {username}")

                        elif msg_type == "register":
                            username = msg_dict.get("username")
                            password = msg_dict.get("password")
                            role = msg_dict.get("role", "client")

                            if add_user(username, password, role):
                                send_response(
                                    client_socket,
                                    {"type": "register_success", "message": "Registration successful"},
                                )
                            else:
                                send_response(
                                    client_socket,
                                    {"type": "register_failed", "message": "Username already exists"},
                                )

                        elif msg_type == "message":
                            # Plain AI chat request. This path does not use a doctor session.
                            user_message = msg_dict.get("data", "")
                            if not user_message:
                                user_message = msg_dict.get("message", "")
                            logger.info(f"Message content: {user_message[:100]}...")

                            ai_response = process_with_ollama(user_message)
                            send_response(
                                client_socket,
                                {
                                    "type": "ai_response",
                                    "data": ai_response,
                                },
                            )
                            logger.info(f"[OK] Sent AI response to {addr}")

                        elif msg_type == "ping":
                            send_response(client_socket, {"type": "pong", "data": "connected"})

                        elif msg_type == "get_doctors":
                            online_doctors = get_online_doctors()
                            send_response(client_socket, {"type": "doctor_list", "doctors": online_doctors})

                        elif msg_type == "connect_doctor":
                            # Manual allocation: patient chooses a doctor by id(socket).
                            doctor_id = msg_dict.get("doctor_id")
                            request_username = msg_dict.get("username")
                            logger.info(f"[DEBUG] connect_doctor request: doctor_id={doctor_id}")
                            target_doctor = None

                            client_info["role"] = "client"
                            if request_username:
                                client_info["name"] = request_username
                            elif not client_info.get("name"):
                                client_info["name"] = "Unknown"

                            if client_info.get("current_doctor") is not None:
                                send_response(
                                    client_socket,
                                    {
                                        "type": "connection_failed",
                                        "message": "Current session is already connected to a doctor",
                                    },
                                )
                                logger.warning(
                                    f"Client {client_info.get('name')} attempted duplicate doctor connection"
                                )
                                continue

                            with clients_lock:
                                for doctor in doctors:
                                    logger.info(
                                        f"[DEBUG] Checking doctor: id={id(doctor['socket'])}, target={doctor_id}"
                                    )
                                    if id(doctor["socket"]) == int(doctor_id):
                                        target_doctor = doctor
                                        break

                            if target_doctor:
                                session_id = bind_client_to_doctor(client_info, target_doctor)

                                send_response(
                                    client_socket,
                                    {
                                        "type": "connection_success",
                                        "message": "Connected to doctor",
                                        "doctor_name": target_doctor.get("name", "Unknown"),
                                        "session_id": session_id,
                                    },
                                )

                                logger.info(
                                    f"Connected client {client_info.get('name')} with doctor {target_doctor.get('name')}"
                                )
                            else:
                                send_response(
                                    client_socket,
                                    {"type": "connection_failed", "message": "Doctor not available"},
                                )
                                logger.warning(f"Doctor not found: {doctor_id}")

                        elif msg_type == "request_doctor":
                            # Auto allocation: choose the least-loaded online doctor.
                            request_username = msg_dict.get("username")
                            client_info["role"] = "client"
                            if request_username:
                                client_info["name"] = request_username
                            elif not client_info.get("name"):
                                client_info["name"] = "Unknown"
                            logger.info(
                                f"[DEBUG] Client {client_info.get('name')} requesting doctor connection"
                            )
                            target_doctor = None

                            with clients_lock:
                                if doctors:
                                    target_doctor = min(
                                        doctors,
                                        key=active_client_count,
                                    )

                            if target_doctor:
                                session_id = bind_client_to_doctor(client_info, target_doctor)

                                send_response(
                                    client_socket,
                                    {
                                        "type": "connection_success",
                                        "message": "Connected to doctor",
                                        "doctor_name": target_doctor.get("name", "Unknown"),
                                        "session_id": session_id,
                                    },
                                )

                                logger.info(
                                    f"Auto-connected client {client_info.get('name')} with doctor {target_doctor.get('name')}"
                                )
                            else:
                                send_response(
                                    client_socket,
                                    {
                                        "type": "waiting_for_doctor",
                                        "message": "No doctors available, please wait",
                                    },
                                )
                                logger.info(f"No doctors available for client {client_info.get('name')}")

                        elif msg_type == "doctor_message":
                            # Shared doctor-chat channel:
                            # - patient -> current doctor
                            # - doctor -> target patient session
                            if client_info["role"] == "client" and client_info.get("current_doctor"):
                                message = msg_dict.get("message")
                                session_id = msg_dict.get("session_id") or client_info.get("doctor_session_id", "")
                                if session_id != client_info.get("doctor_session_id", ""):
                                    send_response(
                                        client_socket,
                                        {
                                            "type": "connection_failed",
                                            "message": "Session expired, please reconnect to the doctor",
                                        },
                                    )
                                    logger.warning(
                                        f"Client {client_info.get('name')} sent message with stale session: {session_id}"
                                    )
                                    continue

                                send_response(
                                    client_info["current_doctor"]["socket"],
                                    {
                                        "type": "client_message",
                                        "sender": client_info.get("name", "client"),
                                        "client_name": client_info.get("name", "client"),
                                        "session_id": session_id,
                                        "message": message,
                                    },
                                )
                                logger.info(
                                    f"Forwarded message from client {client_info.get('name')} to doctor"
                                )
                            elif client_info["role"] == "doctor":
                                message = msg_dict.get("message")
                                target_session_id = msg_dict.get("target_session_id")
                                target_client_name = msg_dict.get("target_client") or msg_dict.get("client_name")
                                target_client = None

                                # Doctors reply by session id to avoid routing the message to the wrong client.
                                for candidate in client_info.get("connected_clients", []):
                                    if candidate.get("session_id") == target_session_id:
                                        target_client = candidate.get("client")
                                        break

                                if target_client is None or target_client.get("socket") is None:
                                    send_response(
                                        client_socket,
                                        {
                                            "type": "connection_failed",
                                            "message": "Target client not available",
                                            "target_client": target_client_name,
                                            "target_session_id": target_session_id,
                                        },
                                    )
                                    logger.warning(
                                        f"Doctor {client_info.get('name')} attempted to reply to unavailable session: {target_session_id}"
                                    )
                                    continue

                                send_response(
                                    target_client["socket"],
                                    {
                                        "type": "client_message",
                                        "sender": client_info.get("name", "doctor"),
                                        "doctor_name": client_info.get("name", "doctor"),
                                        "session_id": target_session_id,
                                        "message": message,
                                    },
                                )
                                logger.info(
                                    f"Forwarded message from doctor {client_info.get('name')} to session {target_session_id}"
                                )
                            else:
                                send_response(
                                    client_socket,
                                    {
                                        "type": "connection_failed",
                                        "message": "No active doctor-client session",
                                    },
                                )
                                logger.warning(
                                    f"Message dropped because no active session exists for {client_info.get('name')}"
                                )

                    except json.JSONDecodeError as exc:
                        logger.error(f"JSON parsing failed: {exc}")
                    except Exception as exc:
                        logger.error(f"Message processing error: {exc}")

            except socket.timeout:
                continue

    except Exception as exc:
        logger.error(f"Client exception {addr}: {exc}")
    finally:
        # Keep both ends in sync when either patient or doctor disconnects.
        with clients_lock:
            if client_info.get("role") == "client" and client_info.get("current_doctor"):
                doctor_info = client_info["current_doctor"]
                for candidate in doctor_info.get("connected_clients", []):
                    candidate_client = candidate.get("client", {})
                    if candidate_client.get("socket") is client_socket:
                        candidate_client["socket"] = None
                        candidate_client["current_doctor"] = None
                        break
                send_response(
                    doctor_info["socket"],
                    {
                        "type": "client_disconnected",
                        "client_name": client_info.get("name", "Unknown"),
                        "username": client_info.get("name", "Unknown"),
                        "session_id": client_info.get("doctor_session_id", ""),
                    },
                )
            elif client_info.get("role") == "doctor":
                for connected_client in list(client_info.get("connected_clients", [])):
                    connected_client["client"]["current_doctor"] = None
                    target_socket = connected_client["client"].get("socket")
                    if target_socket is not None:
                        send_response(
                            target_socket,
                            {
                                "type": "doctor_disconnected",
                                "doctor_name": client_info.get("name", "Unknown"),
                                "message": "Doctor disconnected",
                                "session_id": connected_client.get("session_id", ""),
                            },
                        )
                client_info["connected_clients"].clear()

            clients[:] = [client for client in clients if client.get("socket") is not client_socket]
            if client_info in doctors:
                doctors.remove(client_info)
                logger.info(
                    f"Doctor {client_info.get('name')} disconnected, remaining doctors: {len(doctors)}"
                )

        try:
            client_socket.close()
            logger.info(f"Connection closed: {addr}")
        except Exception:
            pass


def start_server(host="0.0.0.0", port=9999):
    """Start the TCP server and accept incoming connections."""

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
                    clients.append({"socket": client_socket, "addr": addr})

                # One thread per connection; the main thread keeps accepting new sockets.
                threading.Thread(
                    target=handle_client,
                    args=(client_socket, addr),
                    daemon=True,
                ).start()
            except Exception as exc:
                logger.error(f"Accept error: {exc}")
                continue

    except KeyboardInterrupt:
        logger.info("\nShutting down server...")
        server_running = False
    finally:
        server_running = False
        try:
            server_socket.close()
        except Exception:
            pass

        with clients_lock:
            for client_info in clients:
                try:
                    client_info["socket"].close()
                except Exception:
                    pass
            for doctor in doctors:
                try:
                    doctor["socket"].close()
                except Exception:
                    pass

        logger.info("[OK] Server closed")


if __name__ == "__main__":
    start_server(port=9999)
