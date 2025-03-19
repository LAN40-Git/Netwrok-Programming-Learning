#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#define BUF_SIZE 1024
#define OP_NUMS 100
void error_handling(char *message);

int main(int argc, char *argv[])
{
    int serv_sock, clnt_sock;
    struct sockaddr_in serv_addr, clnt_addr;
    socklen_t clnt_addr_sz;
    char message[BUF_SIZE];
    int str_len;
    int op_cnt, op_result;
    int op_nums[OP_NUMS];

    if (argc != 2) {
        printf("Usage : %s <IP>\n", argv[0]);
        exit(1);
    }
    
    serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    if (serv_sock == -1)
        error_handling("socket() error!");

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(atoi(argv[1]));

    if (bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr))==-1)
        error_handling("bind() error!");

    // 设置为等待接收状态
    if (listen(serv_sock, 5)==-1)
        error_handling("listen() error!");

    clnt_addr_sz=sizeof(clnt_addr);
    clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, &clnt_addr_sz);
    if (clnt_sock == -1)
        error_handling("accept() error!");
    else
        puts("Connected......");

    memset(message, 0, BUF_SIZE);
    if (read(clnt_sock, message, BUF_SIZE)<=0)
        error_handling("read() error!");
    op_cnt = atoi(message);
    if (op_cnt > OP_NUMS) {
        strcpy(message, "Error: Too many operands!");
        write(clnt_sock, message, BUF_SIZE);
        close(clnt_sock);
        close(serv_sock);
        return 0;
    }
    // 读取操作数
    for (int i = 0; i < op_cnt; i++) {
        memset(message, 0, BUF_SIZE);
        if (read(clnt_sock, message, BUF_SIZE)<=0)
            error_handling("read() error!");
        op_nums[i] = atoi(message);
    }

    // 读取操作符
    memset(message, 0, BUF_SIZE);
    if (read(clnt_sock, message, BUF_SIZE)<=0)
        error_handling("read() error!");
    // 根据操作符计算结果并返回
    if (strcmp(message,"+") == 0) {
        op_result = 0;
        for (int i = 0; i < op_cnt; i++)
            op_result += op_nums[i];
        char result[BUF_SIZE];
    } else if (strcmp(message,"-") == 0) {
        op_result = 0;
        for (int i = 0; i < op_cnt; i++)
            op_result -= op_nums[i];
        char result[BUF_SIZE];
    }
    else if (strcmp(message,"*") == 0) {
        op_result = 1;
        for (int i = 0; i < op_cnt; i++)
            op_result *= op_nums[i];
        char result[BUF_SIZE];
    } else {
        strcpy(message, "Error! Check the operator!");
        write(clnt_sock, message, BUF_SIZE);
        close(clnt_sock);
        close(serv_sock);
        return 0;
    }

    sprintf(message, "Operation result: %d", op_result);
    write(clnt_sock, message, BUF_SIZE);
    close(clnt_sock);
    close(serv_sock);
    return 0;
}

void error_handling(char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}