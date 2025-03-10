# TCP&IP网络编程笔记

[TOC]

## Part01 开始网络编程

### 第1章 理解网络编程和套接字

#### 1.1 理解网络编程和套接字

**网络编程中接收连接请求的套接字过程**：

1. 调用`socket()`创建套接字
2. 调用`bind()`函数分配IP地址和端口号
3. 调用`listen()`函数转为可接受请求状态
4. 调用`accept()`函数受理连接请求



**类似固定电话**

##### 调用`socket()`-安装电话机

**函数原型**

```c
#include <sys/socket.h>
int socket(int domain, int type, int protocol);
```

**参数说明**

1. **`domain`（协议域）**
   指定套接字使用的协议族（地址族），常见值包括：

- `AF_INET`：IPv4 协议（用于互联网通信）。
- `AF_INET6`：IPv6 协议。                                                                
- `AF_UNIX` 或 `AF_LOCAL`：本地通信（Unix 域套接字）。
- `AF_PACKET`：底层数据包接口（用于抓包或自定义协议）。

2. **`type`（套接字类型）**
   指定套接字的类型，常见值包括：

- `SOCK_STREAM`：面向连接的流式套接字（如 TCP）。
- `SOCK_DGRAM`：无连接的数据报套接字（如 UDP）。
- `SOCK_RAW`：原始套接字（用于自定义协议或访问底层网络层）。

3. **`protocol`（协议）**
   指定套接字使用的具体协议，通常设为 `0`，表示根据 `domain` 和 `type` 自动选择默认协议。例如：

- `domain=AF_INET, type=SOCK_STREAM` 时，默认协议是 TCP。
- `domain=AF_INET, type=SOCK_DGRAM` 时，默认协议是 UDP。
  如果需要显式指定协议，可以使用：
- `IPPROTO_TCP`：TCP 协议。
- `IPPROTO_UDP`：UDP 协议。

**返回值**

- **成功**：返回一个 **文件描述符**（非负整数），用于后续操作（如 `bind()`、`connect()`、`send()` 等）。
- **失败**：返回 `-1`，并设置 `errno` 以指示错误原因。

---

##### 调用`bind()`-分配电话号码

**函数原型**

```c
#include <sys/socket.h>
int bind(int sockfd, struct sockaddr *myaddr, socklen_t addrlen);
```

**参数说明**

1. **`sockfd`**

- 要绑定的套接字文件描述符，通常由 `socket()` 函数创建。

2. **`myaddr`**

- 指向 `struct sockaddr` 的指针，包含了要绑定的地址和端口信息。
- 实际使用时，通常使用 `struct sockaddr_in`（IPv4）或 `struct sockaddr_in6`（IPv6）结构体，然后强制转换为 `struct sockaddr*`。

3. **`addrlen`**

- `myaddr` 结构体的长度，通常使用 `sizeof(struct sockaddr_in)` 或 `sizeof(struct sockaddr_in6)`。

**返回值**

- **成功**：返回 `0`。
- **失败**：返回 `-1`，并设置 `errno` 以指示错误原因。

---

##### 调用`listen()`函数-连接电话线

**函数原型**

```c
#include <sys/socket.h>
int listen(int sockfd, int backlog);
```

**参数说明**

1. **`sockfd`**

- 要设置为监听状态的套接字文件描述符，通常由 `socket()` 创建并通过 `bind()` 绑定到本地地址和端口。

2. **`backlog`**

- 等待连接队列的最大长度。
- 当多个客户端同时请求连接时，未处理的连接请求会被放入队列，`backlog` 指定了队列的最大长度。
- 如果队列已满，新的连接请求会被拒绝。
- 通常设置为 `5` 到 `10`，具体值取决于服务器的负载能力。

**返回值**

- **成功**：返回 `0`。
- **失败**：返回 `-1`，并设置 `errno` 以指示错误原因。

---

##### 调用`accept()`-拿起话筒

**函数原型**

```c
#include <sys/socket.h>
int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
```

**参数说明**

1. **`sockfd`**

- 监听套接字的文件描述符，通常由 `socket()` 创建并通过 `bind()` 和 `listen()` 设置为监听状态。

2. **`addr`**

- 指向 `struct sockaddr` 的指针，用于存储客户端的地址信息（如 IP 地址和端口）。
- 如果不需要客户端的地址信息，可以设置为 `NULL`。

3. **`addrlen`**

- 指向 `socklen_t` 的指针，用于存储客户端地址结构体的长度。
- 调用 `accept()` 前，需要将其初始化为 `sizeof(struct sockaddr)`。
- 如果 `addr` 为 `NULL`，可以设置为 `NULL`。

**返回值**

- **成功**：返回一个新的套接字文件描述符，用于与客户端通信。
- **失败**：返回 `-1`，并设置 `errno` 以指示错误原因。

#### 1.2 基于Linux的文件操作

##### 底层文件访问和文件描述符

> 此处为Linux提供的函数