#include <stdio.h>
#include <fcntl.h>

int main(int argc, char *argv[])
{
    FILE *fp; // file pointer
    int fd;   // file describer
    fd = open("data.dat", O_WRONLY|O_CREAT|O_TRUNC);
    if (fd == -1) {
        fputs("file open error!", stdout);
        return -1;
    }

    fp = fdopen(fd, "w"); // 将文件描述符转换为FILE结构体指针
    fputs("Network C programming \n", fp);
    fclose(fp); // 必须先关闭FILE*，否则会导致刷新操作失败
    return 0;
}