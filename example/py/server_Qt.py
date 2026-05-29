import socket
import struct
import json
import threading

class TcpServer:
    def __init__(self, host='0.0.0.0', port=9999):
        self.host = host
        self.port = port
        self.server_socket = None
        self.running = True

    def start(self):
        """启动服务器"""
        self.server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self.server_socket.bind((self.host, self.port))
        self.server_socket.listen(5)
        print(f"TCP 服务器启动成功：{self.host}:{self.port}")
        print(f"等待客户端连接...")

        while self.running:
            try:
                client_socket, addr = self.server_socket.accept()
                print(f"\n新客户端连接：{addr}")
                # 开启线程处理客户端
                threading.Thread(target=self.handle_client, args=(client_socket, addr), daemon=True).start()
            except Exception as e:
                print(f"服务器异常：{e}")
                break

    def handle_client(self, client_socket, addr):
        """处理单个客户端连接"""
        buffer = b''  # 接收缓冲区

        while True:
            try:
                data = client_socket.recv(4096)
                if not data:
                    print(f"客户端断开：{addr}")
                    break

                buffer += data
                # 循环解析完整数据包（4字节头 + JSON）
                while len(buffer) >= 4:
                    # 读取4字节大端长度头
                    data_len = struct.unpack('>I', buffer[:4])[0]
                    total_len = 4 + data_len

                    if len(buffer) < total_len:
                        break  # 数据不完整，等待下一次接收

                    # 提取完整JSON数据
                    json_data = buffer[4:total_len]
                    buffer = buffer[total_len:]

                    # 解析并打印消息
                    try:
                        msg_dict = json.loads(json_data.decode('utf-8'))
                        print(f"收到 {addr} 消息：{msg_dict}")

                        # 回复客户端（原样返回）
                        self.send_response(client_socket, msg_dict)

                    except json.JSONDecodeError as e:
                        print(f"JSON解析失败：{e}")

            except Exception as e:
                print(f"客户端异常断开 {addr}：{e}")
                break

        client_socket.close()

    def send_response(self, client_socket, msg_dict):
        """发送响应给客户端（同样协议：4字节大端头 + JSON）"""
        try:
            json_str = json.dumps(msg_dict)
            json_bytes = json_str.encode('utf-8')

            # 构造4字节大端长度头
            header = struct.pack('>I', len(json_bytes))
            # 发送：头 + 数据
            client_socket.sendall(header + json_bytes)
            print(f"已回复客户端")
        except Exception as e:
            print(f"发送失败：{e}")

if __name__ == '__main__':
    server = TcpServer(port=9999)
    server.start()