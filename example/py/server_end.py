import json
import socket
import threading
from time import sleep
import time
import os
import logging
from logging.handlers import TimedRotatingFileHandler
from logging import StreamHandler
import sqlite3
import random

stop_python = True
threads = []
clients = []

log_path = "./logs"

os.makedirs(log_path, exist_ok=True)

file_handler = TimedRotatingFileHandler(os.path.join(log_path, "service.log"), # 日志文件路径
                                   when="D",#日志切割的时间单位，D表示按天切割
                                   interval=1,#分隔日志的单位 1天
                                   encoding="utf-8",#日志文件的编码
                                   backupCount=7)#最多保留日志的数量

logging.basicConfig(level=logging.INFO, 
                    format='%(asctime)s | %(levelname)s | %(message)s',
                    datefmt='%Y-%m-%d %H:%M:%S',
                    handlers=[file_handler, StreamHandler()])

logger = logging.getLogger(__name__)
 
def read_data(conn,addr):
    try :
        global stop_python, clients
        sql_conn = sqlite3.connect("user.db")
        sql_cursor = sql_conn.cursor()
        while stop_python:
            data = conn.recv(1024).decode()

            str_len = len(data)

            logger.info("Received: %s,len: %d", data, str_len)

            if not data:
                break
            json_data = json.loads(data)
            if json_data["type"] == "message":
                for client in clients:
                    if client!= conn:
                        client.send(str_len.to_bytes(4, byteorder="big"))
                        client.send(data.encode())
                logger.info("Sent to all clients: %s,len: %d", data, str_len)
            elif json_data["type"] == "login":
                user_info = json_data["data"]
                sql_cursor.execute("SELECT username FROM user WHERE userId =? AND password =?", (int(user_info["userId"]), user_info["password"]))
                sql_conn.commit()
                info = sql_cursor.fetchone()
                if info:
                    json_data2 = json.dumps({"type":"login","state":"success","data":info[0]})
                else:
                    json_data2 = json.dumps({"type":"login","state":"failed","data":1})
                
                str_len = len(json_data2)
                conn.send(str_len.to_bytes(4, byteorder="big"))
                conn.send(json_data2.encode())
                logger.info("Login failed: %s,len: %d", user_info, str_len)
            elif json_data["type"] == "register":
                user_info = json_data["data"]

                sql_cursor.execute("SELECT * FROM user WHERE userId =?", (user_info["userId"],))
                sql_conn.commit()

                if sql_cursor.fetchone():
                    json_data2 = json.dumps({"type":"register","state":"failed","data":1})
                else:
                    random.seed(time.time())
                    username = "agent"+str(random.randint(1000000, 9999999))
                    sql_cursor.execute("INSERT INTO user (username, userId, password) VALUES (?, ?, ?)", (username, user_info["userId"], user_info["password"]))
                    sql_conn.commit()
                    json_data2 = json.dumps({"type":"register","state":"success","data":0})
                str_len = len(json_data2)
                conn.send(str_len.to_bytes(4, byteorder="big"))
                conn.send(json_data2.encode())
                logger.info("Register success: %s,len: %d", user_info, str_len)
    except Exception as e:
        logger.error(e)
    finally:
        try:
            conn.close()
            logger.info("Connection closed:%s", addr)
        except Exception as e:
            pass
        clients.remove(conn)
        
def server_init():
    global stop_python, threads, clients
    sql_conn = sqlite3.connect("user.db")
    sql_cursor = sql_conn.cursor()
    sql_cursor.execute("""
                        CREATE TABLE IF NOT EXISTS user (
                            username TEXT,
                            userId INT PRIMARY KEY,
                            password TEXT)
                    """)
    sql_conn.commit()
    sql_cursor.close()
    sql_conn.close()
    sersck = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sersck.bind(("0.0.0.0", 9999))
    sersck.listen(10)
    logger.info("Service started. Waiting for connection...")
    try:
        while True:
            
            conn, addr = sersck.accept()    
            clients.append(conn)
            logger.info("Connected by %s", addr)
            thread = threading.Thread(target=read_data, args=(conn,addr))
            threads.append(thread)
            thread.start()
    except KeyboardInterrupt:
        stop_python = False
        for thread in threads:
            thread.join()
        for client in clients:
            client.close()
        sersck.close()

if __name__ == "__main__":
    server_init()