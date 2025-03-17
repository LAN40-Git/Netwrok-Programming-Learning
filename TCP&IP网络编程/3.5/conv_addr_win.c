#undef UNICODE
#undef _UNICODE
#include <stdio.h>
#include <winsock2.h>

int main(int argc, char *argv[])
{
    char *strAddr="203.211.218.102:9190";

    char strAddrBuf[50];
    SOCKADDR_IN servAddr; // 相当于struct sockaddr_in servAddr;
    int size;
    unsigned long size2;

    WSADATA wsaData;
    WSAStartup(MAKEWORD(2,2), &wsaData);

    size=sizeof(servAddr);
    WSAStringToAddress(strAddr, AF_INET, NULL, (SOCKADDR*)&servAddr, &size);
    printf("First conv result: %d \n", strAddrBuf);

    size2=sizeof(strAddrBuf);
    WSAAddressToString((SOCKADDR*)&servAddr, sizeof(servAddr), NULL, strAddrBuf, &size2);
    printf("Second conv result: %s \n", strAddrBuf);
    WSACleanup();
    return 0;
}