#ifndef ESO_SOCKET_TCP_SOCKET
#define ESO_SOCKET_TCP_SOCKET

#include <arpa/inet.h>
#include <cstring>
#include <errno.h>
#include <netdb.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>

#include "tcp_stream.h"
#include "../logger/logger.h"

/*
 * Abstraction over a TCP socket.
 */
class TCP_Socket
{
public:
    // Must be called before accept().
    int listen(std::string port);
    // Accept an incoming connection
    TCP_Stream accept();
    // Connect to somewhere.
    TCP_Stream connect(std::string host, std::string port);
private:
    int socket_fd;
    struct sockaddr_in servaddr;  //  Socket address structure.
    const int MAX_QUEUE_SIZE = 5;
};

/*
 * Listens to the designated port.
 *
 * Returns 0 if successful, nonzero otherwise.
 */
int TCP_Socket::listen(std::string port)
{
    // TODO config port numbers.

    struct sockaddr_in serv_addr; 

    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == -1)
    {
        Logger::log("Error creating socket in TCP_Socket::listen().",
                LogLevel::Error);
        return 1;
    }

    // Zero memory and fill data structure.
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(5100); 

    if (bind(socket_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
    {
        Logger::log("Error binding socket in TCP_Socket::listen().",
                LogLevel::Error);
        close(socket_fd);
        return 1;
    }

    if (::listen(socket_fd, MAX_QUEUE_SIZE) == -1 )
    {
        Logger::log("Error listening in TCP_Socket::listen()",
                LogLevel::Error);
        close(socket_fd);
        return 1;
    }
 
    return 0;
}

/*
 * Accept an incoming connection.
 */
TCP_Stream TCP_Socket::accept()
{
    // TODO error checking

    int conn_fd = ::accept(socket_fd, nullptr, nullptr);
   
    return TCP_Stream{conn_fd};
}

TCP_Stream TCP_Socket::connect(std::string host, std::string port)
{
    // TODO hostname to IP
    // TODO config hostname + port

    struct sockaddr_in serv_addr; 

    if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        Logger::log("Could not create socket in TCP_Socket::connect().",
                LogLevel::Error);

        exit(1);
    } 

    memset(&serv_addr, 0, sizeof(serv_addr)); 

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(5100); 

    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0)
    {
        Logger::log("inet_pton error occured in TCP_Socket::connect().",
                LogLevel::Error);
        exit(1);
    } 

    if (::connect(socket_fd, (struct sockaddr *)&serv_addr,
                sizeof(serv_addr)) < 0)
    {
        Logger::log("Connect failed in TCP_Socket::connect().",
                LogLevel::Error);
        exit(1);
    } 

    return TCP_Stream{socket_fd}; 
}

#endif
