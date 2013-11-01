#include <unistd.h>
#include <stdio.h>  
#include <dirent.h>  
#include <string.h>  
#include <sys/stat.h>  
#include <stdlib.h>  

//dir为目录名，depth为空格个数

void printdir(char *dir, int depth) 
{ 
    DIR* dp; 
    struct dirent* entry; 
    struct stat statbuf; 

    if(NULL == (dp = opendir(dir))) //打开dir目录
    { 
        fprintf(stderr, "Can`t open directory %s\n", dir); 
        return ; 
    } 
    chdir(dir);//cd dir 
    while(NULL != (entry = readdir(dp)))
    { 
        lstat(entry->d_name, &statbuf);  //获取文件名信息
        if (S_ISDIR(statbuf.st_mode))    //如果为目录
        {  
            if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            { 
                continue;    
            }
            printf("%*s%s/\n", depth, "", entry->d_name); 
            printdir(entry->d_name, depth+4); //为空格个数
        }
        else 
            printf("%*s%s\n", depth, "", entry->d_name); 
    } 
    chdir(".."); //返回上一路径
    closedir(dp); //关闭目录   
} 

int main(void) 
{ 
    char topdir[32] = {0}; 
    printf("plese input scan of Directory: ");
    scanf("%s", topdir); 
    printf("Directory scan of %s\n", topdir); 
    printdir(topdir, 0); 
    printf("done.\n"); 
    return 0; 
}  
