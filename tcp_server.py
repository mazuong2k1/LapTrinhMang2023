import socket
import sys

# Lấy cổng từ tham số dòng lệnh
port = int(sys.argv[1])

# Tạo socket
server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

# Gán địa chỉ và cổng cho socket
server_socket.bind(('', port))

# Lắng nghe kết nối từ client
server_socket.listen(1)

# Chào mừng client
with open(sys.argv[2], 'r') as f:
    welcome_message = f.read()
while True:
    client_socket, client_address = server_socket.accept()
    client_socket.sendall(welcome_message.encode())

    # Ghi nội dung client gửi đến vào file
    with open(sys.argv[3], 'wb') as f:
        while True:
            data = client_socket.recv(1024)
            if not data:
                break
            f.write(data)

    client_socket.close()
