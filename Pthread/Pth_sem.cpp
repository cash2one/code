// wty:2012/10/13   Pth_sem
//
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define THREAD_NUM 3
#define REPEAT_TIMES 5
#define DELAY 4

sem_t sem[THREAD_NUM];

void *thrd_func(void *arg);

int main()
{
    pthread_t thread[THREAD_NUM];
    int no;
    void *tret;

    srand((int)time(0)); 
    sem_init(&sem[0],0,0);
    sem_init(&sem[1],0,0);
    sem_init(&sem[2],0,1);
    for(no=0;no<THREAD_NUM;no++)
    {
        // wty: (void *)no 换成 &no的话,3个线程共用一个no
        // 线程的互斥会导致传入线程函数的形参错误
        // (void *)的话传入的形参是拷贝,不会受到线程机制影响
        if(pthread_create(&thread[no],NULL,thrd_func,(void *)no)) 
        {
            printf("Create thread %d error!\n",no);
            exit(1);
        }
        else
        {
            printf("Create thread %d success!\n",no);
        }
    }
    for(no=0;no<THREAD_NUM;no++)
    {
        if(pthread_join(thread[no],&tret)!=0)
        {
            printf("Join thread %d error!\n",no);
            exit(1);
        }
        else
        {
            printf("Join thread %d success!\n",no);
        }
    }
    for(no=0;no<THREAD_NUM;no++)
    {
        sem_destroy(&sem[no]);
    }

    return 0;
}

void *thrd_func(void *arg)
{
    // wty: intptr_t 防止精度溢出
    int thrd_num = (intptr_t)arg; 
    int delay_time,count;

    // 带有阻塞的p操作
    sem_wait(&sem[thrd_num]);
    printf("Thread %d is starting.\n",thrd_num);
    for(count=0;count<REPEAT_TIMES;count++) 
    {
        delay_time=(int)(DELAY*(rand()/(double)RAND_MAX))+1;
        sleep(delay_time);
        printf("\tThread %d:job %d delay =%d.\n",thrd_num,count,delay_time);
    }
    printf("Thread %d is exiting.\n",thrd_num);
    // 对前一个信号量进行V操作
    // 由于只有最后一个信号量初始化为1，其余均为0
    // 故线程执行的顺序将为逆序
    //sem_post(&sem[thrd_num - 1]);    // 为什么不这样写
    sem_post(&sem[(thrd_num+THREAD_NUM-1)%THREAD_NUM]);   
    // 线程主动结束
    pthread_exit(NULL); 
}
