#define BUF_SIZE 1024

#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>

void ErrorHandling(char *message);

int main(int argc, char *argv[])
{
    WSADATA wsaData;
    SOCKET hLisnSock, hRecvSock;
    SOCKADDR_IN lisnAddr, recvAddr;
    int recvAddrSz;

    WSABUF dataBuf;
    WSAEVENT evObj;
    WSAOVERLAPPED overlapped;

    char buf[BUF_SIZE];
    DWORD recvBytes = 0, flags = 0;
    if (argc != 2) {
        printf("Usage : %s <port>\n", argv[0]);
        exit(1);
    }
    if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0)
        ErrorHandling("WSAStartup() error!");

    hLisnSock = WSASocket(PF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
    memset(&lisnAddr, 0, sizeof(lisnAddr));
    lisnAddr.sin_family = AF_INET;
    lisnAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    lisnAddr.sin_port = htons(atoi(argv[1]));

    if (bind(hLisnSock, (SOCKADDR*)&lisnAddr, sizeof(lisnAddr)) == -1)
        ErrorHandling("bind() error!");
    if (listen(hLisnSock, 5) == -1)
        ErrorHandling("listen() error!");

    recvAddrSz = sizeof(recvAddr);
    hRecvSock = accept(hLisnSock, (SOCKADDR*)&recvAddr, &recvAddrSz);

    evObj = WSACreateEvent();
    memset(&overlapped, 0, sizeof(overlapped));
    overlapped.hEvent = evObj;
    dataBuf.len = BUF_SIZE;
    dataBuf.buf = buf;

    if (WSARecv(hRecvSock, &dataBuf, 1, &recvBytes, &flags, &overlapped, NULL) == SOCKET_ERROR) {
        if (WSAGetLastError() == WSA_IO_PENDING) {
            puts("Background data receive");
            WSAWaitForMultipleEvents(1, &evObj, TRUE, WSA_INFINITE, FALSE);
            WSAGetOverlappedResult(hRecvSock, &overlapped, &recvBytes, FALSE, NULL);
        } else {
            ErrorHandling("WSARecv() error!");
        }
    }

    printf("Received message: %s \n", buf);
    WSACloseEvent(evObj);
    closesocket(hRecvSock);
    closesocket(hLisnSock);
    WSACleanup();
    return 0;
}

void ErrorHandling(char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}