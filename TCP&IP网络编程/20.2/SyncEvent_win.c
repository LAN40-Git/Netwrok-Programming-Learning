#define STR_LEN 100

#include <stdio.h>
#include <windows.h>
#include <process.h>

unsigned WINAPI NumberOfA(void *arg);
unsigned WINAPI NumberOfOthers(void *arg);

static char str[STR_LEN];
static HANDLE hEvent;

int main(int argc, char *argv[])
{
    HANDLE hThread1, hThread2;
    hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    hThread1 = (HANDLE)_beginthreadex(NULL, 0, NumberOfA, NULL, 0, NULL);
    hThread2 = (HANDLE)_beginthreadex(NULL, 0, NumberOfOthers, NULL, 0, NULL);

    fputs("Input string: ", stdout);
    fgets(str, STR_LEN, stdin);
    SetEvent(hEvent); // 设置为signaled

    WaitForSingleObject(hThread1, INFINITE);
    WaitForSingleObject(hThread2, INFINITE);
    ResetEvent(hEvent); // 设置为non-signaled
    CloseHandle(hEvent);
    return 0;
}

unsigned WINAPI NumberOfA(void *arg)
{
    int cnt = 0;
    WaitForSingleObject(hEvent, INFINITE);
    for (int i = 0; str[i] != 0; i++) {
        if (str[i] == 'A')
            cnt++;
    }
    printf("Num of A: %d \n", cnt);
    return 0;
}

unsigned WINAPI NumberOfOthers(void *arg)
{
    int cnt = 0;
    WaitForSingleObject(hEvent, INFINITE);
    for (int i = 0; str[i] != 0; i++) {
        if (str[i] != 'A')
            cnt++;
    }
    printf("Num of others: %d \n", cnt-1);
    return 0;
}