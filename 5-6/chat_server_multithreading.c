#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <windows.h>

#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

struct client {
    SOCKET socket;
    char name[BUFFER_SIZE];
};

DWORD WINAPI ClientThread(LPVOID lpParam);

int main() {
    WSADATA wsaData;
    struct sockaddr_in serverAddress;
    struct client clients[MAX_CLIENTS];
    HANDLE threadHandles[MAX_CLIENTS];
    char buffer[BUFFER_SIZE];
    int numClients = 0;

    // Khởi tạo Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        perror("Khong the khoi tao Winsock");
        exit(EXIT_FAILURE);
    }

    // Tạo socket
    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        perror("Khong the tao socket");
        exit(EXIT_FAILURE);
    }

    // Thiết lập địa chỉ server
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(8080);

    // Gắn socket với địa chỉ server
    if (bind(serverSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR) {
        perror("Khong the gan ket socket voi dia chi");
        closesocket(serverSocket);
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    // Lắng nghe kết nối từ client
    if (listen(serverSocket, 5) == SOCKET_ERROR) {
        perror("Khong the lang nghe ket noi");
        closesocket(serverSocket);
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    printf("Server da san sang\n");

    while (1) {
        // Chấp nhận kết nối mới từ client
        SOCKET clientSocket = accept(serverSocket, NULL, NULL);
        if (clientSocket == INVALID_SOCKET) {
            perror("Khong the chap nhan ket noi");
            closesocket(serverSocket);
            WSACleanup();
            exit(EXIT_FAILURE);
        }

        // Tạo một luồng riêng để xử lý kết nối từ client này
        DWORD threadId;
        threadHandles[numClients] = CreateThread(NULL, 0, ClientThread, (LPVOID)&clients[numClients], 0, &threadId);
        if (threadHandles[numClients] == NULL) {
            perror("Khong the tao luong cho client");
            closesocket(clientSocket);
            closesocket(serverSocket);
            WSACleanup();
            exit(EXIT_FAILURE);
        }

        clients[numClients].socket = clientSocket;
        numClients++;
    }

    // Đóng socket và dọn dẹp Winsock
    closesocket(serverSocket);
    WSACleanup();

    return 0;
}

DWORD WINAPI ClientThread(LPVOID lpParam) {
    struct client *clients = (struct client *)lpParam;
    SOCKET clientSocket = clients->socket;
    char buffer[BUFFER_SIZE];

    // Gửi thông báo cho client về việc chờ nhập tên
    char *message = "Vui long nhap ten cua ban: ";
    send(clientSocket, message, strlen(message), 0);

    int bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
    if (bytesRead <= 0) {
        closesocket(clientSocket);
        return 1;
    }

    buffer[bytesRead] = '\0';

    // Lưu thông tin của client
    strcpy(clients->name, buffer);

    // Gửi thông báo xác nhận tên cho client
    char confirmation[BUFFER_SIZE];
    snprintf(confirmation, sizeof(confirmation), "Xin chao, %s!\n", buffer);
    send(clientSocket, confirmation, strlen(confirmation), 0);

    while (1) {
        bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
        if (bytesRead <= 0) {
            // Kết nối bị đóng hoặc lỗi xảy ra
            closesocket(clientSocket);
            return 1;
        }

        buffer[bytesRead] = '\0';

        // Gửi tin nhắn từ client cho các client khác
        char message[BUFFER_SIZE];
        snprintf(message, sizeof(message), "%s: %s", clients->name, buffer);

        // Gửi tin nhắn đến tất cả các client khác
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i].socket != INVALID_SOCKET && clients[i].socket != clientSocket) {
                send(clients[i].socket, message, strlen(message), 0);
            }
        }
    }

    return 0;
}
