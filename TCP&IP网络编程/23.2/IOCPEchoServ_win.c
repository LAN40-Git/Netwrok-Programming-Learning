#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <winsock2.h>
#include <windows.h>

#define BUF_SIZE 100
#define READ 3
#define WRITE 5

typedef struct { // 套接字信息
    SOCKET hClntSock;
    SOCKADDR_IN clntAddr;
} PER_HANDLE_DATA, *LPPER_HANDLE_DATA;

typedef struct { // 缓冲信息
    OVERLAPPED overlapped;
    WSABUF wsaBuf;
    char buffer[BUF_SIZE];
    int rwMode; // 读或者写
} PER_IO_DATA, *LPPER_IO_DATA;

unsigned WINAPI EchoThreadMain(LPVOID CompletionPortIO);
void ErrorHandling(char *message);

int main(int argc, char *argv[])
{
    WSADATA wsaData;
    HANDLE hComPort;
    SYSTEM_INFO sysInfo;
    LPPER_IO_DATA ioInfo;
    LPPER_HANDLE_DATA handleInfo;

    SOCKET hServSock;
    SOCKADDR_IN servAddr;
    DWORD recvBytes, flags = 0;
    if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0)
        ErrorHandling("WSAStartup() error!");
    // 创建CP对象
    hComPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
    // 获取系统信息
    GetSystemInfo(&sysInfo);
    // 创建数量为CPU个数的线程
    for (int i = 0; i < sysInfo.dwNumberOfProcessors; i++)
        _beginthreadex(NULL, 0, EchoThreadMain, (LPVOID)hComPort, 0, NULL);

    hServSock = WSASocket(PF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servAddr.sin_port = htons(atoi(argv[1]));

    if (bind(hServSock, (SOCKADDR*)&servAddr, sizeof(servAddr)) == SOCKET_ERROR)
        ErrorHandling("bind() error!");
    if (listen(hServSock, 5) == SOCKET_ERROR)
        ErrorHandling("listen() error!");

    while(1) {
        SOCKET hClntSock;
        SOCKADDR_IN clntAddr;
        int addrLen = sizeof(clntAddr);

        hClntSock = accept(hServSock, (SOCKADDR*)&clntAddr, &addrLen);
        if (hClntSock == INVALID_SOCKET) {
            continue;
        }
            
        handleInfo = (LPPER_HANDLE_DATA)malloc(sizeof(PER_HANDLE_DATA));
        handleInfo->hClntSock = hClntSock;
        memcpy(&(handleInfo->clntAddr), &clntAddr, addrLen);

        CreateIoCompletionPort((HANDLE)hClntSock, hComPort, (ULONG_PTR)handleInfo, 0);

        // 主动发起读取
        ioInfo = (LPPER_IO_DATA)malloc(sizeof(PER_IO_DATA));
        memset(&(ioInfo->overlapped), 0, sizeof(OVERLAPPED));
        ioInfo->wsaBuf.len = BUF_SIZE;
        ioInfo->wsaBuf.buf = ioInfo->buffer;
        ioInfo->rwMode = READ;
        WSARecv(handleInfo->hClntSock, &(ioInfo->wsaBuf), 1, &recvBytes, &flags, &(ioInfo->overlapped), NULL);
    }
    WSACleanup();
    return 0;
}

unsigned WINAPI EchoThreadMain(LPVOID pComPort)
{
    HANDLE hComPort = (HANDLE)pComPort;
    SOCKET sock;
    DWORD bytesTrans;
    LPPER_HANDLE_DATA handleInfo;
    LPPER_IO_DATA ioInfo;
    DWORD flags = 0;

    while(1) {
        // 从I/O请求中取出一个并进行处理
        BOOL status = GetQueuedCompletionStatus(hComPort, &bytesTrans, (PULONG_PTR)handleInfo, (LPOVERLAPPED*)&ioInfo, INFINITE);
        if (!status || handleInfo == NULL)
            continue;
        sock = handleInfo->hClntSock;

        if (ioInfo->rwMode == READ) {
            puts("message recvived!");
            if (bytesTrans == 0) {
                closesocket(sock);
                free(handleInfo);
                free(ioInfo);
                continue;
            }

            memset(&(ioInfo->overlapped), 0, sizeof(OVERLAPPED));
            ioInfo->wsaBuf.len = bytesTrans;
            ioInfo->rwMode = WRITE;
            WSASend(sock, &(ioInfo->wsaBuf), 1, NULL, 0, &(ioInfo->overlapped), NULL);
            ioInfo = (LPPER_IO_DATA)malloc(sizeof(PER_IO_DATA));
            memset(&(ioInfo->overlapped), 0, sizeof(OVERLAPPED));
            ioInfo->wsaBuf.len = BUF_SIZE;
            ioInfo->wsaBuf.buf = ioInfo->buffer;
            ioInfo->rwMode = READ;
            WSARecv(sock, &(ioInfo->wsaBuf), 1, NULL, &flags, &(ioInfo->overlapped), NULL);
        } else {
            puts("message sent!");
            free(ioInfo);
        }
    }
    return 0;
}

void ErrorHandling(char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}