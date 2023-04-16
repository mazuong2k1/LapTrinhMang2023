import socket
import sys

# Lấy thông tin địa chỉ IP và cổng của server từ tham số dòng lệnh
if len(sys.argv) < 3:
    print("Usage: python sv_client.py <server_ip> <server_port>")
    sys.exit(1)

server_ip = sys.argv[1]
server_port = int(sys.argv[2])

# Tạo socket kết nối đến server
client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
client_socket.connect((server_ip, server_port))

# Nhập thông tin của sinh viên từ bàn phím
mssv = input("Nhap MSSV: ")
ho_ten = input("Nhap ho ten: ")
ngay_sinh = input("Nhap ngay sinh: ")
diem_tb = float(input("Nhap diem trung binh: "))

# Đóng gói thông tin của sinh viên vào một chuỗi và gửi đến server
sv_info = f"{mssv} {ho_ten} {ngay_sinh} {diem_tb}"
client_socket.send(sv_info.encode())

# Đóng kết nối socket
client_socket.close()
