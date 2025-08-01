#define BUF_SIZE 30
#include <stdio.h>
#include <stdlib.h>
#include  <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/select.h>

void error_handling(char *message);

int main(int argc, char *argv[])
{
    int serv_sock, clnt_sock;
    struct sockaddr_in serv_addr, clnt_addr;
    struct timeval timeout;
    fd_set reads, cpy_reads;
    
    socklen_t addr_sz;
    int fd_max, str_len, fd_num, i;
    char buf[BUF_SIZE];
    if (argc != 2) {
        printf("Usage : %s <port>\n", argv[0]);
        exit(1);
    }

    serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(atoi(argv[1]));

    if (bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
        error_handling("bind() error!");
    if (listen(serv_sock, 5) == -1)
        error_handling("listen() error!");

    FD_ZERO(&reads);
    FD_SET(serv_sock, &reads);
    fd_max = serv_sock;

    while(1) {
        cpy_reads = reads;
        timeout.tv_sec = 5;
        timeout.tv_usec = 5000;

        if ((fd_num = select (fd_max+1, &cpy_reads, 0, 0, &timeout)) == -1)
            break;
        if (fd_num == 0)
            continue;
        
        for (i = 0; i < fd_max+1; i++) {
            if (FD_ISSET(i, &cpy_reads)) {
                if (i == serv_sock) { // 如果是服务端套接字文件描述符，说明有客户端连接请求
                    addr_sz = sizeof(clnt_addr);
                    clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, &addr_sz);
                    FD_SET(clnt_sock, &reads);
                    if (fd_max < clnt_sock)
                        fd_max = clnt_sock;
                    printf("connected client: %d \n", clnt_sock);
                }
                else { // 如果是客户端套接字文件描述符，则读取客户端发送的数据
                    str_len = read(i, buf, BUF_SIZE);
                    if (str_len == 0) { // 接收的数据为EOF时需要断开连接，并从reads中删除相应信息
                        FD_CLR(i, &reads);
                        close(i);
                        printf("closed client: %d \n", i);
                    }
                    else { // 接收的数据为字符串时，需要执行回声服务
                        write(i, buf, str_len);
                    }
                }
            }
        }
    }
    close(serv_sock);
    return 0;
}

void error_handling(char *buf)
{
    fputs(buf, stderr);
    fputc('\n', stderr);
    exit(1);
}