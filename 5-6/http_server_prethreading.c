#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <process.h>

#define PORT 8081
#define NUM_THREADS 5

void handle_connection(void* client) {
    SOCKET clientSocket = *(SOCKET*)client;
    char buf[256];
    int ret = recv(clientSocket, buf, sizeof(buf), 0);
    buf[ret] = 0;
    puts(buf);

    char* msg = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<html><body><h1>Xin chao cac ban</h1></body></html>";
    send(clientSocket, msg, strlen(msg), 0);

    closesocket(clientSocket);
    free(client);
    _endthread();
}

int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        perror("Failed to initialize Winsock");
        exit(EXIT_FAILURE);
    }

    SOCKET listener = socket(AF_INET, SOCK_STREAM, 0);
    if (listener == INVALID_SOCKET) {
        perror("Failed to create socket");
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(PORT);

    if (bind(listener, (struct sockaddr*)&server_address, sizeof(server_address)) == SOCKET_ERROR) {
        perror("Failed to bind socket");
        closesocket(listener);
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    if (listen(listener, 5) == SOCKET_ERROR) {
        perror("Failed to listen on socket");
        closesocket(listener);
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    printf("Server da san sang\n");

    for (int i = 0; i < NUM_THREADS; i++) {
        // Tạo một luồng để xử lý kết nối từ client
        while (1) {
            SOCKET clientSocket = accept(listener, NULL, NULL);
            if (clientSocket == INVALID_SOCKET) {
                perror("Failed to accept client connection");
                continue;
            }

            printf("New client connected: %d\n", clientSocket);

            SOCKET* clientPtr = (SOCKET*)malloc(sizeof(SOCKET));
            *clientPtr = clientSocket;

            // Sử dụng _beginthread để tạo một luồng mới và gọi hàm handle_connection
            if (_beginthread(handle_connection, 0, (void*)clientPtr) == -1) {
                perror("Failed to create thread");
                closesocket(clientSocket);
                free(clientPtr);
            }
        }
    }

    closesocket(listener);
    WSACleanup();

    return 0;
}
