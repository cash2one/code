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
    char recvbuf[301];
    char filename[50];
    struct sockaddr_in cl_addr;
    float version = 0.0;
    int status = 0;
    int length = 0;
    char dest[250];
    char ip[50],path[30];
    //char ss[20],sx[20];
    memset(dest,0,sizeof(dest));
    memset(ip,0,sizeof(ip));
    memset(path,0,sizeof(path));
    //memset(ss,0,sizeof(ss));
    //memset(sx,0,sizeof(sx));

    if((sockcl=socket(AF_INET,SOCK_STREAM,0))==-1)
    {
        printf("socket err!\n");
        exit(1);
    }
    bzero(&cl_addr,sizeof(cl_addr));
    printf("Please input the server ip:");
    scanf("%s",ip);
    cl_addr.sin_family=AF_INET;
    cl_addr.sin_port=htons(PORT);
    //cl_addr.sin_addr.s_addr=inet_addr("219.219.218.213");
    cl_addr.sin_addr.s_addr=inet_addr(ip);
    if(connect(sockcl,(struct sockaddr*)(&cl_addr),sizeof(cl_addr))==-1)
    {
        printf("connect err!\n");
        exit(1);
    }
    printf("connected!\n");
    printf("Please input the download path:");
    scanf("%s",path);
    printf("Please input the file name:");
    scanf("%s",filename);
    sprintf(recvbuf,"GET %s%s HTTP/1.1\r\nHost:219.219.218.21\r\nAccept:*/*\r\nConection:Keep-Alive\r\n\r\n",path,filename);   
    send(sockcl,recvbuf,strlen(recvbuf),0);
    //printf("http:\n%s\n",recvbuf);

    memset(recvbuf,0,sizeof(recvbuf));   
    recv(sockcl,recvbuf,300,0);
    printf("recv:\n%s\n********************************\n",recvbuf);

    sscanf(strstr(recvbuf,"HTTP/"),"HTTP/%f %d",&version,&status);
    //sscanf(strstr(recvbuf,"Content-Type"),"Content-Type: %s\r\n%s",sx,ss);
    sscanf(strstr(recvbuf,"Content-Length"),"Content-Length: %d",&length);
    //printf("status=%d\ntype=%s\nss=%s----------------\n",status,sx,ss);
    printf("status=%d length=%d\n",status,length);
    if(status != 200 || length == 0)
    {
        printf("http connect failed!\n");
        exit(1);
    }
    else
    {   
        //printf("**\n%s\n**",strstr(recvbuf,"\r\n\r\n")+strlen("\r\n\r\n"));
        //printf("**\n%s\n**",strstr(recvbuf,"\r\n\r\n")+sizeof("\r\n\r\n")-1);
        strcpy(dest,strstr(recvbuf,"\r\n\r\n")+sizeof("\r\n\r\n")-1);
        //printf("dest:%s\n",dest);
        printf("file name:%s\n",filename);
        char buffer[BUFFER_SIZE];
        //FILE *fp=fopen(file_name,"w");
        FILE *fp=fopen(filename,"wb");
        if(NULL==fp)
        {
            printf("File:\t%s Can Not Open To Write\n",filename);
            exit(1);
        }
        fwrite(dest,sizeof(char),strlen(dest),fp);
        bzero(buffer,BUFFER_SIZE);
        length=0;
        while(length=recv(sockcl,buffer,BUFFER_SIZE,0))
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
