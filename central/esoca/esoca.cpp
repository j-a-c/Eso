#include "ca_daemon.h"

/**
 * Starts the daemon that will run on the end hosts.
 */
int main()
{
    CADaemon daemon;
    return daemon.start();
}
