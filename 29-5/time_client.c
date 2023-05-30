#include <stdio.h>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8888
#define BUFFER_SIZE 1024

int main() {
    WSADATA wsaData;
    SOCKET clientSocket;
    struct sockaddr_in serverAddr;
    char buffer[BUFFER_SIZE];

    // Khởi tạo Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("Khong the khoi tao Winsock");
        return 1;
    }

    // Tạo socket
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        printf("Khong the tao socket");
        WSACleanup();
        return 1;
    }

    // Thiết lập địa chỉ server
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP);

    // Kết nối tới server
    if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        printf("Khong the ket noi toi server");
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    // Gửi lệnh GET_TIME đến server
    char command[] = "GET_TIME dd/mm/yyyy";
    send(clientSocket, command, strlen(command), 0);

    // Nhận thời gian từ server và in ra màn hình
    int ret = recv(clientSocket, buffer, BUFFER_SIZE - 1, 0);
    if (ret == SOCKET_ERROR || ret == 0) {
        printf("Khong the nhan thoi gian tu server");
    } else {
        buffer[ret] = '\0';
        printf("Thoi gian hien tai: %s\n", buffer);
    }

    // Đóng kết nối và dọn dẹp Winsock
    closesocket(clientSocket);
    WSACleanup();

    return 0;
}
