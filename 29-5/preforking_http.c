#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#define PORT 8080
#define NUM_CHILDREN 5

void handle_connection(SOCKET client) {
    char buf[256];
    int ret = recv(client, buf, sizeof(buf), 0);
    buf[ret] = 0;
    puts(buf);

    char *msg = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<html><body><h1>Xin chao cac ban</h1></body></html>";
    send(client, msg, strlen(msg), 0);

    closesocket(client);
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

    if (bind(listener, (struct sockaddr *)&server_address, sizeof(server_address)) == SOCKET_ERROR) {
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

    for (int i = 0; i < NUM_CHILDREN; i++) {
        // Quá trình con
        if (fork() == 0) {
            while (1) {
                SOCKET client = accept(listener, NULL, NULL);
                printf("New client connected: %d\n", client);
                handle_connection(client);
            }
            exit(EXIT_SUCCESS);
        }
    }

    // Quá trình cha
    while (1) {
        // Chờ kết thúc của quá trình con
        wait(NULL);
    }

    closesocket(listener);
    WSACleanup();

    return 0;
}
