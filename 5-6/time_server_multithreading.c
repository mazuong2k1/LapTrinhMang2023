#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <winsock2.h>
#include <windows.h>

#pragma comment(lib, "ws2_32.lib")

#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

void handleClient(SOCKET clientSocket);

int main() {
    WSADATA wsaData;
    SOCKET listenSocket, clientSocket;
    struct sockaddr_in serverAddr, clientAddr;
    int clientAddrLen;

    // Khởi tạo Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("Khong the khoi tao Winsock");
        return 1;
    }

    // Tạo socket lắng nghe kết nối từ client
    if ((listenSocket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        printf("Khong the tao socket");
        return 1;
    }

    // Cấu hình địa chỉ server
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8888);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    // Liên kết socket lắng nghe với địa chỉ server
    if (bind(listenSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        printf("Khong the lien ket socket");
        closesocket(listenSocket);
        return 1;
    }

    // Lắng nghe kết nối từ client
    if (listen(listenSocket, MAX_CLIENTS) == SOCKET_ERROR) {
        printf("Khong the lang nghe ket noi");
        closesocket(listenSocket);
        return 1;
    }

    printf("Dang lang nghe ket noi...\n");

    // Chấp nhận và xử lý kết nối từ client
    while (1) {
        // Chấp nhận kết nối từ client
        clientAddrLen = sizeof(clientAddr);
        clientSocket = accept(listenSocket, (struct sockaddr*)&clientAddr, &clientAddrLen);
        if (clientSocket == INVALID_SOCKET) {
            printf("Khong the chap nhan ket noi");
            closesocket(listenSocket);
            return 1;
        }

        printf("Client da ket noi: %s:%d\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));

        // Tạo một luồng riêng để xử lý kết nối từ client
        DWORD threadId;
        HANDLE hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)handleClient, (LPVOID)clientSocket, 0, &threadId);
        if (hThread == NULL) {
            printf("Khong the tao luong cho client");
            closesocket(clientSocket);
            continue;
        }

        // Đóng handle của luồng
        CloseHandle(hThread);
    }

    // Đóng socket lắng nghe
    closesocket(listenSocket);

    // Giải phóng Winsock
    WSACleanup();

    return 0;
}

void handleClient(SOCKET clientSocket) {
    char buffer[BUFFER_SIZE];
    int ret;

    // Nhận lệnh từ client
    ret = recv(clientSocket, buffer, BUFFER_SIZE, 0);
    if (ret == SOCKET_ERROR || ret == 0) {
        printf("Khong the nhan lenh tu client");
        closesocket(clientSocket);
        return;
    }

    // Chuyển buffer nhận được thành chuỗi ký tự
    buffer[ret] = '\0';

    // Kiểm tra lệnh của client
    if (strcmp(buffer, "GET_TIME dd/mm/yyyy") == 0) {
        // Lấy thời gian hiện tại
        time_t currentTime;
        struct tm* localTime;
        char timeStr[BUFFER_SIZE];

        currentTime = time(NULL);
        localTime = localtime(&currentTime);

        // Định dạng thời gian theo dd/mm/yyyy
        strftime(timeStr, BUFFER_SIZE, "%d/%m/%Y", localTime);

        // Gửi thời gian về cho client
        send(clientSocket, timeStr, strlen(timeStr), 0);
    } else if (strcmp(buffer, "GET_TIME dd/mm/yy") == 0) {
        // Lấy thời gian hiện tại
        time_t currentTime;
        struct tm* localTime;
        char timeStr[BUFFER_SIZE];

        currentTime = time(NULL);
        localTime = localtime(&currentTime);

        // Định dạng thời gian theo dd/mm/yy
        strftime(timeStr, BUFFER_SIZE, "%d/%m/%y", localTime);

        // Gửi thời gian về cho client
        send(clientSocket, timeStr, strlen(timeStr), 0);
    } else if (strcmp(buffer, "GET_TIME mm/dd/yyyy") == 0) {
        // Lấy thời gian hiện tại
        time_t currentTime;
        struct tm* localTime;
        char timeStr[BUFFER_SIZE];

        currentTime = time(NULL);
        localTime = localtime(&currentTime);

        // Định dạng thời gian theo mm/dd/yyyy
        strftime(timeStr, BUFFER_SIZE, "%m/%d/%Y", localTime);

        // Gửi thời gian về cho client
        send(clientSocket, timeStr, strlen(timeStr), 0);
    } else if (strcmp(buffer, "GET_TIME mm/dd/yy") == 0) {
        // Lấy thời gian hiện tại
        time_t currentTime;
        struct tm* localTime;
        char timeStr[BUFFER_SIZE];

        currentTime = time(NULL);
        localTime = localtime(&currentTime);

        // Định dạng thời gian theo mm/dd/yy
        strftime(timeStr, BUFFER_SIZE, "%m/%d/%y", localTime);

        // Gửi thời gian về cho client
        send(clientSocket, timeStr, strlen(timeStr), 0);
    } else {
        // Lệnh không hợp lệ, gửi thông báo lỗi về cho client
        char errorStr[] = "Lệnh không hợp lệ!";
        send(clientSocket, errorStr, strlen(errorStr), 0);
    }

    // Đóng kết nối với client
    closesocket(clientSocket);
}
