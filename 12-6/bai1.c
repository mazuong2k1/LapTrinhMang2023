#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <windows.h>

#define MAX_FILES 100
#define BUFFER_SIZE 1024

void handleClientRequest(SOCKET clientSocket);

int main() {
    WSADATA wsa;
    SOCKET listenSocket, clientSocket;
    struct sockaddr_in server, client;
    int clientAddrLen;

    // Khởi tạo Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("Khong the khoi tao thu vien Winsock");
        return 1;
    }

    // Tạo socket lắng nghe
    if ((listenSocket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        printf("Khong the tao socket");
        return 1;
    }

    // Thiết lập địa chỉ và cổng cho server
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(8888);

    // Liên kết địa chỉ và cổng với socket lắng nghe
    if (bind(listenSocket, (struct sockaddr*)&server, sizeof(server)) == SOCKET_ERROR) {
        printf("Khong the lien ket socket");
        return 1;
    }

    // Lắng nghe kết nối từ client
    listen(listenSocket, 3);
    printf("Dang lang nghe ket noi...\n");

    // Chấp nhận kết nối từ client và xử lý yêu cầu
    while (1) {
        clientAddrLen = sizeof(client);
        if ((clientSocket = accept(listenSocket, (struct sockaddr*)&client, &clientAddrLen)) != INVALID_SOCKET) {
            printf("Client da ket noi.\n");

            // Tạo một tiến trình con để xử lý yêu cầu của client
            HANDLE hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)handleClientRequest, (LPVOID)clientSocket, 0, NULL);
            CloseHandle(hThread);
        }
    }

    closesocket(listenSocket);
    WSACleanup();

    return 0;
}

void handleClientRequest(SOCKET clientSocket) {
    char response[BUFFER_SIZE];
    char fileList[BUFFER_SIZE];
    char filename[BUFFER_SIZE];

    // Mở thư mục chứa file
    const char* folderPath = "C:\\MyFiles";
    HANDLE hFind;
    WIN32_FIND_DATAA fileData;
    char filePath[MAX_PATH];
    int fileCount = 0;

    sprintf(filePath, "%s\\*", folderPath);
    hFind = FindFirstFileA(filePath, &fileData);
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            if (!(fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                strcat(fileList, fileData.cFileName);
                strcat(fileList, "\r\n");
                fileCount++;
            }
        } while (FindNextFileA(hFind, &fileData) != 0);
        FindClose(hFind);
    }

    // Gửi danh sách file cho client
    if (fileCount > 0) {
        sprintf(response, "OK%d\r\n%s\r\n\r\n", fileCount, fileList);
        send(clientSocket, response, strlen(response), 0);
    } else {
        sprintf(response, "ERRORNofilestodownload\r\n");
        send(clientSocket, response, strlen(response), 0);
        closesocket(clientSocket);
        return;
    }

    // Nhận tên file từ client
    memset(filename, 0, sizeof(filename));
    if (recv(clientSocket, filename, BUFFER_SIZE, 0) == SOCKET_ERROR) {
        printf("Khong the nhan du lieu tu client");
        closesocket(clientSocket);
        return;
    }

    // Kiểm tra xem file có tồn tại hay không
    sprintf(filePath, "%s\\%s", folderPath, filename);
    FILE* file = fopen(filePath, "rb");
    if (file == NULL) {
        sprintf(response, "ERRORFileNotFound\r\n");
        send(clientSocket, response, strlen(response), 0);
        closesocket(clientSocket);
        return;
    }

    // Gửi phản hồi OK và kích thước file cho client
    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);
    sprintf(response, "OK %ld\r\n", fileSize);
    send(clientSocket, response, strlen(response), 0);

    // Gửi nội dung file cho client
    memset(response, 0, sizeof(response));
    while (fread(response, sizeof(char), BUFFER_SIZE, file) > 0) {
        send(clientSocket, response, BUFFER_SIZE, 0);
        memset(response, 0, sizeof(response));
    }

    fclose(file);
    closesocket(clientSocket);
}
