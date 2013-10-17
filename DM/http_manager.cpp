#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <iostream>
#include <unistd.h>
#include <time.h>
#include <http_manager.h>

using namespace std;

#define PORT 80
#define BUFFER_SIZE 1024*10

void http_info_t::http_get(int sockcl, char* buffer_)
{
    float version = 0.0;
    int status = 0;
    char dest[301];
    memset(dest,0,sizeof(dest));

    char filename[] = "PLAGame-Win32-Shipping.exe";//"launcher_server.xml";
    char net_name[] = "updategrsm.ztgame.com.cn";
    char path[] = "/PLAGameDis/Binaries/Win32/";

    sprintf(buffer_,"GET %s%s HTTP/1.1\r\nHost:%s\r\nAccept:*/*\r\nConection:Keep-Alive\r\n\r\n",path,filename, net_name);   
    send(sockcl,buffer_,strlen(buffer_),0);

    memset(buffer_,0,sizeof(buffer_));   
    int recv_len = recv(sockcl,buffer_,300,0);
    printf("pack_head_len = %d\n", recv_len);
    printf("recv:\n%s\n********************************\n",buffer_);

    sscanf(strstr(buffer_,"HTTP/"), "HTTP/%f %d", &version, &status);
    sscanf(strstr(buffer_,"Content-Length"),"Content-Length: %d",&total_size);
    printf("status=%d total_size=%d\n", status, total_size);
    if(status != 200 || total_size == 0)
    {
        printf("http connect failed!\n");
        exit(1);
    }
    else
    {   
        strcpy(dest,strstr(buffer_,"\r\n\r\n")+sizeof("\r\n\r\n")-1);
        int pack_head_len = strstr(buffer_,"\r\n\r\n") - buffer_ + sizeof("\r\n\r\n") - 1;
        long progress = recv_len - pack_head_len;
        FILE *fp=fopen(filename,"wb");
        if(NULL==fp)
        {
            printf("File:\t%s Can Not Open To Write\n",filename);
            exit(1);
        }
        fwrite(dest,sizeof(char),strlen(dest),fp);

        char buffer[BUFFER_SIZE];
        bzero(buffer,BUFFER_SIZE);

        struct timespec time_start;
        struct timespec time_end;
        struct timespec total_time_start;
        struct timespec total_time_end;
        clock_gettime(CLOCK_MONOTONIC, &total_time_start); 
        while(1)
        {
            clock_gettime(CLOCK_MONOTONIC, &time_start);
            recv_len = recv(sockcl,buffer,BUFFER_SIZE,0);
            if(recv_len <= 0)
            {
                printf("Recieve Data From Server Failed!\n");
                break;
            }

            int write_length = fwrite(buffer, sizeof(char), recv_len, fp);
            if(write_length < recv_len)
            {
                printf("File:\t%s Write Failed\n", filename);
                break;
            }
            bzero(buffer, BUFFER_SIZE);
            progress += recv_len;

            int percent = progress * 100 / total_size;
            printf("%d%%\[", percent);
            for(int i = 0; i < percent; i++)
                printf("=");
            printf(">]");

            clock_gettime(CLOCK_MONOTONIC, &time_end);
            int use_time_ns = time_end.tv_nsec - time_start.tv_nsec;
            int speed = recv_len * 1000 / (use_time_ns / 1000);
            printf(" recv_len=%d  use_time_ns=%d  %dK/s\n", recv_len, use_time_ns, speed);
            if(progress >= total_size)
                break;
        }
        clock_gettime(CLOCK_MONOTONIC, &total_time_end);
        int total_time_s = total_time_end.tv_sec - total_time_start.tv_sec; 
        printf("Recieve File [%s]\n",filename);
        printf("Recieve Size [%dKb]\n", progress/1000);
        printf("Consume Time [%ds]\n",total_time_s);
        printf("Average Speed[%dKb/s]\n",total_size/1000/total_time_s);
    }   
}
