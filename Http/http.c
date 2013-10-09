#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/wait.h>
#define PORT 80
#define BUFFER_SIZE 1024

int main()
{
    int sockcl;
    char recvbuf[BUFFER_SIZE];
    struct sockaddr_in cl_addr;
    float version = 0.0;
    int status = 0;
    int length = 0;

    if((sockcl=socket(AF_INET,SOCK_STREAM,0))==-1)
    {
        printf("socket err!\n");
        exit(1);
    }
    bzero(&cl_addr,sizeof(cl_addr));
    char ip[] = "218.92.1.21";
    char path[] = "/xml/";
    char filename[] = "launcher_server.xml";
    char net_name[] = "updategrsm.ztgame.com.cn";
    cl_addr.sin_family=AF_INET;
    cl_addr.sin_port=htons(PORT);
    cl_addr.sin_addr.s_addr=inet_addr(ip);
    if(connect(sockcl,(struct sockaddr*)(&cl_addr),sizeof(cl_addr))==-1)
    {
        printf("connect err!\n");
        exit(1);
    }
    printf("connected!\n");
    
    sprintf(recvbuf,"GET %s%s HTTP/1.1\r\nHost:%s\r\nAccept:*/*\r\nConection:Keep-Alive\r\n\r\n",path,filename, net_name);
    send(sockcl,recvbuf,strlen(recvbuf),0);
    printf("http:\n%s\n",recvbuf);

    memset(recvbuf,0,sizeof(recvbuf));   
    recv(sockcl,recvbuf,BUFFER_SIZE,0);
    printf("recv:\n%s\n********************************\n",recvbuf);

    sscanf(strstr(recvbuf,"HTTP/"),"HTTP/%f %d",&version,&status);
    sscanf(strstr(recvbuf,"Content-Length"),"Content-Length: %d",&length);
    printf("status=%d length=%d\n",status,length);
    if(status != 200 || length == 0)
    {
        printf("http connect failed!\n");
        exit(1);
    }
    else
    {  
        char *pack_body = strstr(recvbuf,"\r\n\r\n") + sizeof("\r\n\r\n") - 1;
        int pack_head_length = pack_body - recvbuf;
        printf("pack_head_length=%d\n", pack_head_length);
        char dest[BUFFER_SIZE - pack_head_length];
        memset(dest,0,sizeof(dest));

        strcpy(dest, pack_body);
        //printf("file name:%s\n",filename);
        printf("caonima");
        FILE *fp=fopen(filename,"wb");
        printf("1111111111111");
        if(NULL==fp)
        {
            printf("File:\t%s Can Not Open To Write\n",filename);
            exit(1);
        }
        fwrite(dest, sizeof(char), sizeof(dest),fp);

        printf("2222222222"); 
        char buffer[BUFFER_SIZE];
        bzero(buffer, length);
        //length=0;
        printf("status=%d length=%d\n",status,length);
        while(length=recv(sockcl, buffer, BUFFER_SIZE, 0))
        {
            if(length<0)
            {
                printf("Recieve Data From Server [%s] Failed!\n",ip);
                break;
            }
            //int write_length = write(fp, buffer,length);
            int write_length=fwrite(buffer,sizeof(char),length,fp);
            if(write_length<length)
            {
                printf("File:\t%s Write Failed\n",filename);
                break;
            }
            bzero(buffer,BUFFER_SIZE);
        }
        printf("Recieve File:%s From Server [%s] Finished\n",filename,ip);
    }   
    close(sockcl);
    exit(1);
}
