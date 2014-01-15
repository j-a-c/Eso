#ifndef ESO_SOCKET_UDS_STREAM
#define ESO_SOCKET_UDS_STREAM

#include <string>
#include <sys/socket.h>
#include <sys/types.h> 
#include <sys/un.h>

/*
 * Wrapper for a Unix Domain socket stream.
 */
class UDS_Stream 
{
public:
    UDS_Stream(int con_fd, sockaddr_un remote, int remote_len);
    ~UDS_Stream();
    // Send data.
    void send(const std::string msg) const;
    // Receive data.
    std::string recv() const;
private:
    int _con_fd;
    struct sockaddr_un _remote;
    int _remote_len;
    const int MAX_LENGTH = 8449; // 8192+256+1
};

UDS_Stream::UDS_Stream(int con_fd, sockaddr_un remote, int remote_len)
    : _con_fd{con_fd}, _remote_len{remote_len}
{
    _remote = remote;
}

UDS_Stream::~UDS_Stream()
{
    close(_con_fd);
}

/*
 * Send data.
 */
void UDS_Stream::send(const std::string msg) const
{
    ::send(_con_fd, msg.c_str(), msg.length()+1, 0);
}

/* 
 * Receive data.
 */
std::string UDS_Stream::recv() const
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
