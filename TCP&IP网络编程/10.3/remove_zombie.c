#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

void read_childproc(int sig)
{
    int status;
    pid_t id = waitpid(-1, &status, WNOHANG);
    if (WIFEXITED(status)) {
        printf("Removed proc id: %d \n", id);
        printf("Child send: %d \n", WEXITSTATUS(status));
    }
}

int main(int argc, char *argv[])
{
    pid_t pid;
    struct sigaction act;
    act.sa_handler = read_childproc;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGCHLD, &act, 0);

    pid = fork();
    if (pid == 0) { /*子进程执行区域*/
        puts("Hi! I'm child process");
        sleep(10);
        return 12;
    }
    else { /*父进程执行区域*/
        printf("Child proc id: %d \n", pid);
        pid = fork();
        if (pid == 0) { /*另一子进程执行区域*/
            puts("Hi! I'm child process");
            sleep(10);
            exit(24);
        }
        else {
            int i;
            printf("Child proc id: %d \n", pid);
            for (i = 0; i < 5; i++) {
                puts("wait...");
                sleep(5);
            }
        }
    }
    return 0;
}