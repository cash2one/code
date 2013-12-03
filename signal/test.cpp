#include <stdio.h>
#include <signal.h>
#include <cstdlib>
#include <unistd.h>

void HandleSigint(int signo) //callback
{
    printf("receive signal %d\n",signo);
}

int main()
{
    if(signal(SIGINT,HandleSigint)  == SIG_ERR) // set signal callback
    {
        perror("signal");
        exit(0);
    }
    pause();
}
