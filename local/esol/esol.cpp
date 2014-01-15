#include "local_daemon.h"

/**
 * Starts the daemon that will run on the end hosts.
 */
int main()
{
    LocalDaemon daemon;
    return daemon.start();
}
