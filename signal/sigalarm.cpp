#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
int timeout;
void Handle_Alarm(int signo)
{
    timeout = 1;
    printf("SIGALRM received.\n");
}
main()
{
    if(signal(SIGALRM,Handle_Alarm) ==SIG_ERR )
    {
        perror("signal");
        exit(0);
    }
    timeout = 0;
    alarm(10);
    pause();
    if(timeout)
    {
        printf("Pause time out.\n");
    }
}
