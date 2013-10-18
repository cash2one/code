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
};

class http_info_t
{
private:
    url_info_t url_info;
    int   total_size;
    int   progress;
public:
    http_info_t(){}

    http_info_t(url_info_t url_info_)
        :url_info(url_info_){}
        
    void http_get(int sockcl_);
    void begin_download();
};

#endif
