#define BUF_SIZE 1024

#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>

void CALLBACK ReadComRoutine(DWORD, DWORD, LPWSAOVERLAPPED, DWORD);
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
    DWORD recvBytes;
    LPPER_IO_DATA hbInfo;
    int mode = 1, recvAddrSz, flagInfo = 0;

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
    return 0;
}