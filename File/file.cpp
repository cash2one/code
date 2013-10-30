#include "file.h"
#include <iostream>

using namespace std;

void file::make_dir(const char* dir_name)
{
    mkdir(dir_name, 0755);
}

void file::write_file(const char* file_name, const char* msg)
{
    ofstream ofs;
    ofs.open(file_name, ios::app);
    ofs << msg << endl;
    ofs.close();
}

void file::read_file(const char* file_name)
{
    string test;
    ifstream ifs(file_name);
    while(getline(ifs, test))
    {
        int loc = test.find("Top");
        if(loc > 0)
        {
            cout << "find in " << loc << endl;
            test.insert(loc + strlen("Top"), "<<WTY>>");
        }

        cout << test << endl;
    }
}

void file::copy_file(const char* org_name, const char* des_name)
{
    string read_buf;
    ifstream ifs(org_name);
    ofstream ofs;
    ofs.open(des_name, ios::app);
    while(getline(ifs, read_buf))
    {
        ofs << read_buf << endl;
    }
    ofs.close();
}

void file::insert_word(const char* org_name, const char* des_name,\
    const char* new_word, const char* des_word)
{
    string read_buf;
    ifstream ifs(org_name);
    ofstream ofs;
    ofs.open(des_name, ios::app);
    while(getline(ifs, read_buf))
    {
        int loc = read_buf.find(des_word);
        if(loc >0)
        {
            read_buf.insert(loc + strlen(des_word), new_word);
        }
        ofs << read_buf << endl;
    }
    ofs.close();
}

void file::insert_word(const char* file_name, const char* new_word, const char* des_word)
{
    insert_word(file_name, "temp", new_word, des_word);
    remove(file_name);
    copy_file("temp", file_name);
    remove("temp");
}

void file::delete_file(const char* file_name)
{
    remove(file_name); 
}
