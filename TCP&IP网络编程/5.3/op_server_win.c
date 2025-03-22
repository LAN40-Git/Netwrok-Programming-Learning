#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#define BUF_SIZE 1024
#define MAX_OP_COUNT 100
void ErrorHandling(char *message);

int main(int argc, char *argv[])
{
    WSADATA wsaData;
    SOCKET hServSock, hClntSock;
    SOCKADDR_IN hServAddr, hClntAddr;
    int strLen;
    char message[BUF_SIZE];
    int hClntAddrSize;
    int opCount, opResult, opArray[MAX_OP_COUNT];

    if (argc != 2) {
        printf("Usage : %s <port>\n", argv[0]);
        exit(1);
    }

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
        ErrorHandling("WSAStartup() error!");

    hServSock = socket(PF_INET, SOCK_STREAM, 0);
    if (hServSock == INVALID_SOCKET)
        ErrorHandling("socket() error!");

    memset(&hServAddr, 0, sizeof(hServSock));
    hServAddr.sin_family = AF_INET;
    hServAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    hServAddr.sin_port = htons(atoi(argv[1]));

    if (bind(hServSock, (SOCKADDR*)&hServAddr, sizeof(hServAddr)) == -1)
        ErrorHandling("bind() error!");

    if (listen(hServSock, 5) == -1)
        ErrorHandling("listen() error!");

    hClntAddrSize = sizeof(hClntAddr);
    hClntSock = accept(hServSock, (SOCKADDR*)&hClntAddr, &hClntAddrSize);
    if (hClntSock == INVALID_SOCKET)
        ErrorHandling("accept() error!");
    else
        puts("Connected......");

    if(recv(hClntSock, message, BUF_SIZE, 0) <= 0)
        ErrorHandling("recv() error!");
    opCount = atoi(message);
    for (int i = 0; i < opCount; i++) {
        memset(message, 0, sizeof(message));
        if(recv(hClntSock, message, BUF_SIZE, 0) <= 0)
            ErrorHandling("recv() error!");
        opArray[i] = atoi(message);
    }
    if(recv(hClntSock, message, BUF_SIZE, 0) <= 0)
        ErrorHandling("recv() error!");
    if (strcmp(message, "+") == 0) {
        opResult = 0;
        for (int i = 0; i < opCount; i++)
            opResult += opArray[i];
    } else if (strcmp(message, "-") == 0) {
        opResult = opArray[0];
        for (int i = 1; i < opCount; i++)
            opResult -= opArray[i];
    } else if (strcmp(message, "*") == 0) {
        opResult = 1;
        for (int i = 0; i < opCount; i++)
            opResult *= opArray[i];
    } else {
        strcpy(message, "Error! Check your input!");
        send(hClntSock, message, strlen(message), 0);
        exit(1);
    }
    sprintf(message, "Operation result: %d", opResult);
    send(hClntSock, message, strlen(message), 0);
    closesocket(hClntSock);
    closesocket(hServSock);
    WSACleanup();
    return 0;
}

void ErrorHandling(char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}