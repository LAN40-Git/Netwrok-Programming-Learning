#define BUF_SIZE 100

#include <stdio.h>
#include <string.h>
#include <winsock2.h>

void CompressSockets(SOCKET hSockArr[], int idx, int total);
void CompressEvents(WSAEVENT hEventArr[], int idx, int total);
void ErrorHandling(char *message);

int main(int argc, char *argv[])
{
    WSADATA wsaData;
    SOCKET hServSock, hClntSock;
    SOCKADDR_IN servAddr, clntAddr;

    int clntAddrSz;
    int numOfClntSock = 0;
    int strLen;
    int posInfo, startIdx;
    char buf[BUF_SIZE];

    SOCKET hSockArr[WSA_MAXIMUM_WAIT_EVENTS];
    WSAEVENT hEventArr[WSA_MAXIMUM_WAIT_EVENTS];
    WSAEVENT newEvent;
    WSANETWORKEVENTS netEvents;

    if (argc != 2) {
        printf("Usage : %s <port>\n", argv[0]);
        exit(1);
    }
    
    if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0)
        ErrorHandling("WSAStartup() error!");

    hServSock = socket(PF_INET, SOCK_STREAM, 0);
    if (hServSock == -1)
        ErrorHandling("socket() error!");
    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servAddr.sin_port = htons(atoi(argv[1]));

    if (bind(hServSock, (SOCKADDR*)&servAddr, sizeof(servAddr)) == -1)
        ErrorHandling("bind() error!");
    if (listen(hServSock, 5) == -1)
        ErrorHandling("listen() error!");

    newEvent = WSACreateEvent();
    if (WSAEventSelect(hServSock, newEvent, FD_ACCEPT) == SOCKET_ERROR)
        ErrorHandling("WSAEventSelect() error!");
    
    hSockArr[numOfClntSock] = hServSock;
    hEventArr[numOfClntSock] = newEvent;
    numOfClntSock++;

    while(1) {
        posInfo = WSAWaitForMultipleEvents(numOfClntSock, hEventArr, FALSE, WSA_INFINITE, FALSE);
        startIdx = posInfo - WSA_WAIT_EVENT_0;

        for (int i = startIdx; i < numOfClntSock; i++) {
            int sigEventIdx = WSAWaitForMultipleEvents(1, &hEventArr[i], TRUE, 0, FALSE);
            if (sigEventIdx == WSA_WAIT_FAILED || sigEventIdx == WSA_WAIT_TIMEOUT) {
                continue;
            } else {
                sigEventIdx = i;
                WSAEnumNetworkEvents(hSockArr[sigEventIdx], hEventArr[sigEventIdx], &netEvents);
                // 请求连接事件
                if (netEvents.lNetworkEvents == FD_ACCEPT) {
                    if (netEvents.iErrorCode[FD_ACCEPT_BIT] != 0) {
                        puts("Accept error!");
                        break;
                    }
                    clntAddrSz = sizeof(clntAddr);
                    hClntSock = accept(hSockArr[sigEventIdx], (SOCKADDR*)&clntAddr, &clntAddrSz);
                    if (hClntSock == -1)
                        ErrorHandling("accept() error!");
                    newEvent = WSACreateEvent();
                    if (WSAEventSelect(hClntSock, newEvent, FD_READ|FD_CLOSE) == SOCKET_ERROR)
                        ErrorHandling("WSAEventSelect() error!");
                    
                    hSockArr[numOfClntSock] = hClntSock;
                    hEventArr[numOfClntSock] = newEvent;
                    numOfClntSock++;
                    printf("Connected client: %lld\n", hClntSock);
                }
                // 接收数据事件
                if (netEvents.lNetworkEvents == FD_READ) {
                    if (netEvents.iErrorCode[FD_READ_BIT] != 0) {
                        puts("Read Error!");
                        break;
                    }
                    strLen = recv(hSockArr[sigEventIdx], buf, BUF_SIZE, 0);
                    if (strLen != 0 && strLen != -1) {
                        buf[strLen] = 0;
                        for (int i = 0; i < numOfClntSock; i++) {
                            if (i != sigEventIdx)
                                send(hSockArr[i], buf, strLen, 0);
                        }
                    }
                }
                // 关闭连接事件
                if (netEvents.lNetworkEvents == FD_CLOSE) {
                    if (netEvents.iErrorCode[FD_CLOSE_BIT] != 0) {
                        puts("Close Error!");
                        break;
                    }
                    WSACloseEvent(hEventArr[sigEventIdx]);
                    closesocket(hSockArr[sigEventIdx]);

                    numOfClntSock--;
                    CompressSockets(hSockArr, sigEventIdx, numOfClntSock);
                    CompressEvents(hEventArr, sigEventIdx, numOfClntSock);
                    printf("Closed client: %d \n", hSockArr[sigEventIdx]);
                }
            }
        }
    }
    WSACleanup();
    return 0;
}

void CompressSockets(SOCKET hSockArr[], int idx, int total)
{
    for (int i = idx; i < total; i++)
        hSockArr[i] = hSockArr[i+1];
}

void CompressEvents(WSAEVENT hEventArr[], int idx, int total)
{
    for (int i = idx; i < total; i++)
        hEventArr[i] = hEventArr[i+1];
}

void ErrorHandling(char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}