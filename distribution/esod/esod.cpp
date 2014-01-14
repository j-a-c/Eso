// TODO delete after testing
#include <string>

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

#include "DistroDaemon.h"

/**
 * Starts the daemon that will run on the end hosts.
 */
int main()
{
    DistroDaemon daemon;
    return daemon.start();
}
