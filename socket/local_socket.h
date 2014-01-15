#ifndef ESO_SOCKET_LOCAL_SOCKET
#define ESO_SOCKET_LOCAL_SOCKET

#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "socket.h"
#include "socket_stream.h"
#include "../logger/logger.h"

/*
 * Abstraction over a Unix Domain Socket.
 */
class Local_Socket : public Socket
{
public:
    Local_Socket(std::string location);
    // Must be called before accept()
    int listen() const;
    Socket_Stream accept() const;
    Socket_Stream connect() const;
private:
    int sock_len; // sock_info 
    int socket_fd;
    struct sockaddr_un sock_info;
    const int MAX_QUEUE_SIZE = 5;
};

Local_Socket::Local_Socket(std::string location)
{
    // Stream-oriented, local socket.
    if ((socket_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) 
    {
        // Error creating socket.
        Logger::log("Error creating socket.\n");
        exit(1);
    }

    // Clear address structure
    memset(&sock_info, 0, sizeof(struct sockaddr_un));

    // Set the address parameters
    sock_info.sun_family = AF_UNIX;
    strcpy(sock_info.sun_path, location.c_str());
    sock_len = strlen(sock_info.sun_path) + sizeof(sock_info.sun_family);
}

int Local_Socket::listen() const
{
    unlink(sock_info.sun_path);

    // Bind the address to the address in the Unix domain. 
    if (bind(socket_fd, (struct sockaddr *) &sock_info, sock_len) != 0)
    {
        Logger::log("Bind() failed.", LogLevel::Error);
        return 1;
    }

    // Listen for incoming connections from client programs.
    if (::listen(socket_fd, MAX_QUEUE_SIZE) != 0)
    {
        Logger::log("Listen() failed.", LogLevel::Error);
        return 1;
    }

    Logger::log("Listening successfully.", LogLevel::Debug);
    return 0;
}

Socket_Stream Local_Socket::accept() const
{
    struct sockaddr_un client;
    int client_len;

    int connection_fd = ::accept(socket_fd, (struct sockaddr *) &client,
                (socklen_t *) &client_len);
    if (connection_fd == -1) 
    {
        Logger::log("Error accepting connection.", LogLevel::Debug);
        
        switch(errno)
        {
            case EBADF:
                Logger::log("Invalid descriptor.");
                break;
            case ECONNABORTED:
                Logger::log("Connection aborted.");
                break;
            case ENOTSOCK:
                Logger::log("Descriptor references a file.");
                break;
            case EMFILE:
                Logger::log("Process file descriptor limit reached.");
                break;
            default:
                Logger::log("Something else went wrong.");
        }

        exit(1);
    }

    return Socket_Stream{connection_fd, client, client_len};
}

Socket_Stream Local_Socket::connect() const
{
    if (::connect(socket_fd, (struct sockaddr *)&sock_info, sock_len) == -1)
    {
        // Error connecting to socket.
        Logger::log("Error connecting to host.");
        exit(1);
    }

    return Socket_Stream{socket_fd, sock_info, sock_len};
}

#endif
