#define NUM_THREAD 100

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
void *thread_inc(void *arg);
void *thread_des(void *arg);

long long num = 0;
pthread_mutex_t mutex;

int main(int argc, char *argv[])
{
    pthread_t thread_id[NUM_THREAD];
    // 初始化互斥量
    pthread_mutex_init(&mutex, NULL);

    for (int i = 0; i < NUM_THREAD; i++) {
        if (i % 2 == 0) 
            pthread_create(&thread_id[i], NULL, thread_inc, NULL);
        else
            pthread_create(&thread_id[i], NULL, thread_des, NULL);
    }

    for (int i = 0; i < NUM_THREAD; i++)
        pthread_join(thread_id[i], NULL);

    printf("result: %lld \n", num);
    // 摧毁互斥量
    pthread_mutex_destroy(&mutex);
    return 0;
}

void *thread_inc(void *arg)
{
    pthread_mutex_lock(&mutex);
    for (int i = 0; i < 50000000; i++)
        num += 1;
    pthread_mutex_unlock(&mutex);
    return NULL;
}

void *thread_des(void *arg)
{
    pthread_mutex_lock(&mutex);
    for (int i = 0; i < 50000000; i++)
        num -= 1;
    pthread_mutex_unlock(&mutex);
    return NULL;
}