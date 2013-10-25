#ifndef HTTP_MANAGER_H
#define HTTP_MANAGER_H

#include <string>

using namespace std;

struct url_info_t
{
    string net_name;
    string file_name;
    string path;
    string ip;

    url_info_t(){}
    url_info_t(const string& net_name_, const string& file_name_, const string& path_)
        :net_name(net_name_), file_name(file_name_), path(path_){}
    url_info_t(const url_info_t& info_)
        :net_name(info_.net_name), file_name(info_.file_name), path(info_.path), ip(info_.ip){}
    url_info_t& operator= (const url_info_t& other)
    {
        net_name = other.net_name;
        file_name = other.file_name;
        path = other.path;
        ip = other.ip;
    }
};

struct http_head_info_t
{
    int state;
    int file_size;
    int cur_progress;

    http_head_info_t(int state_, int file_size_, int cur_progress_)
        :state(state_), file_size(file_size_), cur_progress(cur_progress_){}
};

class http_info_t
{
public:
    http_info_t(){}

    http_head_info_t http_get_head(int fd);
    static void http_get(int fd, short ev, void* arg);
    string url_ip(const string& url);
    void begin_download();
private:
};

#endif
