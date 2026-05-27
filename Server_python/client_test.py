import socket
import json
import struct

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect(("127.0.0.1", 9999))

def send_msg(msg_type, data):
    req = {"type": msg_type, "data": data}
    json_bytes = json.dumps(req, ensure_ascii=False).encode()
    header = struct.pack('>I', len(json_bytes))
    s.sendall(header + json_bytes)

def recv_msg():
    header = s.recv(4, socket.MSG_WAITALL)
    length = struct.unpack('>I', header)[0]
    body = s.recv(length, socket.MSG_WAITALL)
    return json.loads(body.decode('utf-8'))

# 测试
send_msg("ping", "")
print("连接正常:", recv_msg())

send_msg("message", "帮我整理一下感冒的相关症状")
res = recv_msg()
print("AI 回复:", res["data"])

s.close()