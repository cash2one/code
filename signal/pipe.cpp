#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <sys/wait.h>

int main(int argc, char *argv[])
{
    int pfd[2]; 
    pid_t cpid;
    char buf;
    if(argc != 2)
    {
        fprintf(stderr,"Usage: %s <string>\n",argv[0]);
        exit(0);
    }
    if (pipe(pfd) == -1)
    {
        perror("pipe");
        exit(EXIT_FAILURE);
    }
    cpid = fork();
    if (cpid == -1)
    {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    if (cpid == 0)
    {
        close(pfd[1]);          
        while (read(pfd[0], &buf, 1) > 0)
            write(STDOUT_FILENO, &buf, 1);
        write(STDOUT_FILENO, "\n", 1);
        close(pfd[0]);
        exit(0);
    }
    else
    {
        close(pfd[0]);         
        write(pfd[1], argv[1], strlen(argv[1]));
        close(pfd[1]);
        wait(NULL); 
        exit(0);
    }
}
