#define BUF_SIZE 100
#define MAX_CLNT 256

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <process.h>

unsigned WINAPI HandleClnt(void *arg);
void SendMsg(char *arg, int len);
void ErrorHandling(char *msg);

int clntCnt = 0;
SOCKET clntSocks[MAX_CLNT];
HANDLE hMutex;

int main(int argc, char *argv[])
{
    WSADATA wsaData;
    SOCKET hServSock, hClntSock;
    SOCKADDR_IN servAddr, clntAddr;
    int clntAddrSz;
    HANDLE hThread;
    if (argc != 2) {
        printf("Usage : %s <port>\n", argv[0]);
        exit(1);
    }

    if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0)
        ErrorHandling("WSAStartup() error!");
    
    hMutex = CreateMutex(NULL, FALSE, NULL);
    hServSock = socket(PF_INET, SOCK_STREAM, 0);
    if (hServSock == -1)
        ErrorHandling("socket() error!");
    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servAddr.sin_port = htons(atoi(argv[1]));

    if (bind(hServSock, (struct sockaddr*)&servAddr, sizeof(servAddr)) == -1)
        ErrorHandling("bind() error!");
    if (listen(hServSock, 5) == -1)
        ErrorHandling("listen() error!");

    while(1) {
        clntAddrSz = sizeof(clntAddr);
        hClntSock = accept(hServSock, (struct sockaddr*)&clntAddr, &clntAddrSz);
        if (hClntSock == -1)
            ErrorHandling("accept() error!");
        WaitForSingleObject(hMutex, INFINITE);
        clntSocks[clntCnt++] = hClntSock;
        ReleaseMutex(hMutex);

        hThread = (HANDLE)_beginthreadex(NULL, 0, HandleClnt, (void*)&hClntSock, 0, NULL);
        printf("Connected client IP: %s \n", inet_ntoa(clntAddr.sin_addr));
    }
    closesocket(hServSock);
    CloseHandle(hThread);
    WSACleanup();
    return 0;
}

unsigned WINAPI HandleClnt(void *arg)
{
    SOCKET hClntSock = *((SOCKET*)arg);
    int strLen = 0;
    char msg[BUF_SIZE];

    while (1) {
        strLen = recv(hClntSock, msg, sizeof(msg), 0);
        if (strLen == SOCKET_ERROR) {
            puts("Client Error!");
            break;
        }
        if (strLen == 0)
            break;
        SendMsg(msg, strLen);
    }
        

    WaitForSingleObject(hMutex, INFINITE);
    for (int i = 0; i < clntCnt; i++) {
        if (hClntSock == clntSocks[i]) {
            while(i++ < clntCnt-1)
                clntSocks[i] = clntSocks[i+1];
            break;
        }
    }
    clntCnt--;
    ReleaseMutex(hMutex);
    closesocket(hClntSock);
    printf("Closed client: %lld\n", hClntSock);
    return 0;
}

void SendMsg(char *msg, int len)
{
    WaitForSingleObject(hMutex, INFINITE);
    for (int i = 0; i < clntCnt; i++)
        send(clntSocks[i], msg, len, 0);
    ReleaseMutex(hMutex);
}

void ErrorHandling(char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}