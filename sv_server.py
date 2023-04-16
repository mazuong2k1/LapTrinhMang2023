import socket
import sys
import datetime

# Lấy thông tin cổng và tên file log từ tham số dòng lệnh
if len(sys.argv) < 3:
    print("Usage: python sv_server.py <server_port> <log_file>")
    sys.exit(1)

server_port = int(sys.argv[1])
log_file = sys.argv[2]

# Tạo socket server
server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server_socket.bind(("localhost", server_port))
server_socket.listen(1)

print("Server is running at port", server_port)

while True:
    # Chấp nhận kết nối từ client
    client_socket, client_address = server_socket.accept()
    
    # Nhận thông tin của sinh viên từ client
    sv_info = client_socket.recv(1024).decode().strip()
    
    # In thông tin của sinh viên ra màn hình và ghi vào file log
    now = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    log_line = f"{client_address[0]} {now} {sv_info}"
    print(log_line)
    
    with open(log_file, "a") as f:
        f.write(log_line + "\n")
    
    # Đóng kết nối socket
    client_socket.close()
