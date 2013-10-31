#ifndef FILE_H
#define FILE_H 

#include <fstream>
#include <string>
#include <sys/stat.h>
#include <stdio.h>

using namespace std;

class file
{
public:
    static void make_dir(const char* dir_name);
    static void write_file(const char* file_name, const char* msg);
    static void read_file(const char* file_name);
    static void copy_file(const char* org_name, const char* des_name);
    static void insert_word(const char* org_name, const char* des_name,\
            const char* new_word, const char* des_word);
    static void insert_word(const char* file_name, const char* new_word, const char* des_word);
    static void replace_word(const char* org_name, const char* des_name,\
            const char* new_word, const char* des_word);
    static void replace_word(const char* file_name, const char* new_word, const char* des_word);
    static void delete_file(const char* file_name);
};

#endif
