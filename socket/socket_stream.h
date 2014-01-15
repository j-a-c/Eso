#ifndef ESO_SOCKET_SOCKET_STREAM
#define ESO_SOCKET_SOCKET_STREAM

#include <string>
#include <sys/socket.h>
#include <sys/types.h> 
#include <sys/un.h>

/*
 * Wrapper for a socket stream.
 */
class Socket_Stream
{
public:
    Socket_Stream(int con_fd, sockaddr_un remote, int remote_len);
    ~Socket_Stream();
    void send(std::string msg);
    std::string recv();
private:
    int _con_fd;
    struct sockaddr_un _remote;
    int _remote_len;
    const int MAX_LENGTH = 8449; // 8192+256+1
};

Socket_Stream::Socket_Stream(int con_fd, sockaddr_un remote, int remote_len)
    : _con_fd{con_fd}, _remote_len{remote_len}
{
    _remote = remote;
}

Socket_Stream::~Socket_Stream()
{
    close(_con_fd);
}

void Socket_Stream::send(std::string msg)
{
    ::send(_con_fd, msg.c_str(), msg.length()+1, 0);
}

std::string Socket_Stream::recv()
{
    char recv_msg[MAX_LENGTH];

    // Receive request type.
    if(int len = ::recv(_con_fd, recv_msg, MAX_LENGTH, 0))
    {
        recv_msg[len] = '\0';
        return std::string{recv_msg};
    }

    return std::string{};
}

#endif
