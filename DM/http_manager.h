#ifndef HTTP_MANAGER_H
#define HTTP_MANAGER_H

class http_info_t
{
    private:
        char* net_name;
        char* file_name;
        char* path;
        int     total_size;
        int     progress;
    public:
        http_info_t(){}

        http_info_t(char* net_name_, char* file_name_, char* path_)
            :net_name(net_name_), file_name(file_name_), path(path_)
        {}
        
        void http_get(int sockcl_, char* buffer_);
};

#endif
