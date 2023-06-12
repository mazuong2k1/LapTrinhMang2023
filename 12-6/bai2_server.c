#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <windows.h>

#define BUFFER_SIZE 1024
#define MAX_CLIENTS 10

// Cấu trúc lưu trữ thông tin client
typedef struct {
    SOCKET socket;
    HANDLE thread;
    int paired;
} ClientInfo;

// Mảng lưu trữ thông tin của các client
ClientInfo clients[MAX_CLIENTS];
CRITICAL_SECTION clientCriticalSection;

// Hàm xử lý yêu cầu từ client
DWORD WINAPI handleClient(LPVOID lpParam) {
    ClientInfo* clientInfo = (ClientInfo*)lpParam;
    SOCKET clientSocket = clientInfo->socket;

    char buffer[BUFFER_SIZE];
    int bytesRead;

    while (1) {
        // Nhận tin nhắn từ client
        memset(buffer, 0, BUFFER_SIZE);
        bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
        if (bytesRead <= 0)
            break;

        // Tìm client đã ghép cặp với client hiện tại
        ClientInfo* pairedClient = NULL;
        EnterCriticalSection(&clientCriticalSection);
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i].socket != INVALID_SOCKET && clients[i].paired && &clients[i] != clientInfo) {
                pairedClient = &clients[i];
                break;
            }
        }
        LeaveCriticalSection(&clientCriticalSection);

        // Gửi tin nhắn cho client đã ghép cặp
        if (pairedClient != NULL) {
            if (send(pairedClient->socket, buffer, bytesRead, 0) == SOCKET_ERROR) {
                printf("Loi gui tin nhan toi client da ghep cap\n");
                break;
            }
        }
    }

    // Đóng kết nối và giải phóng tài nguyên
    closesocket(clientSocket);

    EnterCriticalSection(&clientCriticalSection);
    clientInfo->socket = INVALID_SOCKET;
    clientInfo->paired = 0;
    LeaveCriticalSection(&clientCriticalSection);

    return 0;
}

int main() {
    WSADATA wsa;
    SOCKET serverSocket, clientSocket;
    struct sockaddr_in server, client;
    int clientCount = 0;

    // Khởi tạo Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("Khong the khoi tao thu vien Winsock");
        return 1;
    }

    // Tạo socket server
    if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        printf("Khong the tao socket");
        return 1;
    }

    // Thiết lập thông tin địa chỉ và cổng của server
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(8888);

    // Gắn socket với địa chỉ và cổng
    if (bind(serverSocket, (struct sockaddr*)&server, sizeof(server)) == SOCKET_ERROR) {
        printf("Khong the gan socket voi dia chi va cong");
        return 1;
    }

    // Lắng nghe kết nối từ client
    listen(serverSocket, MAX_CLIENTS);

    // Khởi tạo khóa bảo vệ và client
    InitializeCriticalSection(&clientCriticalSection);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        clients[i].socket = INVALID_SOCKET;
        clients[i].paired = 0;
    }

    printf("Dang cho client ket noi...\n");

    // Chấp nhận kết nối từ client và ghép cặp
    while (1) {
        // Chấp nhận kết nối từ client mới
        int clientSize = sizeof(client);
        clientSocket = accept(serverSocket, (struct sockaddr*)&client, &clientSize);
        if (clientSocket == INVALID_SOCKET) {
            printf("Khong the chap nhan ket noi");
            continue;
        }

        printf("Client da ket noi: %s:%d\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port));

        // Lưu thông tin của client vào mảng
        EnterCriticalSection(&clientCriticalSection);
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i].socket == INVALID_SOCKET) {
                clients[i].socket = clientSocket;
                clients[i].paired = 0;
                break;
            }
        }
        LeaveCriticalSection(&clientCriticalSection);

        // Kiểm tra số lượng client trong hàng đợi
        int pairCount = 0;
        EnterCriticalSection(&clientCriticalSection);
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i].socket != INVALID_SOCKET && !clients[i].paired)
                pairCount++;
        }
        LeaveCriticalSection(&clientCriticalSection);

        // Ghép cặp hai client nếu có đủ số lượng
        if (pairCount >= 2) {
            EnterCriticalSection(&clientCriticalSection);
            int pairedCount = 0;
            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (clients[i].socket != INVALID_SOCKET && !clients[i].paired) {
                    if (pairedCount == 0) {
                        clients[i].paired = 1;
                        pairedCount++;
                    }
                    else if (pairedCount == 1) {
                        clients[i].paired = 1;
                        pairedCount++;

                        // Tạo luồng xử lý cho cặp client mới
                        clients[i].thread = CreateThread(NULL, 0, handleClient, &clients[i], 0, NULL);
                    }
                }
            }
            LeaveCriticalSection(&clientCriticalSection);
        }
    }

    // Đóng socket và thu hồi tài nguyên
    closesocket(serverSocket);
    DeleteCriticalSection(&clientCriticalSection);
    WSACleanup();

    return 0;
}
