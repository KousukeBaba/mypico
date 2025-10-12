import socket
import threading

HOST = "192.168.40.77"  #IP
PORTS = [4242, 4243]    # USER_TASK_MAX=2

#受信データを標準出力して ひたすらエコーバック

def handle_client(conn, addr, port):
    print(f"[Port {port}] Connected by {addr}")
    with conn:
        while True:
            data = conn.recv(2048)
            if not data:
                print(f"[Port {port}] Disconnected")
                break
            print(f"[Port {port}] Received: {data.decode(errors='ignore').strip()}")
            conn.sendall(data)  # エコーバック

def start_server(port):
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        s.bind((HOST, port))
        s.listen(1)
        print(f"[Port {port}] Listening on {HOST}:{port}")
        while True:
            conn, addr = s.accept()
            threading.Thread(target=handle_client, args=(conn, addr, port), daemon=True).start()

if __name__ == "__main__":
    for p in PORTS:
        threading.Thread(target=start_server, args=(p,), daemon=True).start()
    print("Multi-port server running.")
    try:
        while True:
            pass
    except KeyboardInterrupt:
        print("\nServer stopped.")
