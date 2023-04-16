import socket
import sys

# Lấy địa chỉ IP và cổng từ tham số dòng lệnh
ip_address = sys.argv[1]
port = int(sys.argv[2])

# Tạo socket
client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

# Kết nối tới server
client_socket.connect((ip_address, port))

# Nhập dữ liệu từ bàn phím và gửi tới server
while True:
    message = input("Enter message: ")
    client_socket.sendall(message.encode())

# Đóng socket
client_socket.close()
