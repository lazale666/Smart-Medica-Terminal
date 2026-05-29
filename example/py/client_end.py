import json
import socket
import threading
from time import sleep

stop_python = True

dict_obj = {"type":None,"data":None}
user_info = {"userId":None,"password":None}

def receive_message(client):
    global stop_python
    while stop_python:
        str_len = int.from_bytes(client.recv(4), byteorder="big")
        data = client.recv(str_len).decode()
        json_data = json.loads(data)
        if json_data["type"] == "register":
            if json_data["state"] == "success":
                dict_obj["type"] = "login"
                dict_obj["data"] = user_info
                user_info["userId"] = 182326
                user_info["password"] = "123456"
                client.send(json.dumps(dict_obj).encode())
            else:
                if dict_obj["data"] == 1:
                    print("Register failed")
        elif json_data["type"] == "login":
            if json_data["state"] == "success":
                print("Login success username:", json_data["data"])
            else:
                if dict_obj["data"] == 1:
                    print("Login failed")
dict_obj["type"] = "register"
dict_obj["data"] = user_info

user_info["userId"] = 182326
user_info["password"] = "123456"

client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
client.connect(('127.0.0.1', 9999))
thread = threading.Thread(target=receive_message, args=(client,))
thread.start()

client.send(json.dumps(dict_obj).encode())

try:
    while True:
        dict_obj["type"] = "message"
        input_data = input("Enter message: ")
        if not input_data:
            break
        dict_obj["data"] = input_data
        client.send(json.dumps(dict_obj).encode())
except KeyboardInterrupt:
    stop_python = False
    thread.join()
    client.close()