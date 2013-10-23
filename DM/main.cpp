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
#include <event.h>
#include <assert.h>

#define PORT 80

using namespace std;

sock_info_t g_sock_info;
event_base  *g_ev_base;
event g_down_ev;
url_info_t g_url_info;
FILE *cur_fp;

int main()
{
    http_info_t http_info;
    const char *url_ip = http_info.url_ip("updategrsm.ztgame.com.cn").data();

    g_sock_info.http_init(url_ip, PORT); 
    int fd = g_sock_info.get_cli_sock(); 
/*
    g_ev_base = event_init();
    assert(g_ev_base != NULL);
    event_set(&g_down_ev, fd, EV_READ | EV_PERSIST, http_info_t::http_get, NULL);
    event_add(&g_down_ev, NULL);
*/
    //event_base_dispatch(g_ev_base);

    g_url_info.net_name = "updategrsm.ztgame.com.cn";
    g_url_info.path = "/PLAGameDis/Binaries/Win32/";
    g_url_info.file_name = "PLAGame-Win32-Shipping.exe";

    http_info_t g_http_info;
    http_head_info_t head = g_http_info.http_get_head(fd);
    g_http_info.http_get(fd,1, (void*)&head);
    
    exit(1);
}
