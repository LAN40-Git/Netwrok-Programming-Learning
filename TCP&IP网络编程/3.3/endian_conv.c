#include <stdio.h>
#include <arpa/inet.h>

int main(int argc, char *argv[])
{
    unsigned short host_port=0x1234; // 主机字节序的端口号
    unsigned short net_port; // 网络字节序的端口号
    unsigned long host_addr=0x12345678; // 主机字节序的IP地址
    unsigned long net_addr; // 网络字节徐的IP地址

    net_port = htons(host_port); // 主机字节序的端口号转在网络字节序
    net_addr = htonl(host_addr); // 主机字节序的IP地址转网络字节序

    printf("Host ordered port: %#x \n", host_port);
    printf("Network ordered port: %#x \n", net_port);
    printf("Host ordered address: %#lx \n", host_addr);
    printf("Network ordered address： %#lx \n", net_addr);
    return 0;
}