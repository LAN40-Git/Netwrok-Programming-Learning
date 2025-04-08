#define BUF_SIZE 100

#include <stdio.h>
#include <string.h>
#include <winsock2.h>

void CompressSockets(SOCKET hSockArr[], int idx, int total);
void CompressEvents(WSAEVENT hEventArr[], int idx, int total);
void ErrorHandling(char *msg);


int main(int argc, char *argv[])
{
    WSADATA wsaData; // Winsock库基本信息
    SOCKET hServSock, hClntSock; 
    SOCKADDR_IN servAddr, clntAddr;

    SOCKET hSockArr[WSA_MAXIMUM_WAIT_EVENTS];    // 准备被监听的套接字队列
    WSAEVENT hEventArr[WSA_MAXIMUM_WAIT_EVENTS]; // 监听事件队列
    WSAEVENT newEvent; // 监听事件临时变量
    WSANETWORKEVENTS netEvents; // 保存事件发生类型和错误信息的结构体变量

    int numOfClntSock = 0;
    int strLen;
    int posInfo, startIdx;
    int clntAddrLen;
    char msg[BUF_SIZE];

    if (argc != 2) {
        printf("Usage %s <port>\n", argv[0]);
        exit(1);
    }
    if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0)
        ErrorHandling("WSAStartup() Error!");

    hServSock = socket(PF_INET, SOCK_STREAM, 0);
    if (hServSock == -1)
        ErrorHandling("socket() Error!");
    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servAddr.sin_port = htons(atoi(argv[1]));

    if (bind(hServSock, (SOCKADDR*)&servAddr, sizeof(servAddr)) == -1)
        ErrorHandling("bind() Error!");
    if (listen(hServSock, 5) == -1)
        ErrorHandling("listen() Error!");

    newEvent = WSACreateEvent();
    if (WSAEventSelect(hServSock, newEvent, FD_ACCEPT) == SOCKET_ERROR)
        ErrorHandling("WSAEventSelect() Error!");

    hSockArr[numOfClntSock] = hServSock;
    hEventArr[numOfClntSock] = newEvent;
    numOfClntSock++;

    while(1) {
        // 获取发生事件的索引信息
        posInfo = WSAWaitForMultipleEvents(numOfClntSock, hEventArr, FALSE, WSA_INFINITE, FALSE);
        // 计算起始地址
        startIdx = posInfo - WSA_WAIT_EVENT_0;

        // 处理从startIdx至最后的套接字中发生事件的套接字
        for (int i = startIdx; i < numOfClntSock; i++) {
            int sigEventIdx = WSAWaitForMultipleEvents(1, &hEventArr[i], TRUE, 0, FALSE);
            if (sigEventIdx == WSA_WAIT_FAILED || sigEventIdx == WSA_WAIT_TIMEOUT) {
                continue;
            }
            else {
                sigEventIdx = i;
                WSAEnumNetworkEvents(hSockArr[sigEventIdx], hEventArr[sigEventIdx], &netEvents);
                // 处理请求连接事件
                if (netEvents.lNetworkEvents & FD_ACCEPT) {
                    // 连接错误
                    if (netEvents.iErrorCode[FD_ACCEPT_BIT] != 0) {
                        puts("Accept() Error!");
                        break;
                    }
                    // 处理客户端连接
                    clntAddrLen = sizeof(clntAddr);
                    hClntSock = accept(hSockArr[sigEventIdx], (SOCKADDR*)&clntAddr, &clntAddrLen);
                    if (hClntSock == -1)
                        ErrorHandling("accept() Error!");
                    newEvent = WSACreateEvent();
                    // 监听客户端的读取事件和关闭事件
                    WSAEventSelect(hClntSock, newEvent, FD_READ|FD_CLOSE);

                    // 套接字和事件队列加一
                    hEventArr[numOfClntSock] = newEvent;
                    hSockArr[numOfClntSock] = hClntSock;
                    numOfClntSock++;
                    printf("Connected client: %lld \n", hClntSock);
                }

                // 处理接收数据事件
                if (netEvents.lNetworkEvents & FD_READ) {
                    // 接收数据错误
                    if (netEvents.iErrorCode[FD_READ_BIT] != 0) {
                        puts("Read Error!");
                        break;
                    }
                    strLen = recv(hSockArr[sigEventIdx], msg, sizeof(msg), 0);
                    send(hSockArr[sigEventIdx], msg, strLen, 0);
                }

                // 处理请求关闭事件
                if (netEvents.lNetworkEvents & FD_CLOSE) {
                    // 关闭错误
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

void ErrorHandling(char *msg)
{
    fputs(msg, stderr);
    fputc('\n', stderr);
    exit(1);
}