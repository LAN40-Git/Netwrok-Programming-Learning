#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <winsock2.h>
#include <windows.h>
#include <pthread.h>

#define READ 3
#define WRITE 5
#define BUF_SIZE 100
#define MAX_CLNT 100

typedef struct { // 套接字信息
    SOCKET hClntSock;
    SOCKADDR_IN clntAddr;
} PER_HANDLE_DATA, *LPPER_HANDLE_DATA;

typedef struct {
    OVERLAPPED overlapped;
    WSABUF wsaBuf;
    char buffer[BUF_SIZE];
    int rwMode;
} PER_IO_DATA, *LPPER_IO_DATA;

unsigned WINAPI ChatThreadMain(LPVOID CompletionPortIO);
void ErrorHandling(char *message);
int FindSock(SOCKET sock);
void ReleaseResources(int index, LPPER_IO_DATA ioData, LPPER_HANDLE_DATA handleData);

SOCKET hSockArr[MAX_CLNT];
int numOfClnt = 0;
pthread_mutex_t mutex;

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
    if (argc != 2) {
        printf("Usage : %s <port>\n", argv[0]);
        exit(1);
    }

    if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0)
        ErrorHandling("WSAStartup() error!");

    pthread_mutex_init(&mutex, NULL);
    // 创建CP对象
    hComPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
    if (hComPort == NULL)
        ErrorHandling("CreateIoCompletionPort() error!");
    GetSystemInfo(&sysInfo);
    for (int i = 0; i < sysInfo.dwNumberOfProcessors; i++)
        _beginthreadex(NULL, 0, ChatThreadMain, (LPVOID)hComPort, 0, NULL);
    
    // 创建具有重叠属性的套接字
    hServSock = WSASocket(PF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
    if (hServSock == INVALID_SOCKET)
        ErrorHandling("WSASocket() error!");
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

        pthread_mutex_lock(&mutex);
        hSockArr[numOfClnt++] = hClntSock;
        pthread_mutex_unlock(&mutex);
        printf("Connected client: %lld , Number of client: %d\n", hClntSock, numOfClnt);

        ioInfo = (LPPER_IO_DATA)malloc(sizeof(PER_IO_DATA));
        memset(&(ioInfo->overlapped), 0, sizeof(OVERLAPPED));
        ioInfo->wsaBuf.len = BUF_SIZE;
        ioInfo->wsaBuf.buf = ioInfo->buffer;
        ioInfo->rwMode = READ;
        WSARecv(hClntSock, &(ioInfo->wsaBuf), 1, &recvBytes, &flags, &(ioInfo->overlapped), NULL);
    }
    pthread_mutex_destroy(&mutex);
    WSACleanup();
    return 0;
}

unsigned WINAPI ChatThreadMain(LPVOID pComPort)
{
    HANDLE hComPort = (HANDLE)pComPort;
    SOCKET sock;
    DWORD bytesTrans;
    LPPER_IO_DATA ioInfo;
    LPPER_HANDLE_DATA handleInfo;
    ULONG_PTR completionKey;
    DWORD recvBytes, flags = 0;

    while(1) {
        BOOL status = GetQueuedCompletionStatus(hComPort, &bytesTrans, &completionKey, (LPOVERLAPPED*)&ioInfo, INFINITE);
        LPPER_HANDLE_DATA handleInfo = (LPPER_HANDLE_DATA)completionKey;
        if (!status || handleInfo == NULL || ioInfo == NULL) {
            // 错误处理
            if (ioInfo) free(ioInfo);
            if (handleInfo) {
                int index = FindSock(handleInfo->hClntSock);
                ReleaseResources(index, NULL, handleInfo);
            }
            continue;
        }
        sock = handleInfo->hClntSock;
        if (ioInfo->rwMode == READ) {
            if (bytesTrans == 0) {  // 客户端断开连接
                int index = FindSock(sock);
                ReleaseResources(index, ioInfo, handleInfo);
                continue;
            }

            // 保存接收到的消息内容
            char message[BUF_SIZE];
            memcpy(message, ioInfo->buffer, bytesTrans);

            // 将消息转发至除发送者之外的所有客户端
            pthread_mutex_lock(&mutex);
            for (int i = 0; i < numOfClnt; i++) {
                if (hSockArr[i] != sock) {
                    LPPER_IO_DATA writeIoInfo = (LPPER_IO_DATA)malloc(sizeof(PER_IO_DATA));
                    memset(&(writeIoInfo->overlapped), 0, sizeof(OVERLAPPED));
                    memcpy(writeIoInfo->buffer, message, bytesTrans);
                    writeIoInfo->wsaBuf.len = bytesTrans;
                    writeIoInfo->wsaBuf.buf = writeIoInfo->buffer;
                    writeIoInfo->rwMode = WRITE;
                    
                    WSASend(hSockArr[i], &(writeIoInfo->wsaBuf), 1, NULL, 
                        0, &(writeIoInfo->overlapped), NULL);
                }
            }
            pthread_mutex_unlock(&mutex);

            memset(&(ioInfo->overlapped), 0, sizeof(OVERLAPPED));
            ioInfo->wsaBuf.len = BUF_SIZE;
            ioInfo->wsaBuf.buf = ioInfo->buffer;
            ioInfo->rwMode = READ;

            int ret = WSARecv(sock, &(ioInfo->wsaBuf), 1, &recvBytes, &flags, &(ioInfo->overlapped), NULL);
            if (ret == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
                int index = FindSock(sock);
                ReleaseResources(index, ioInfo, handleInfo);
            }
        } else if (ioInfo->rwMode == WRITE) {
            printf("Message has been sent to client: %lld\n", sock);
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

int FindSock(SOCKET sock)
{
    pthread_mutex_lock(&mutex);
    int index = -1;
    for (int i = 0; i < numOfClnt; i++) {
        if (hSockArr[i] == sock) {
            index = i;
            break;
        }
    }
    pthread_mutex_unlock(&mutex);
    return index;
}

void ReleaseResources(int index, LPPER_IO_DATA ioData, LPPER_HANDLE_DATA handleData)
{
    pthread_mutex_lock(&mutex);
    if (index >= 0 && index < numOfClnt) {
        if (hSockArr[index] != INVALID_SOCKET) {
            closesocket(hSockArr[index]);
            hSockArr[index] = INVALID_SOCKET;
        }
        for (int i = index; i < numOfClnt-1; i++) {
            hSockArr[i] = hSockArr[i+1];
        }
        numOfClnt--;
    }
    pthread_mutex_unlock(&mutex);
    if (ioData != NULL) free(ioData);
    if (handleData != NULL) free(handleData);
}