#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE 1024
void error_handling(char *message);

int main(int argc, char *argv[])
{
    int clnt_sock;
    char message[BUF_SIZE];
    int str_len;
    struct sockaddr_in serv_addr;
    socklen_t serv_addr_sz;

    if (argc != 3) {
        printf("Usage : %s <IP> <port>\n", argv[0]);
        exit(1);
    }

    clnt_sock = socket(PF_INET, SOCK_DGRAM, 0);
    if (clnt_sock == -1)
        error_handling("socket() error!");

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_addr.sin_port = htons(atoi(argv[2]));

    while(1) {
        serv_addr_sz = sizeof(serv_addr);
        printf("Input your message: ");
        fgets(message, sizeof(message), stdin);
        sendto(clnt_sock, message, sizeof(message), 0, (struct sockaddr*)&serv_addr, serv_addr_sz);
        str_len = 0;
        memset(message, 0, sizeof(message));
        printf("Wait message from server......\n");
        while(str_len <= 0)
            str_len = recvfrom(clnt_sock, message, sizeof(message), 0, 
            (struct sockaddr*)&serv_addr, &serv_addr_sz);
        printf("Message from server: %s", message);
    }
    close(clnt_sock);
    return 0;
}

void error_handling(char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}