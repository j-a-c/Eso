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

#include "esoca_config.h"
#include "../../daemon/Daemon.h"
#include "../../logger/logger.h"

/* 
 * Local daemon implementation
 */
class CADaemon : public Daemon
{
    public:
        int start() const;
    private:
        int work() const;
        const char * lock_path() const;
};

int CADaemon::start() const
{
    return Daemon::start();
}

const char * CADaemon::lock_path() const
{
    // TODO create config file?
    std::string path = "/home/bose/Desktop/eso/central/esoca//esoca_lock";
    return path.c_str();
}

int CADaemon::work() const
{
    // TODO save pid

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
    strcpy(server.sun_path, ESOCA_SOCKET_PATH);
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
    if (listen(socket_fd, ESOCA_QUEUE_SIZE) != 0)
    {
        Logger::log("listen() failed", LogLevel::Error);
        return 1;
    }

    Logger::log("esoca is listening successfully.", LogLevel::Debug);

    // TODO multithread
    // Accept client connections.
    while (int connection_fd = accept(socket_fd, (struct sockaddr *) &client, 
                (socklen_t *) &len) > -1)
    {
        Logger::log("esoca accepted new connection.", LogLevel::Debug);

        // TODO authenticate to make sure it is our web requesting access

        // Protocol
        Logger::log("Waiting for protocol type.");

        // Holds our messages from the web app.
        char recv_msg[1000]; // TODO config file for size?

        // Receive request type.
        if(int len = recv(connection_fd, recv_msg, 1000, 0))
        {
            recv_msg[len] = '\0';
            Logger::log(recv_msg);
        }

        // Check for valid request.
        if (strcmp(recv_msg, "permission") == 0)
        {
            std::string msg = "ok";
            send(connection_fd, msg.c_str(), msg.length()+1, 0);

            if(int len = recv(connection_fd, recv_msg, 1000, 0))
            {
                recv_msg[len] = '\0';
                Logger::log(recv_msg);

                char *pch = strtok(recv_msg,";");
                char *set_name = nullptr;
                char *entity = nullptr;

                // TODO Error checking?
                set_name = pch;
                pch = strtok(nullptr, ";");
                entity = pch;
                Logger::log(set_name);
                Logger::log(entity);

                // TODO propagate permissions.
            }

            Logger::log("Sending bye.");
            msg = "bye";
            send(connection_fd, msg.c_str(), msg.length()+1, 0);

            Logger::log("Daemon is closing connection.");
            close(connection_fd);
        }
        else
        {
            std::string msg = "ok";
            send(connection_fd, msg.c_str(), msg.length()+1, 0);
            close(connection_fd);
        } 

        // TODO delete after testing to keep the infinite loop
        return 0;
    }

    Logger::log("accept() error", LogLevel::Error);
    close(socket_fd);
    unlink(server.sun_path);
    return 1;

}

#endif
