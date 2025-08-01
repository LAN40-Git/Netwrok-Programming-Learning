#define BUF_SIZE 4
#define EPOLL_SIZE 50

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/epoll.h>

void setnonblockingmode(int fd);
void error_handling(char *message);

int main(int argc, char *argv[])
{
    int serv_sock, clnt_sock;
    struct sockaddr_in serv_addr, clnt_addr;
    int str_len;
    socklen_t addr_sz;
    char buf[BUF_SIZE];

    struct epoll_event *ep_events; // 监听对象中发生对应事件变化的对象缓存
    struct epoll_event event;
    int epfd, event_cnt;

    if (argc != 2) {
        printf("Usage : %s <port>\n", argv[0]);
        exit(1);
    }

    serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    if (serv_sock == -1)
        error_handling("socket() error!");
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(atoi(argv[1]));

    if (bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
        error_handling("bind() error!");
    if (listen(serv_sock, 5) == -1)
        error_handling("listen() error!");

    epfd = epoll_create(EPOLL_SIZE);
    ep_events = malloc(sizeof(struct epoll_event)*EPOLL_SIZE);

    setnonblockingmode(serv_sock);
    event.events = EPOLLIN;
    event.data.fd = serv_sock;
    // 将服务端套接字添加到epoll实例的监听对象，并监听event指定的事件
    epoll_ctl(epfd, EPOLL_CTL_ADD, serv_sock, &event);

    while(1) {
        event_cnt = epoll_wait(epfd, ep_events, EPOLL_SIZE, -1);
        if (event_cnt == -1) {
            puts("epoll_wait() error!");
            break;
        }

        puts("return epoll_wait");
        for (int i = 0; i < event_cnt; i++) {
            if (ep_events[i].data.fd == serv_sock) {
                addr_sz = sizeof(clnt_addr);
                clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, &addr_sz);
                if (clnt_sock == -1)
                    error_handling("accept() error!");
                setnonblockingmode(clnt_sock); // 设置客户端套接字为非阻塞
                event.data.fd = clnt_sock;
                event.events = EPOLLIN|EPOLLET; // 将客户端套接字注册方式改为边缘触发
                epoll_ctl(epfd, EPOLL_CTL_ADD, clnt_sock, &event);
                printf("client connected: %d \n", clnt_sock);
            } else {
                while(1) {
                    str_len = read(ep_events[i].data.fd, buf, BUF_SIZE);
                    if (str_len == 0) {
                        epoll_ctl(epfd, EPOLL_CTL_DEL, ep_events[i].data.fd, NULL);
                        close(ep_events[i].data.fd);
                        printf("closed client: %d \n", ep_events[i].data.fd);
                    } else if (str_len < 0) {
                        if (errno == EAGAIN)
                            break;
                    } else {
                        write(ep_events[i].data.fd, buf, str_len);
                    }
                }
                
            }   
        }
    }
    close(serv_sock);
    close(epfd);
    return 0;
}

void setnonblockingmode(int fd)
{
    int flag = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flag|O_NONBLOCK);
}

void error_handling(char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}