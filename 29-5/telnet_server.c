#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <winsock2.h>
#include <windows.h>

#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

typedef struct {
    SOCKET socket;
    char username[100];
    char password[100];
} Client;

Client clients[MAX_CLIENTS];
int numClients = 0;

bool checkLogin(char* username, char* password) {
    FILE* file = fopen("logindb.txt", "r");
    if (file == NULL) {
        printf("Loi doc file database!\n");
        return false;
    }
    
    char line[256];
    while (fgets(line, sizeof(line), file)) {
        char* user = strtok(line, " ");
        char* pass = strtok(NULL, " ");
        
        if (pass[strlen(pass) - 1] == '\n') {
            pass[strlen(pass) - 1] = '\0';
        }
        
        if (strcmp(username, user) == 0 && strcmp(password, pass) == 0) {
            fclose(file);
            return true;
        }
    }
    
    fclose(file);
    return false;
}

void executeCommand(SOCKET clientSocket, char* command) {
    FILE* fp;
    char buffer[256];
    char result[1024] = "";
    
    fp = _popen(command, "r");
    if (fp == NULL) {
        printf("Loi thuc hien lenh!\n");
        return;
    }
    
    while (fgets(buffer, sizeof(buffer), fp)) {
        strcat(result, buffer);
    }
    
    _pclose(fp);
    
    send(clientSocket, result, strlen(result), 0);
}

int main() {
    WSADATA wsaData;
    SOCKET listenSocket, clientSocket;
    struct sockaddr_in serverAddr, clientAddr;
    int clientAddrLen = sizeof(clientAddr);
    char buffer[BUFFER_SIZE];
    
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("KHoi tao winsock that bai.\n");
        return 1;
    }
    
    listenSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (listenSocket == INVALID_SOCKET) {
        printf("Khong the tao socket lang nghe.\n");
        return 1;
    }
    
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8888);
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    
    if (bind(listenSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        printf("Khong the rang buoc socket lang nghe.\n");
        closesocket(listenSocket);
        return 1;
    }
    
    if (listen(listenSocket, 5) == SOCKET_ERROR) {
        printf("Loi lang nghe ket noi.\n");
        closesocket(listenSocket);
        return 1;
    }
    
    fd_set readfds;
    SOCKET maxSocket;
    int activity, i, valread;
    
    printf("Telnet Server da san sang.\n");
    
    while (true) {
        FD_ZERO(&readfds);
        FD_SET(listenSocket, &readfds);
        maxSocket = listenSocket;
        
        for (i = 0; i < numClients; i++) {
            SOCKET clientSocket = clients[i].socket;
            FD_SET(clientSocket, &readfds);
            
            if (clientSocket > maxSocket) {
                maxSocket = clientSocket;
            }
        }
        
        activity = select(maxSocket + 1, &readfds, NULL, NULL, NULL);
        if ((activity == SOCKET_ERROR) && (WSAGetLastError() != WSAEINTR)) {
            printf("Loi trong ham select.\n");
            break;
        }
        
        if (FD_ISSET(listenSocket, &readfds)) {
            clientSocket = accept(listenSocket, (struct sockaddr*)&clientAddr, &clientAddrLen);
            if (clientSocket == INVALID_SOCKET) {
                printf("Loi chap nhan ket noi.\n");
                continue;
            }
            
            printf("Client moi da ket noi.\n");
            
            if (numClients < MAX_CLIENTS) {
                clients[numClients].socket = clientSocket;
                numClients++;
                send(clientSocket, "Yeu cau nhap tdn va pass:\n", strlen("Yeu cau nhap tdn va pass:\n"), 0);
            } else {
                send(clientSocket, "Số lượng client vượt quá giới hạn.\n", strlen("Số lượng client vượt quá giới hạn.\n"), 0);
                closesocket(clientSocket);
            }
        }
        
        for (i = 0; i < numClients; i++) {
            clientSocket = clients[i].socket;
            
            if (FD_ISSET(clientSocket, &readfds)) {
                valread = recv(clientSocket, buffer, BUFFER_SIZE, 0);
                if (valread == SOCKET_ERROR) {
                    printf("Loi doc du lieu tu client.\n");
                    closesocket(clientSocket);
                    clients[i].socket = INVALID_SOCKET;
                } else if (valread == 0) {
                    printf("Client da ngat ket noi.\n");
                    closesocket(clientSocket);
                    clients[i].socket = INVALID_SOCKET;
                } else {
                    buffer[valread] = '\0';
                    
                    if (strlen(clients[i].username) == 0) {
                        strcpy(clients[i].username, buffer);
                        send(clientSocket, "Mat khau: ", strlen("Mat khau: "), 0);
                    } else if (strlen(clients[i].password) == 0) {
                        strcpy(clients[i].password, buffer);
                        
                        bool loggedIn = checkLogin(clients[i].username, clients[i].password);
                        
                        if (loggedIn) {
                            send(clientSocket, "Dang nhap thanh cong.\n", strlen("Dang nhap thanh cong.\n"), 0);
                            send(clientSocket, "Nhập lệnh: ", strlen("Nhập lệnh: "), 0);
                        } else {
                            send(clientSocket, "Error. Ten dang nhap hoac pass khong dung.\n", strlen("Error. Ten dang nhap hoac pass khong dung.\n"), 0);
                            closesocket(clientSocket);
                            clients[i].socket = INVALID_SOCKET;
                        }
                    } else {
                        if (strcmp(buffer, "exit") == 0) {
                            send(clientSocket, "Da ngat ket noi.\n", strlen("Da ngat ket noi.\n"), 0);
                            closesocket(clientSocket);
                            clients[i].socket = INVALID_SOCKET;
                        } else {
                            executeCommand(clientSocket, buffer);
                            send(clientSocket, "Nhap lenh: ", strlen("Nhap lenh: "), 0);
                        }
                    }
                }
            }
        }
        
        // Xóa các client không hợp lệ khỏi mảng clients
        int j;
        for (i = 0; i < numClients; i++) {
            if (clients[i].socket == INVALID_SOCKET) {
                for (j = i; j < numClients - 1; j++) {
                    clients[j] = clients[j + 1];
                }
                numClients--;
                i--;
            }
        }
    }
    
    closesocket(listenSocket);
    WSACleanup();
    
    return 0;
}
