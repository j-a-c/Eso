#include <sys/stat.h>
#include <unistd.h>

/**
 * Starts the daemon that will run on the end hosts.
 */
int main(void)
{
    // Fork the parent process and have the parent exit.
    if (pid_t pid = fork())
    {
        // Did fork() fail?
        if(pid < 0)
        {
            // First fork failed.
            return 1;
        }
        else // Fork was successful.
        {
            return 0;
        }
    }

    // We are now in the child process.

    // Detach process from its controlling terminal and make it a 
    // process group leader.
    setsid();

    // A process inherits working directory from its parent.
    // We need to changes the root directory to avoid some problems.
    // chroot("/new/root/directory"); // requires superuser privileges
    chdir("/");

    // Clear the file mode creation mask.
    umask(0);
    
    // Fork a second time to ensure process cannot acquire a controlling
    // terminal.
    if (pid_t pid = fork())
    {
        if (pid < 0)
        {
            // Second fork failed
            return 1;
        }
        else
            return 0;
    }

    // We are now in the grandchild process, which is truly daemonized.
    // init is in charge of process cleanup.

    // Close the standard streams.
    // Decouples daemon from the terminal that started it.
    close(0);
    close(1);
    close(2);
}
