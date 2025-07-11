#define BUF_SIZE 30
#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>

void ErrorHandling(char *message);

int main(int argc, char *argv[])
{
    WSADATA wsaData;
    SOCKET hSocket;
    SOCKADDR_IN sendAddr;
    if (argc != 3) {
        printf("Usage : %s <IP> <port>\n", argv[0]);
        exit(1);
    }

    if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0)
        ErrorHandling("WSAStartup() error!");
    
    hSocket = socket(PF_INET, SOCK_STREAM, 0);
    if (hSocket == -1)
        ErrorHandling("socket() error!");
    memset(&sendAddr, 0, sizeof(sendAddr));
    sendAddr.sin_family = AF_INET;
    sendAddr.sin_addr.s_addr = inet_addr(argv[1]);
    sendAddr.sin_port = htons(atoi(argv[2]));

    if (connect(hSocket, (SOCKADDR*)&sendAddr, sizeof(sendAddr)) == -1)
        ErrorHandling("connect() error!");
    
    send(hSocket, "123", 3, 0);
    send(hSocket, "4", 1, MSG_OOB);
    send(hSocket, "567", 3, 0);
    send(hSocket, "890", 3, MSG_OOB);

    closesocket(hSocket);
    WSACleanup();
    return 0;
}

void ErrorHandling(char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}