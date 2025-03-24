#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#define BUF_SIZE 30
void ErrorHandling(char *message);

int main(int argc, char *argv[])
{
    WSADATA wsaData;
    SOCKET hServSock, hClntSock;
    FILE *fp;
    char buf[BUF_SIZE];
    int readCnt;

    SOCKADDR_IN servAddr, clntAddr;
    int clntAddrSz;
    if (argc != 2) {
        printf("Usage : %s <port>\n", argv[0]);
        exit(1);
    }
    if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0)
        ErrorHandling("WSAStartup() error!");

    fp = fopen("file_server_win.c", "rb");
    hServSock = socket(PF_INET, SOCK_STREAM, 0);
    if (hServSock == INVALID_SOCKET)
        ErrorHandling("socket() error!");

    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servAddr.sin_port = htons(atoi(argv[1]));

    if (bind(hServSock, (SOCKADDR*)&servAddr, sizeof(servAddr)) == SOCKET_ERROR)
        ErrorHandling("bind() error!");
    if (listen(hServSock, 5) == SOCKET_ERROR)
        ErrorHandling("listen() error!");

    clntAddrSz = sizeof(clntAddr);
    hClntSock = accept(hServSock, (SOCKADDR*)&clntAddr, &clntAddrSz);

    while(1) {
        readCnt = fread((void*)buf, 1, BUF_SIZE, fp);
        if (readCnt < BUF_SIZE) {
            send(hClntSock, (char*)&buf, readCnt, 0);
            break;
        }
        send(hClntSock, (char*)&buf, BUF_SIZE, 0);
    }
    shutdown(hClntSock, SD_SEND);
    recv(hClntSock, (char*)buf, BUF_SIZE, 0);
    printf("Message from client: %s \n", buf);

    fclose(fp);
    closesocket(hClntSock); closesocket(hServSock);
    WSACleanup();
    return 0;
}

void ErrorHandling(char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}