#include <unistd.h>

int main(void)
{
    pid_t pid;

    /* Fork off the parent process */
    pid = fork();

    /* Did fork() fail? */
    if(pid < 0)
    {
        return 1;
    }
    /* Child */
    if (pid > 0)
    {
        return 0;
    }

    /* Parent */
    return 0;
}
