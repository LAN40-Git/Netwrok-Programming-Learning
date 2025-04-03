#define EPOLL_SIZE 50
#define BUF_SIZE 1024

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/epoll.h>

void error_handling(char *message);
void setnonblockingmode(int fd);

int main(int argc, char *argv[])
{
    int serv_sock, clnt_sock;
    struct sockaddr_in serv_addr, clnt_addr;
    socklen_t clnt_addr_sz;
    char buf[BUF_SIZE];
    int str_len;

    struct epoll_event *ep_events;
    struct epoll_event event;
    int epfd, event_cnt;

    if (argc != 2) {
        printf("Usage : %s <port>\n", argv[0]);
        exit(1);
    }

    // 创建服务端套接字
    serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    if (serv_sock == -1)
        error_handling("socket() error!");
    // 分配服务端地址信息
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(atoi(argv[1]));

    // 将服务端套接字和地址绑定
    if (bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
        error_handling("bind() error!");
    // 将服务端套接字设置为监听状态
    if (listen(serv_sock, 5) == -1)
        error_handling("listen() error!");

    // 创建epoll实例
    epfd = epoll_create(EPOLL_SIZE);
    // 动态分配监听对象缓存结构体
    ep_events = malloc(sizeof(struct epoll_event)*EPOLL_SIZE);

    // 将serv_sock设置为非阻塞
    setnonblockingmode(serv_sock);
    // 将serv_sock添加到epoll实例中，并监听event指定的事件
    event.data.fd = serv_sock;
    event.events = EPOLLIN;
    epoll_ctl(epfd, EPOLL_CTL_ADD, serv_sock, &event);

    while(1) { // 服务端开始服务，等待被服务对象
        // 获取epfd实例中发生事件的监听对象数量，并将这些对象缓存到ep_events中
        event_cnt = epoll_wait(epfd, ep_events, EPOLL_SIZE, -1);
        if (event_cnt == -1) {
            puts("epoll_wait() error!");
            break;
        }

        // 处理发生事件的监听对象
        for (int i = 0; i < event_cnt; i++) {
            if (ep_events[i].data.fd == serv_sock) { // 有客户端请求连接
                // 处理客户端连接请求
                clnt_addr_sz = sizeof(clnt_addr);
                clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, &clnt_addr_sz);
                if (clnt_sock == -1) {
                    puts("connect() error!");
                    continue;
                }
                // 将clnt_sock设置为非阻塞
                setnonblockingmode(clnt_sock);
                // 将客户端套接字添加到epoll实例中，并监听event指定的事件
                event.data.fd = clnt_sock;
                event.events = EPOLLIN;
                epoll_ctl(epfd, EPOLL_CTL_ADD, clnt_sock, &event);
                printf("client connected: %d \n", clnt_sock);
            } else {
                while(1) {
                    str_len = read(ep_events[i].data.fd, buf, BUF_SIZE);
                    if (str_len == 0) { // 客户端请求断开连接
                        // 将对应客户端套接字从epoll实例的监听对象中移除
                        epoll_ctl(epfd, EPOLL_CTL_DEL, ep_events[i].data.fd, NULL);
                        // 关闭对应的客户端套接字
                        close(ep_events[i].data.fd);
                        printf("closed client: %d \n", ep_events[i].data.fd);
                    } else if (str_len < 0) {
                        // 当前客户端套接字中没有数据可读，直接跳出循环
                        if (errno == EAGAIN)
                            break;
                    } else { // 与客户端对话
                        buf[str_len] = '\0';
                        printf("Message from client %d: %s \n", ep_events[i].data.fd, buf);
                        // 发送消息给客户端
                        fgets(buf, BUF_SIZE, stdin);
                        write(ep_events[i].data.fd, buf, sizeof(buf));
                    }
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

void setnonblockingmode(int fd)
{
    int flag = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flag | O_NONBLOCK);
}