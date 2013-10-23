#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <sys/queue.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/types.h>
#include <iostream>
#include <unistd.h>
#include <time.h>
#include <http_manager.h>
#include <event.h>

extern event_base g_ev_base;
extern url_info_t g_url_info;
extern FILE *cur_fp;

using namespace std;

#define PORT 80
#define BUFFER_SIZE 4096

string http_info_t::url_ip(const string& url)
{
    //  域名ip转换 分离
    struct hostent *host = gethostbyname(url.data());
    string ret(inet_ntoa(*((struct in_addr *)host->h_addr)));
    printf("IP Address:%s\n", ret.data());

    return ret;
}

void http_info_t::begin_download()
{     
}

http_head_info_t http_info_t::http_get_head(int fd)
{
    char head_buf[301];
    float version = 0.0;
    int state = 0;
    char dest[301];
    memset(dest,0,sizeof(dest));
    const char *file_name = g_url_info.file_name.data();
    int total_size;

    sprintf(head_buf,"GET %s%s HTTP/1.1\r\nHost:%s\r\nAccept:*/*\r\nConection:Keep-Alive\r\n\r\n", \
            g_url_info.path.data(), file_name, g_url_info.net_name.data());
    send(fd,head_buf,strlen(head_buf),0);

    memset(head_buf,0,sizeof(head_buf));   
    int recv_len = recv(fd,head_buf,300,0);
    printf("pack_head_len = %d\n", recv_len);
    printf("recv:\n%s\n********************************\n",head_buf);

    sscanf(strstr(head_buf,"HTTP/"), "HTTP/%f %d", &version, &state);
    sscanf(strstr(head_buf,"Content-Length"),"Content-Length: %d",&total_size);
    printf("state=%d total_size=%d\n", state, total_size);
    if(state != 200 || total_size == 0)
    {
        printf("http connect failed!\n");
        exit(1);
    }
    strcpy(dest,strstr(head_buf,"\r\n\r\n")+sizeof("\r\n\r\n")-1);
    int pack_head_len = strstr(head_buf,"\r\n\r\n") - head_buf + sizeof("\r\n\r\n") - 1;
    long progress = recv_len - pack_head_len;
    cur_fp = fopen(g_url_info.file_name.data(),"wb");
    if(NULL==cur_fp)
    {
        printf("File:\t%s Can Not Open To Write\n",file_name);
        exit(1);
    }
    fwrite(dest,sizeof(char),strlen(dest), cur_fp);

    http_head_info_t head(state, total_size, progress); 
    return head;
}

void http_info_t::http_get(int fd, short ev, void* arg)
{
    http_head_info_t *head = (http_head_info_t*)arg;
    char buffer[BUFFER_SIZE];
    bzero(buffer,BUFFER_SIZE);

    struct timespec time_start;
    struct timespec time_end;
    struct timespec total_time_start;
    struct timespec total_time_end;
    clock_gettime(CLOCK_MONOTONIC, &total_time_start); 
    int percent = 0;
    int percent_o = -1;
    long progress = head->cur_progress;
    int total_size = head->file_size;
    while(1)
    {
        clock_gettime(CLOCK_MONOTONIC, &time_start);
        int recv_len = recv(fd,buffer,BUFFER_SIZE,0);
        if(recv_len <= 0)
        {
            printf("Recieve Data From Server Failed!\n");
            break;
        }

        int write_length = fwrite(buffer, sizeof(char), recv_len, cur_fp);
        if(write_length < recv_len)
        {
            printf("File:\t%s Write Failed\n", g_url_info.file_name.data());
            break;
        }
        bzero(buffer, BUFFER_SIZE);
        progress += recv_len;

        percent = progress * 100 / total_size;
        int bar = progress * 150 / total_size;
        if(percent <= percent_o)
            continue;

        printf("%d%%\[", percent);
        for(int i = 0; i <= 150; i++)
        {
            if(i == bar)
                printf(">");
            else if(i < bar)
                printf("=");
            else
                printf(" ");
        }
        printf("]");

        clock_gettime(CLOCK_MONOTONIC, &time_end);
        int use_time_ns = time_end.tv_nsec - time_start.tv_nsec;
        int speed = recv_len * 1000 / (use_time_ns / 1000);
        printf(" recv_len=%d\buse_time_ns=%d\b%dK/s\r", recv_len, use_time_ns, speed);
        percent_o = percent;
        fflush ( stdout ) ;
        // download finish
        if(progress >= total_size)
        {
            printf("\n");
            break;
        }
    }
    clock_gettime(CLOCK_MONOTONIC, &total_time_end);
    int total_time_s = total_time_end.tv_sec - total_time_start.tv_sec; 
    printf("Recieve File [%s]\n",g_url_info.file_name.data());
    printf("Recieve Size [%dKb]\n", progress/1000);
    printf("Consume Time [%ds]\n",total_time_s);
    printf("Average Speed[%dKb/s]\n",total_size/1000/total_time_s);
}
