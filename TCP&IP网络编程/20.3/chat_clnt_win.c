#define BUF_SIZE 100
#define NAME_SIZE 20

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <windows.h>
#include <process.h>

unsigned WINAPI SendMsg(void *arg);
unsigned WINAPI RecvMsg(void *arg);
void ErrorHandling(char *msg);

char name[NAME_SIZE] = "[DEFAULT]";
char msg[BUF_SIZE];

int main(int argc, char *argv[])
{
    WSADATA wsaData;
    SOCKET sock;
    SOCKADDR_IN servAddr;
    HANDLE hSndThread, hRcvThread;
    if (argc != 4) {
        printf("Usage : %s <IP> <port> <name>\n", argv[0]);
        exit(1);
    }

    if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0)
        ErrorHandling("WSAStartup() error!");

    sprintf(name, "[%s]", argv[3]);
    
    sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock == -1)
        ErrorHandling("socket() error!");
    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = inet_addr(argv[1]);
    servAddr.sin_port = htons(atoi(argv[2]));

    if (connect(sock, (struct sockaddr*)&servAddr, sizeof(servAddr)) == -1)
        ErrorHandling("connect() error!");

    hSndThread = (HANDLE)_beginthreadex(NULL, 0, SendMsg, (void*)&sock, 0, NULL);
    hRcvThread = (HANDLE)_beginthreadex(NULL, 0, RecvMsg, (void*)&sock, 0, NULL);
    WaitForSingleObject(hSndThread, INFINITE);
    WaitForSingleObject(hRcvThread, INFINITE);

    closesocket(sock);
    CloseHandle(hSndThread);
    CloseHandle(hRcvThread);
    WSACleanup();
    return 0;
}

unsigned WINAPI SendMsg(void *arg)
{
    SOCKET sock = *((SOCKET*)arg);
    char nameMsg[NAME_SIZE+BUF_SIZE];
    while(1) {
        fgets(msg, BUF_SIZE, stdin);
        if (!strcmp(msg, "q\n") || !strcmp(msg, "Q\n")) {
            shutdown(sock, SD_BOTH);
            return 0;
        }
        sprintf(nameMsg, "%s %s", name, msg);
        send(sock, nameMsg, sizeof(nameMsg), 0);
    }
    return 0;
}

unsigned WINAPI RecvMsg(void *arg)
{
    SOCKET sock = *((SOCKET*)arg);
    char nameMsg[NAME_SIZE+BUF_SIZE];
    int strLen;
    while(1) {
        strLen = recv(sock, nameMsg, BUF_SIZE+NAME_SIZE-1, 0);
        if (strLen <= 0) {
            shutdown(sock, SD_RECEIVE);
            return 0;
        }
        nameMsg[strLen] = 0;
        fputs(nameMsg, stdout);
    }
    return 0;
}

void ErrorHandling(char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}