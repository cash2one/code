#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <iostream>
#include <unistd.h>
#include <log.h>
#include <socket_manager.h>
#include <http_manager.h>

#define PORT 80
#define BUFFER_SIZE 1024

using namespace std;

sock_info_t g_sock_info;
http_info_t g_http_info;

int main()
{
    char recvbuf[301];
    struct sockaddr_in cl_addr;
    float version = 0.0;
    int status = 0;
    int total_size = 0;
    char dest[301];
    int sockcl;
    
    memset(dest,0,sizeof(dest));

    // socket init
    char ip[] = "218.92.1.21";
    g_sock_info.http_init(ip, PORT); 
    sockcl = g_sock_info.get_cli_sock(); 
    // http download
    g_http_info.http_get(sockcl, recvbuf);

    exit(1);
}
