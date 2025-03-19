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
    int sock;
    struct sockaddr_in serv_addr;
    int op_cnt;
    char message[BUF_SIZE];
    int str_len;

    if (argc != 3) {
        printf("Usage : %s <IP> <port>\n", argv[0]);
        exit(1);
    }

    sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock == -1)
        error_handling("socket() error!");

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_addr.sin_port = htons(atoi(argv[2]));

    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr))==-1)
        error_handling("connect() error!");
    else
        puts("Connected......");

    printf("Operand count: ");
    while(1) {
        if (scanf("%d", &op_cnt) == 1)
            break;
        printf("Error, input again!\n");
        printf("Operand count: ");
        while(getchar() != '\n');
    }
    sprintf(message, "%d", op_cnt);
    write(sock, message, strlen(message));
    for (int i = 0; i < op_cnt; i++) {
        printf("Operand %d: ", i+1);
        int op_i;
        while (1) {
            if (scanf("%d", &op_i)==1)
                break;
            printf("Error, input again!\n");
            printf("Operand %d: ", i+1);
            while(getchar() != '\n');
        }
        sprintf(message, "%d", op_i);
        write(sock, message, strlen(message));
    }
    printf("Operator: ");
    scanf("%s", message);
    write(sock, message, strlen(message));
    memset(message, 0, BUF_SIZE);
    if (read(sock, message, BUF_SIZE-1)<=0)
        error_handling("read() error!");
    printf("%s\n", message);
    close(sock);
    return 0;
}

void error_handling(char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}