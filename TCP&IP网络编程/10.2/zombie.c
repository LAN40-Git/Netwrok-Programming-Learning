#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>

int main(int argc, char *argv[])
{
    pid_t pid = fork();

    if (pid == 0) {// 子进程
        puts("Hi, I am a child process");
    }
    else {         // 父进程
        printf("Child Process ID: %d \n", pid);
        sleep(30); // Sleep 30 sec
    }

    if (pid == 0)
        puts("End child process");
    else
        puts("End parent process");
    return 0;
}