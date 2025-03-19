#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#define BUF_SIZE 1024
void error_handling(char *message);

int main(int argc, char *argv[])
{
    int serv_sock, clnt_sock;
    struct sockaddr_in serv_addr;
    char *message[BUF_SIZE];
    int str_len, recv_len;

    if (argc != 2) {
        printf("Usage : %s <IP>\n", argv[0]);
        exit(1);
    }

    serv_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (serv_sock == -1)
        error_handling("socket() error!");

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(INADDR_ANY);
    serv_addr.sin_port = htonl(atoi(argv[1]));

    if (bind(serv_sock, (struct sockaddr*)&serv_addr, BUF_SIZE)==-1)
        error_handling("bind() error!");

    // 设置为等待接收状态
    if (listen(serv_sock, 5)==-1)
        error_handling("listen() error!");

    while(1)
    {
        
    }
    
    return 0;
}

void error_handling(char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}