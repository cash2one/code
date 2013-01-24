#include<iostream>
#include<unistd.h>
#include<fcntl.h>
#include<string>
#include<stdio.h>

using namespace std;

void test(char str[100])
{
    cout << str << endl;
}

int main(int argc,char *argv[])
{
    if(access(argv[1],F_OK) != 0)
    {
        cout << "file not found" << endl;
        return 0;
    }
    int fin = open(argv[1],O_RDONLY,0777);
    int fout = open(argv[2],O_WRONLY,0777);
    char buff[1024] = {'\0'};
    int len = 0;
    while((len = read(fin,buff,sizeof(buff))) > 0)
    {
        write(fout,buff,len);
    }

    close(fin);
    close(fout);
/*
    int fa;
    string str = "hello world";
    
    fa = open("data.txt",O_RDWR|O_CREAT|O_TRUNC);
    //write(fa,str.c_str(),str.length());
    write(fa,str.c_str(),str.length());
    if(write(fa,str2.c_str(),str2.length()) != str2.length())
    {
        cout << "write data error" << endl;
    }
    cout << "1:" << fa << endl;
    close(fa);
    cout << "2:" << fa << endl;
    fa = open("data.txt".O_RDONLY);
    cout << "3:" << fa << endl;
    while((num = read(fa,buffer,10))>0)
    {
    }

    close(fa);
    cout << "4:" << fa << endl;
*/
    return 0;
}
