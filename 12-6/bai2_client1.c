#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#define BUFFER_SIZE 1024

int main() {
    WSADATA wsa;
    SOCKET clientSocket;
    struct sockaddr_in server;
    char serverIP[16];

    // Khởi tạo Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("Khong the khoi tao thu vien Winsock");
        return 1;
    }

    // Tạo socket client
    if ((clientSocket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        printf("Khong the tao socket");
        return 1;
    }

    printf("Nhap dia chi IP cua server: ");
    scanf("%s", serverIP);

    // Thiết lập thông tin địa chỉ và cổng của server
    server.sin_family = AF_INET;
    server.sin_port = htons(8888);
    server.sin_addr.s_addr = inet_addr(serverIP);

    // Kết nối tới server
    if (connect(clientSocket, (struct sockaddr*)&server, sizeof(server)) < 0) {
        printf("Khong the ket noi toi server");
        return 1;
    }

    printf("Da ket noi toi server\n");

    char buffer[BUFFER_SIZE];
    int bytesRead;

    while (1) {
        // Nhập tin nhắn từ người dùng
        memset(buffer, 0, BUFFER_SIZE);
        printf("Nhap tin nhan: ");
        fgets(buffer, BUFFER_SIZE, stdin);

        // Gửi tin nhắn tới server
        if (send(clientSocket, buffer, strlen(buffer), 0) == SOCKET_ERROR) {
            printf("Loi gui tin nhan");
            break;
        }
    }

    // Đóng kết nối và thu hồi tài nguyên
    closesocket(clientSocket);
    WSACleanup();

    return 0;
}
