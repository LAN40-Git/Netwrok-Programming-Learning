#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#define BUF_SIZE 1024
void ErrorHandling(char *message);

int main(int argc, char *argv[])
{
    WSADATA wsaData;
    SOCKET hServSock, hClntSock;
    SOCKADDR_IN servAddr, clntAddr;
    TIMEVAL timeout;
    fd_set reads, cpyReads;

    int addrSz;
    int strLen, fdNum, i;
    char buf[BUF_SIZE];

    if (argc != 2) {
        printf("Usage : %s <port>\n", argv[0]);
        exit(1);
    }
    if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0)
        ErrorHandling("WSAStartup() error");
    
    hServSock = socket(PF_INET, SOCK_STREAM, 0);
    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servAddr.sin_port = htons(atoi(argv[1]));

    if (bind(hServSock, (SOCKADDR*)&servAddr, sizeof(servAddr)) == SOCKET_ERROR)
        ErrorHandling("bind() error!");
    if (listen(hServSock, 5) == SOCKET_ERROR)
        ErrorHandling("listen() error!");

    FD_ZERO(&reads);
    FD_SET(hServSock, &reads);

    while(1) {
        cpyReads = reads;
        timeout.tv_sec = 5;
        timeout.tv_usec = 5000;
        
        if ((fdNum=select(0, &cpyReads, 0, 0, &timeout)) == SOCKET_ERROR)
            break;

        if(fdNum == 0)
            continue;

        for (i = 0; i < reads.fd_count; i++) {
            if (FD_ISSET(reads.fd_array[i], &cpyReads)) {
                if (reads.fd_array[i] == hServSock) {
                    addrSz = sizeof(clntAddr);
                    hClntSock = accept(hServSock, (SOCKADDR*)&clntAddr, &addrSz);
                    FD_SET(hClntSock, &reads);
                    printf("connected client: %d \n", hClntSock);
                }
                else {
                    strLen =recv(reads.fd_array[i], buf, BUF_SIZE-1, 0);
                    if (strLen == 0) {
                        FD_CLR(reads.fd_array[i], &reads);
                        closesocket(cpyReads.fd_array[i]);
                        printf("closed client: %d \n", cpyReads.fd_array[i]);
                    }
                    else {
                        send(reads.fd_array[i], buf, strLen, 0);
                    }
                }
            }
        }
    }
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