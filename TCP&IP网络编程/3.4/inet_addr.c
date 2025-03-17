#include <stdio.h>
#include <arpa/inet.h>

int main(int argc, char *string)
{
    char *addr1="1.2.3.4";
    char *addr2="1.2.3.256";

    // 将点分十进制的字符串转换为32位整数型IP地址
    unsigned long conv_addr = inet_addr(addr1);

    if (conv_addr == INADDR_NONE)
        printf("Error occurred!\n");
    else
        printf("%#lx \n", conv_addr);

    conv_addr = inet_addr(addr2);
    
    if (conv_addr == INADDR_NONE)
        printf("Error occurred!\n");
    else
        printf("%#lx \n", conv_addr);
    return 0;
}