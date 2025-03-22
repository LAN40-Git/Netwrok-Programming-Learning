#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#define BUF_SIZE 1024
void ErrorHandling(char *message);

int main(int argc, char *argv[])
{
    WSADATA wsaData;
    SOCKET hSocket;
    SOCKADDR_IN hServAddr;
    int opCount;
    char message[BUF_SIZE];

    if (argc != 3) {
        printf("Usage : %s <IP> <port>\n", argv[0]);
        exit(1);
    }

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
        ErrorHandling("WSAStartup() error!");

    hSocket = socket(PF_INET, SOCK_STREAM, 0);
    if (hSocket == INVALID_SOCKET)
        ErrorHandling("socket() error!");

    memset(&hServAddr, 0, sizeof(hSocket));
    hServAddr.sin_family = AF_INET;
    hServAddr.sin_addr.s_addr = inet_addr(argv[1]);
    hServAddr.sin_port = htons(atoi(argv[2]));

    if (connect(hSocket, (SOCKADDR*)&hServAddr, sizeof(hServAddr)))
        ErrorHandling("connect() error!");
    else
        puts("Connected......");

    printf("Operand count: ");
    while(1) {
        if (scanf("%d", &opCount) == 1)
            break;
        printf("Error, input again!");
    }
    sprintf(message, "%d", opCount);
    send(hSocket, message, sizeof(message), 0);
    for (int i = 0; i < opCount; i++) {
        printf("Operand %d: ", i+1);
        int op_i;
        while(1) {
            if (scanf("%d", &op_i) == 1)
                break;
            printf("Error, input again!");
        }
        sprintf(message, "%d", op_i);
        send(hSocket, message, sizeof(message), 0);
    }
    printf("Operator: ");
    scanf("%s", message);
    send(hSocket, message, sizeof(message), 0);
    memset(message, 0, BUF_SIZE);
    if (recv(hSocket, message, BUF_SIZE, 0) <= 0)
        ErrorHandling("recv() error!");
    fputs(message, stdout);
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