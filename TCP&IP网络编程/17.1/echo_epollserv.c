#define BUF_SIZE 100
#define EPOLL_SIZE 50

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/epoll.h>

void error_handling(char *message);

int main(int argc, char *argv[])
{
    int serv_sock, clnt_sock;
    struct sockaddr_in serv_addr, clnt_addr;
    socklen_t addr_sz;
    int str_len;
    char buf[BUF_SIZE];

    struct epoll_event *ep_events;
    struct epoll_event event;
    int epfd, event_cnt;

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
    
    epfd = epoll_create(EPOLL_SIZE); // 创建epoll例程
    ep_events = malloc(sizeof(struct epoll_event)*EPOLL_SIZE);

    event.events = EPOLLIN; // 监视读取数据情况
    event.data.fd = serv_sock;
    // 将serv_sock注册到epoll实例（epfd），并监听event指定的事件
    epoll_ctl(epfd, EPOLL_CTL_ADD, serv_sock, &event);

    while(1) {
        // 等待发生事件并获取发生的数量，同时将发生事件的文件描述符u缓存到ep_events中
        event_cnt = epoll_wait(epfd, ep_events, EPOLL_SIZE, -1);
        if (event_cnt == -1) {
            puts("epoll_wait() error!");
            break;
        }

        for (int i = 0; i < event_cnt; i++) {
            if (ep_events[i].data.fd == serv_sock) { // 有客户端请求连接
                addr_sz = sizeof(clnt_addr);
                clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, &addr_sz);
                if (clnt_sock == -1) {
                    puts("connect() error!\n");
                    continue;
                }
                event.events = EPOLLIN;
                event.data.fd = clnt_sock;
                // 成功接收到客户端请求时，将客户端套接字注册到epoll实例，并监听event指定的事件
                epoll_ctl(epfd, EPOLL_CTL_ADD, clnt_sock, &event);
                printf("connected client: %d \n", clnt_sock);
            } else { // 监听到客户端发送的消息
                str_len = read(ep_events[i].data.fd, buf, BUF_SIZE);
                if (str_len == 0) { // 客户端断开连接
                    // 将对应的客户端文件描述符从epfd的监听对象中移除
                    epoll_ctl(epfd, EPOLL_CTL_DEL, ep_events[i].data.fd, NULL);
                    close(ep_events[i].data.fd);
                    printf("closed client: %d \n", ep_events[i].data.fd);
                } else {
                    write(ep_events[i].data.fd, buf, str_len);
                }
            }
        }
    }
    close(serv_sock);
    close(epfd);
    return 0;
}

void error_handling(char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}