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
    TCP_Stream connect(std::string hostname, std::string port);
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
    serv_addr.sin_port = htons(std::stoi(port)); 

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

/*
 * Connects to the port at the given hostname.
 * 
 * TODO Current only handles IPv4. To use IPv6, update instances of AF_INET and 
 * update serv_addr.sin_family to the appropriate family when attempting 
 * to connect.
 */
TCP_Stream TCP_Socket::connect(std::string hostname, std::string port)
{
    // Where we want to connect to
    struct sockaddr_in serv_addr; 
    // Possible connections.
    struct addrinfo hints, *p; 
    struct addrinfo *servinfo; 

    // Create a socket.
    if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        Logger::log("Could not create socket in TCP_Socket::connect().",
                LogLevel::Error);

        exit(1);
    } 

    // Fill in server's data structure.
    memset(&serv_addr, 0, sizeof(serv_addr)); 
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(std::stoi(port)); 

    // Hints to resolve the hostname.
    memset(&hints, 0, sizeof hints); 
    hints.ai_family   = AF_INET;    
    hints.ai_socktype = SOCK_STREAM;  
    hints.ai_flags    = AI_PASSIVE; 

    // Resolve the hostname. 
    if ((getaddrinfo(hostname.c_str(), nullptr, &hints, &servinfo)) == -1)
    {
        Logger::log("getaddrinfo() error in TCP_Socket::connect()", 
                LogLevel::Error);
        exit(1);
    }       

    bool connected = false;
    // Attempt to connect
    for (p=servinfo; p; p=p->ai_next) 
    { 
        struct in_addr  *addr;  
        if (p->ai_family == AF_INET) 
        { 
            struct sockaddr_in *ipv = (struct sockaddr_in *)p->ai_addr; 
            addr = &(ipv->sin_addr);  
        } 
        // We will keep this just in case we change from AF_INET in the future.
        else 
        { 
            struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr; 
            addr = (struct in_addr *) &(ipv6->sin6_addr); 
        }

        // Set the network address structure.
        serv_addr.sin_addr = *addr;

        // Attempt to connect.
        if (::connect(socket_fd, (struct sockaddr *)&serv_addr, 
                    sizeof(serv_addr)) < 0)
        {
            Logger::log("Connect failed in TCP_Socket::connect().",
                    LogLevel::Error);
            continue;
        }
        else
        {
            connected = true;
            // We are connected!
            break;
        }
    } 

    freeaddrinfo(servinfo);     

    if (connected)
        return TCP_Stream{socket_fd}; 
    else
        exit(1);
}

#endif
