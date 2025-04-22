import socket

def serve_firmware():
    host = '192.168.1.146'  # 任意可接受的 IP 地址
    port = 8080        # 伺服器端口

    # 創建 TCP 套接字
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as server_socket:
        server_socket.bind((host, port))
        server_socket.listen(1)
        print(f"Server listening on {host}:{port}...")

        # 等待客戶端連接
        conn, addr = server_socket.accept()
        with conn:
            print(f"Connected by {addr}")
            try:
                # 打開固件檔案並傳送給客戶端
                with open('main.bin', 'rb') as f:
                    while chunk := f.read(1024):  # 每次傳送 1KB
                        try:
                            conn.sendall(chunk)
                        except BrokenPipeError:
                            print("Client disconnected, stopping file transfer.")
                            break  # 停止傳送
            except Exception as e:
                print(f"Error during file transfer: {e}")
            else:
                print("Firmware sent successfully!")

if __name__ == '__main__':
    serve_firmware()
