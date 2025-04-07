#define NUM_THREAD 50

#include <stdio.h>
#include <windows.h>
#include <process.h>

unsigned WINAPI threadInc(void *arg);
unsigned WINAPI threadDes(void *arg);
long long num = 0;

int main(int argc, char *argv[])
{
    HANDLE tHandles[NUM_THREAD];

    printf("sizeof long long: %d \n", sizeof(long long));
    for (int i = 0; i < NUM_THREAD; i++) {
        if (i % 2)
            tHandles[i] = (HANDLE)_beginthreadex(NULL, 0, threadInc, NULL, 0, NULL);
        else
            tHandles[i] = (HANDLE)_beginthreadex(NULL, 0, threadDes, NULL, 0, NULL);
    }

    WaitForMultipleObjects(NUM_THREAD, tHandles, TRUE, INFINITE);
    printf("result: %lld \n", num);
    for (int i = 0; i < NUM_THREAD; i++)
        CloseHandle(tHandles[i]);
    return 0;
}

unsigned WINAPI threadInc(void *arg)
{
    for (int i = 0; i < 50000000; i++) {
        num += 1;
    }
    return 0;
}

unsigned WINAPI threadDes(void *arg)
{
    for (int i = 0; i < 50000000; i++) {
        num -= 1;
    }
    return 0;
}