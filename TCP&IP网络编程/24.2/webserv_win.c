#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <process.h>

#define BUF_SIZE 2048
#define BUF_SMALL 100

unsigned WINAPI RequestHandler(void *arg);
char *ContentType(char *file);
void SendData(SOCKET sock, char *ct, char *fileName);
void SendErrorMSG(SOCKET sock);
void ErrorHandling(char *message);

int main(int argc, char *argv[])
{
    WSADATA wsaData;
    SOCKET hServSock, hClntSock;
    SOCKADDR_IN servAddr, clntAddr;

    HANDLE hThread;
    DWORD dwThreadID;
    int clntAddrSize;

    if (argc != 2) {
        printf("Usage : %s <port>\n", argv[0]);
        exit(1);
    }
    if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0)
        ErrorHandling("WSAStartup() error!");
    
    hServSock = socket(PF_INET, SOCK_STREAM, 0);
    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servAddr.sin_port = htons(atoi(argv[1]));

    if (bind(hServSock, (SOCKADDR*)&servAddr, sizeof(servAddr)) == SOCKET_ERROR)
        ErrorHandling("bind() error!");
    if (listen(hServSock, 5) == SOCKET_ERROR)
        ErrorHandling("listen() error!");

    while(1) {
        clntAddrSize = sizeof(clntAddr);
        hClntSock = accept(hServSock, (SOCKADDR*)&clntAddr, &clntAddrSize);
        printf("Connection Request : %s:%d\n", inet_ntoa(clntAddr.sin_addr), ntohs(clntAddr.sin_port));
        hThread = (HANDLE)_beginthreadex(NULL, 0, RequestHandler, (void*)hClntSock, 0, (unsigned*)&dwThreadID);
    }
    closesocket(hServSock);
    WSACleanup();
    return 0;
}

unsigned WINAPI RequestHandler(void *arg)
{
    SOCKET hClntSock = (SOCKET)arg;
    char buf[BUF_SIZE];
    char method[BUF_SMALL];
    char ct[BUF_SMALL];
    char fileName[BUF_SMALL];

    recv(hClntSock, buf, BUF_SIZE, 0);
    if (strstr(buf, "HTTP/") == NULL) { // 查看是否为HTTP提出的请求
        SendErrorMSG(hClntSock);
        closesocket(hClntSock);
        return 1;
    }

    strcpy(method, strtok(buf, " /"));
    if (strcmp(method, "GET"))
        SendErrorMSG(hClntSock);

    strcpy(fileName, strtok(NULL, " /")); // 查看请求文件名
    if(strlen(fileName) == 0) {  // 处理根路径
        strcpy(fileName, "index.html");
    }
    strcpy(ct, ContentType(fileName)); // 查看Content-type
    SendData(hClntSock, ct, fileName); // 响应
    return 0;
}

void SendData(SOCKET sock, char *ct, char *fileName) 
{
    FILE* sendFile = fopen(fileName, "rb");
    if (!sendFile) {
        SendErrorMSG(sock);
        return;
    }

    // 获取实际文件大小
    fseek(sendFile, 0, SEEK_END);
    long fileSize = ftell(sendFile);
    rewind(sendFile);

    // 构建标准HTTP头
    char header[512];
    snprintf(header, sizeof(header),
        "HTTP/1.1 200 OK\r\n"
        "Server: SimpleWebServer\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %ld\r\n"
        "Connection: close\r\n\r\n",
        ct, fileSize);
    
    send(sock, header, strlen(header), 0);

    // 发送文件内容（）
    char buf[BUF_SIZE];
    size_t bytesRead;
    while((bytesRead = fread(buf, 1, BUF_SIZE, sendFile)) > 0) send(sock, buf, bytesRead, 0);

    fclose(sendFile);
    closesocket(sock);
}

void SendErrorMSG(SOCKET sock) // 发生错误时传递消息
{
    char protocol[] = "HTTP/1.0 400 Bad Request\r\n";
    char servName[] = "Server:simple web server\r\n";
    char cntLen[] = "Content-length:2048\r\n";
    char cntType[] = "Content-type:text/html\r\n\r\n";
    char content[] = "<html><head><title>NETWORK</title></head>"
                     "<body><font size=+5><br> 发生错误！查看请求文件名和请求方式！"
                     "</font></body></html>";
    send(sock, protocol, strlen(protocol), 0);
    send(sock, servName, strlen(servName), 0);
    send(sock, cntLen, strlen(cntLen), 0);
    send(sock, cntType, strlen(cntType), 0);
    send(sock, content, strlen(content), 0);
    closesocket(sock);
}

char *ContentType(char *file)
{
    char extension[BUF_SMALL];
    char fileName[BUF_SMALL];
    strcpy(fileName, file);
    strtok(fileName, ".");
    strcpy(extension, strtok(NULL, "."));
    if (!strcmp(extension, "html") || !strcmp(extension, "htm"))
        return "text/html";
    else
        return "text/plain";
}

void ErrorHandling(char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}