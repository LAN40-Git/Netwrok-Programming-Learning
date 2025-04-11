/**
 * 纯重叠I/O服务器端
 * 使用非阻塞套接字，循环接收客户端连接
 * 若无连接则直接进行下一次循环
 * 连接上之后通过回调进行异步处理
 */

#define BUF_SIZE 1024

#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>

void CALLBACK ReadCompRoutine(DWORD, DWORD, LPWSAOVERLAPPED, DWORD);
void CALLBACK WriteCompRoutine(DWORD, DWORD, LPWSAOVERLAPPED, DWORD);
void ErrorHandling(char *message);

typedef struct {
    SOCKET hClntSock;
    char buf[BUF_SIZE];
    WSABUF wsaBuf;
} PER_IO_DATA, *LPPER_IO_DATA;

int main(int argc, char *argv[])
{
    WSADATA wsaData;
    SOCKET hLisnSock, hRecvSock;
    SOCKADDR_IN lisnAddr, recvAddr;
    LPWSAOVERLAPPED lpOvlp;
    DWORD recvBytes, flagInfo = 0;
    LPPER_IO_DATA hbInfo;
    u_long mode = 1;
    int recvAddrSz;

    if (argc != 2) {
        printf("Usage : %s <port>\n", argv[0]);
        exit(1);
    }
    if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0)
        ErrorHandling("WSAStartup() error!");

    hLisnSock = WSASocket(PF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
    // 将hLisnSock设置为非阻塞
    ioctlsocket(hLisnSock, FIONBIO, &mode);

    memset(&lisnAddr, 0, sizeof(lisnAddr));
    lisnAddr.sin_family = AF_INET;
    lisnAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    lisnAddr.sin_port = htons(atoi(argv[1]));

    if (bind(hLisnSock, (SOCKADDR*)&lisnAddr, sizeof(lisnAddr)) == -1)
        ErrorHandling("bind() error!");
    if (listen(hLisnSock, 5) == -1)
        ErrorHandling("listen() error!");

    recvAddrSz = sizeof(recvAddr);
    // 因为是非阻塞，所以需要循环调用accept
    while(1) {
        SleepEx(100, TRUE);
        hRecvSock = accept(hLisnSock, (SOCKADDR*)&recvAddr, &recvAddrSz);
        if (hRecvSock == INVALID_SOCKET) {
            if (WSAGetLastError() == WSAEWOULDBLOCK) 
                continue;
            else    
                ErrorHandling("accept() error!");
        }
        printf("Connected Client: %lld \n", hRecvSock);

        lpOvlp = (LPWSAOVERLAPPED)malloc(sizeof(WSAOVERLAPPED));
        memset(lpOvlp, 0, sizeof(WSAOVERLAPPED));

        hbInfo = (LPPER_IO_DATA)malloc(sizeof(PER_IO_DATA));
        hbInfo->hClntSock = (DWORD)hRecvSock;
        (hbInfo->wsaBuf).buf = hbInfo->buf;
        (hbInfo->wsaBuf).len = BUF_SIZE;

        lpOvlp->hEvent = (HANDLE)hbInfo;
        WSARecv(hRecvSock, &(hbInfo->wsaBuf), 1, &recvBytes, &flagInfo, lpOvlp, ReadCompRoutine);
    }
    closesocket(hRecvSock);
    closesocket(hLisnSock);
    WSACleanup();
    return 0;
}

void CALLBACK ReadCompRoutine(DWORD dwError, DWORD szRecvBytes, LPWSAOVERLAPPED lpOverlapped, DWORD flags)
{
    LPPER_IO_DATA hbInfo = (LPPER_IO_DATA)(lpOverlapped->hEvent);
    SOCKET hSock = hbInfo->hClntSock;
    LPWSABUF bufInfo = &(hbInfo->wsaBuf);
    DWORD sentBytes;

    // 客户端关闭请求
    if (szRecvBytes == 0) {
        closesocket(hSock);
        free(lpOverlapped->hEvent);
        free(lpOverlapped);
        printf("Client disconnected: %lld\n", hSock);
    } else { // 发送数据
        bufInfo->len = szRecvBytes;
        WSASend(hSock, bufInfo, 1, &sentBytes, 0, lpOverlapped, WriteCompRoutine);
    }
}

void CALLBACK WriteCompRoutine(DWORD dwError, DWORD szSendBytes, LPWSAOVERLAPPED lpOverlapped, DWORD flags)
{
    LPPER_IO_DATA hbInfo = (LPPER_IO_DATA)(lpOverlapped->hEvent);
    SOCKET hSock = hbInfo->hClntSock;
    LPWSABUF bufInfo = &(hbInfo->wsaBuf);
    DWORD recvBytes, flagInfo = 0;
    WSARecv(hSock, bufInfo, 1, &recvBytes, &flagInfo, lpOverlapped, ReadCompRoutine);
}

void ErrorHandling(char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}