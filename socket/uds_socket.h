#ifndef ESO_SOCKET_UDS_SOCKET
#define ESO_SOCKET_UDS_SOCKET

#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "exception.h"
#include "uds_stream.h"
#include "../logger/logger.h"

/*
 * Abstraction over a Unix Domain Socket.
 */
class UDS_Socket
{
public:
    UDS_Socket(std::string location);
    // Must be called before accept().
    int listen();
    // Accept an incoming connection.
    UDS_Stream accept();
    // Connect to the location this socket was created with.
    UDS_Stream connect();
private:
    int sock_len; // sock_info 
    int socket_fd;
    struct sockaddr_un sock_info;
    const int MAX_QUEUE_SIZE = 5;
};

/*
 * The location specified will be the location to bind to if listen() is
 * called, or the location to connect to if connect() is called.
 */
UDS_Socket::UDS_Socket(std::string location)
{
    // Stream-oriented, local socket.
    if ((socket_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) 
    {
        // Error creating socket.
        Logger::log("Error creating socket in UDS_Socket().",
                LogLevel::Error);
        exit(1);
    }

    // Clear address structure
    memset(&sock_info, 0, sizeof(struct sockaddr_un));

    // Set the address parameters
    sock_info.sun_family = AF_UNIX;
    strcpy(sock_info.sun_path, location.c_str());
    sock_len = strlen(sock_info.sun_path) + sizeof(sock_info.sun_family);
}

/*
 * Listen to the specified connection.
 * Returns 0 if everything went ok.
 */
int UDS_Socket::listen()
{
    unlink(sock_info.sun_path);

    // Bind the address to the address in the Unix domain. 
    if (bind(socket_fd, (struct sockaddr *) &sock_info, sock_len) != 0)
    {
        Logger::log("Bind() failed in UDS_Socket::listen().", LogLevel::Error);
        close(socket_fd);
        return 1;
    }

    // Listen for incoming connections from client programs.
    if (::listen(socket_fd, MAX_QUEUE_SIZE) != 0)
    {
        Logger::log("Listen() failed in UDS_Socket::listen().", 
                LogLevel::Error);
        close(socket_fd);
        return 1;
    }

    return 0;
}

/*
 * Accept an incoming connection.
 */
UDS_Stream UDS_Socket::accept()
{
    struct sockaddr_un client;
    int client_len;

    int connection_fd = ::accept(socket_fd, (struct sockaddr *) &client,
                (socklen_t *) &client_len);
    if (connection_fd == -1) 
    {
        int err = errno;
        Logger::log("Error accepting connection.", LogLevel::Error);
        
        // For error-reporting purposes.
        switch(err)
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

        close(socket_fd);
        exit(1);
    }

    return UDS_Stream{connection_fd, client, client_len};
}

/*
 * Connect to the location specified when the UDS_Socket was created.
 */
UDS_Stream UDS_Socket::connect()
{
    if (::connect(socket_fd, (struct sockaddr *)&sock_info, sock_len) == -1)
    {
        // Error connecting to socket.
        Logger::log("Error connecting to host in UDS_Socket::connect().", 
                LogLevel::Error);
        throw connect_exception();
    }

    return UDS_Stream{socket_fd, sock_info, sock_len};
}

#endif
