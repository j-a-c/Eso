// TODO delete after testing
#include <string>

#include <sys/resource.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <unistd.h>
#include "Config.h"


/*
 * Prevents the process from writing any memory dump to disk.
 */
void limit_core(void)
{
    struct rlimit rlim;

    rlim.rlim_cur = rlim.rlim_max = 0;
    setrlimit(RLIMIT_CORE, &rlim);
}

/*
 * Starts the server run by the daemonized grandchild.
 */
int start_daemon(void)
{
    // Prevent core dumps.
    limit_core();

    // TODO save pid

    // TODO authenticate

    struct sockaddr_un server, client;
    int socket_fd, connection_fd;

    // Stream-oriented, local socket.
    socket_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (socket_fd < 0)
    {
        // TODO log
        // socket() failed.
        return 1;
    }

    // Clear the address structure
    memset(&server, 0, sizeof(struct sockaddr_un));

    // Set the address parameters.
    server.sun_family = AF_UNIX;
    strcpy(server.sun_path, ESOD_PATH);
    // The address should not exist, but unlink() just in case.
    unlink(server.sun_path);

    // Bind the address to the address in the Unix domain.
    int len = strlen(server.sun_path) + sizeof(server.sun_family);
    if (bind(socket_fd, (struct sockaddr *) &server, len) != 0)
    {
        // TODO log
        // bind() failed
        return 1;
    }

    // Listen for incoming connections from client programs.
    if (listen(socket_fd, ESOD_QUEUE_SIZE) != 0)
    {
        // TODO log
        // listen() failed
        return 1;
    }

    // Accept client connections.
    while (connection_fd = accept(socket_fd, (struct sockaddr *) &client, 
                (socklen_t *) &len) > -1)
    {
        // TODO Implement protocol

        // TODO delete this test
        std::string msg = "connecteddddd!";
        send(connection_fd, msg.c_str(), msg.length()+1, 0);

        close(connection_fd);

        // TODO delete after testing
        return 0;
    }

    // TODO log
    // accept() error
    close(socket_fd);
    unlink(server.sun_path);
    return 1;
}

/**
 * Starts the daemon that will run on the end hosts.
 */
int main(void)
{
    // TODO check if server is already running

    // Fork the parent process and have the parent exit.
    if (pid_t pid = fork())
    {
        // Did fork() fail?
        if(pid < 0)
        {
            // TODO log
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
            // TODO log
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

    int error = start_daemon();
    return error;
}
