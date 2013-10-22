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
#include <memory.h>
#include <socket_manager.h>
#include <http_manager.h>

#define PORT 80

using namespace std;

sock_info_t g_sock_info;

int main()
{
    http_info_t http_info;
    const char *url_ip = http_info.url_ip("updategrsm.ztgame.com.cn").data();

    g_sock_info.http_init(url_ip, PORT); 
    int fd = g_sock_info.get_cli_sock(); 

    url_info_t url_info;
    url_info.net_name = "updategrsm.ztgame.com.cn";
    url_info.path = "/PLAGameDis/Binaries/Win32/";
    url_info.file_name = "PLAGame-Win32-Shipping.exe";

    http_info_t g_http_info(url_info);
    g_http_info.http_get(fd);
    
    exit(1);
}
