#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<pthread.h>

void* task1(void*);
void* task2(void*);

void usr();
pthread_t pid1, pid2;
pthread_attr_t attr;
int p1,p2;

int main()
{
    usr();
    getchar();
    return 1;
}

void usr()
{
    void *p;
    int ret=0;
    pthread_attr_init(&attr);           //��ʼ���߳����Խṹ
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);   //����attr�ṹΪ����
    pthread_create(&pid1, &attr, task1, NULL);         //�����̣߳������̺߳Ÿ�pid1,�߳���������Ϊattr�����ԣ��̺߳������Ϊtask1������ΪNULL
    pthread_create(&pid2, &attr, task2, NULL);  
    //pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    //pthread_create(&pid2, &attr, task2, NULL);
    //ǰ̨����
    //ret=pthread_join(pid2, &p);         //�ȴ�pid2���أ�����ֵ����p
    printf("after pthread2:ret=%d,p=%d\n", ret,(intptr_t)p);          
}

void* task1(void *arg1)
{
    printf("thread1 begin\n");
    //�����޷�Ԥ�ϵĹ���������Ϊ�����̣߳�������������
    sleep(3);
    pthread_create(&pid1, &attr, task1, NULL);         
    pthread_exit( (void *)1);
}

void* task2(void *arg2)
{
    printf("thread2 begin.\n");
    sleep(4);
    pthread_create(&pid2, &attr, task2, NULL);
    pthread_exit((void *)2);
}
