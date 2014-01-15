#ifndef ESO_LOCAL_ESOL_LOCALDAEMON
#define ESO_LOCAL_ESOL_LOCALDAEMON

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <string>
#include <sys/resource.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

#include "esol_config.h"
#include "../../daemon/Daemon.h"
#include "../../logger/logger.h"

/* 
 * Local daemon implementation
 */
class LocalDaemon : public Daemon
{
    public:
        int start() const;
    private:
        int work() const;
        const char * lock_path() const;
};

int LocalDaemon::start() const
{
    return Daemon::start();
}

const char * LocalDaemon::lock_path() const
{
    // TODO create config file?
    std::string path = "/home/bose/Desktop/eso/local/esol/esol_lock";
    return path.c_str();
}

int LocalDaemon::work() const
{
    struct sockaddr_un server, client;

    // Stream-oriented, local socket.
    int socket_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (socket_fd < 0)
    {
        Logger::log("socket() failed", LogLevel::Error);
        return 1;
    }

    // Clear the address structure
    memset(&server, 0, sizeof(struct sockaddr_un));

    // Set the address parameters.
    server.sun_family = AF_UNIX;
    strcpy(server.sun_path, ESOD_SOCKET_PATH);
    // The address should not exist, but unlink() just in case.
    unlink(server.sun_path);

    // Bind the address to the address in the Unix domain.
    int len = strlen(server.sun_path) + sizeof(server.sun_family);
    if (bind(socket_fd, (struct sockaddr *) &server, len) != 0)
    {
        Logger::log("bind() failed", LogLevel::Error);
        return 1;
    }

    // Listen for incoming connections from client programs.
    if (listen(socket_fd, ESOD_QUEUE_SIZE) != 0)
    {
        Logger::log("listen() failed", LogLevel::Error);
        return 1;
    }

    Logger::log("esod is listening successfully.", LogLevel::Debug);

    // TODO multithread
    // Accept client connections.
    while (int connection_fd = accept(socket_fd, (struct sockaddr *) &client, 
                (socklen_t *) &len) > -1)
    {
        Logger::log("esod accepted new connection.", LogLevel::Debug);

        // TODO authenticate by checking pid

        // TODO Implement protocol

        // TODO delete this test
        std::string msg = "connecteddddd!";
        send(connection_fd, msg.c_str(), msg.length()+1, 0);

        Logger::log("Daemon is closing connection.");
        close(connection_fd);

        // TODO delete after testing
        return 0;
    }

    Logger::log("accept() error", LogLevel::Error);
    close(socket_fd);
    unlink(server.sun_path);
    return 1;

}

#endif
