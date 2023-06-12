#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#define BUFFER_SIZE 1024

int main() {
    WSADATA wsa;
    SOCKET clientSocket;
    struct sockaddr_in server;
    char serverIP[15];
    char request[BUFFER_SIZE];
    char response[BUFFER_SIZE];

    // Khởi tạo Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("Khong the khoi tao thu vien Winsock");
        return 1;
    }

    // Tạo socket kết nối
    if ((clientSocket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        printf("Khong the tao socket");
        return 1;
    }

    // Nhập địa chỉ IP của server
    printf("Nhap dia chi IP cua server: ");
    fgets(serverIP, sizeof(serverIP), stdin);
    serverIP[strcspn(serverIP, "\n")] = 0;

    // Thiết lập thông tin địa chỉ và cổng của server
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(serverIP);
    server.sin_port = htons(8888);

    // Kết nối tới server
    if (connect(clientSocket, (struct sockaddr*)&server, sizeof(server)) < 0) {
        printf("Khong the ket noi den server");
        return 1;
    }

    // Nhận danh sách file từ server
    memset(response, 0, sizeof(response));
    if (recv(clientSocket, response, BUFFER_SIZE, 0) == SOCKET_ERROR) {
        printf("Khong the nhan du lieu tu server");
        closesocket(clientSocket);
        return 1;
    }

    // In danh sách file nhận được từ server
    printf("%s\n", response);

    // Nhập tên file để tải về
    printf("Nhap ten file de tai ve: ");
    fgets(request, sizeof(request), stdin);
    request[strcspn(request, "\n")] = 0;

    // Gửi tên file tới server
    if (send(clientSocket, request, strlen(request), 0) == SOCKET_ERROR) {
        printf("Khong the gui du lieu den server");
        closesocket(clientSocket);
        return 1;
    }

    // Nhận phản hồi từ server
    memset(response, 0, sizeof(response));
    if (recv(clientSocket, response, BUFFER_SIZE, 0) == SOCKET_ERROR) {
        printf("Khong the nhan du lieu tu server");
        closesocket(clientSocket);
        return 1;
    }

    // Xử lý phản hồi từ server
    if (strncmp(response, "ERROR", 5) == 0) {
        printf("Server tra loi: %s\n", response);
    } else if (strncmp(response, "OK", 2) == 0) {
        char* fileSizeStr = strtok(response, " ");
        char* fileSize = strtok(NULL, "\r\n");

        printf("Server tra loi: OK\n");
        printf("Kich thuoc file: %s bytes\n", fileSize);

        // Tạo file mới để lưu nội dung file nhận được từ server
        FILE* file = fopen("downloaded_file.txt", "wb");
        if (file == NULL) {
            printf("Khong the tao file");
            closesocket(clientSocket);
            return 1;
        }

        // Nhận và ghi nội dung file vào file mới
        int totalBytesReceived = 0;
        int bytesRead;
        while (totalBytesReceived < atoi(fileSize)) {
            memset(response, 0, sizeof(response));
            bytesRead = recv(clientSocket, response, BUFFER_SIZE, 0);
            fwrite(response, sizeof(char), bytesRead, file);
            totalBytesReceived += bytesRead;
        }

        printf("Da tai xong file\n");
        fclose(file);
    }

    // Đóng kết nối và thu hồi Winsock
    closesocket(clientSocket);
    WSACleanup();

    return 0;
}
