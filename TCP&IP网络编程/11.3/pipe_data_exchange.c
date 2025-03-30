#define BUF_SIZE 30
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>

int main(int argc, char *argv[])
{
    char buf[BUF_SIZE];
    pid_t pid;
    int fds1[2], fds2[2];

    pipe(fds1), pipe(fds2);
    pid = fork();
    if (pid == 0) {
        int i;
        for (i = 0; i < 3; i++) {
            read(fds1[0], buf, BUF_SIZE);
            printf("Childproc receive message%d: %s \n", i+1, buf);
            snprintf(buf, BUF_SIZE, "Child's message%d", i+1);
            write(fds2[1], buf, sizeof(buf));
        }
    }
    else {
        int i;
        for (i = 0; i < 3; i++) {
            snprintf(buf, BUF_SIZE, "Parent's message%d", i+1);
            write(fds1[1], buf, sizeof(buf));
            read(fds2[0], buf, BUF_SIZE);
            printf("Parentproc receive message%d: %s \n", i+1, buf);
        }
    }
    return 0;
}