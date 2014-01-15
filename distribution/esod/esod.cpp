#include "distro_daemon.h"

/**
 * Starts the daemon that will run on the end hosts.
 */
int main()
{
    DistroDaemon daemon;
    return daemon.start();
}
