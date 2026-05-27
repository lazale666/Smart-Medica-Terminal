import json
import socket
import struct
import threading
import logging
import subprocess
import time
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
clients_lock = threading.Lock()


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


def handle_client(client_socket, addr):
    """Handle single client connection"""
    logger.info(f"New client connected: {addr}")
    buffer = b""
    client_socket.settimeout(120)

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
                        logger.info(f"Received message type from {addr}: {msg_dict.get('type', 'unknown')}")

                        if msg_dict.get('type') == 'message':
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
            if client_socket in clients:
                clients.remove(client_socket)
        try:
            client_socket.close()
            logger.info(f"Connection closed: {addr}")
        except:
            pass


def send_response(client_socket, msg_dict):
    """Send response to client"""
    try:
        json_str = json.dumps(msg_dict, ensure_ascii=False)
        json_bytes = json_str.encode('utf-8')
        header = struct.pack('>I', len(json_bytes))
        client_socket.sendall(header + json_bytes)
    except Exception as e:
        logger.error(f"Failed to send response: {e}")


def start_server(host='0.0.0.0', port=9999):
    """Start TCP server"""
    global server_running

    logger.info("=" * 60)
    logger.info("AI Chat Server Starting...")
    logger.info("=" * 60)

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
                    clients.append(client_socket)
                # ✅ 终极修复：Windows 必崩元凶，必须加 daemon=True
                threading.Thread(target=handle_client, args=(client_socket, addr), daemon=True).start()
            except Exception as e:
                # ✅ 修复：accept 报错不退出服务器
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
            for client in clients:
                try:
                    client.close()
                except:
                    pass
        logger.info("[OK] Server closed")


if __name__ == "__main__":
    start_server(port=9999)