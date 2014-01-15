#ifndef ESO_SOCKET_TCP_STREAM
#define ESO_SOCKET_TCP_STREAM

#include <errno.h>
#include <string>

/*
 * Wrapper for a TCP stream.
 */
class TCP_Stream
{
public:
    TCP_Stream(int con_fd); 
    ~TCP_Stream();
    void send(std::string msg) const;
    std::string recv() const;
private:
    int _con_fd;
    const int MAX_LENGTH = 8449; // 8192+256+1
};

TCP_Stream::TCP_Stream(int con_fd) : _con_fd{con_fd}
{

}

TCP_Stream::~TCP_Stream()
{
    close(_con_fd);
}

void TCP_Stream::send(std::string msg) const
{
    if (::send(_con_fd, msg.c_str(), msg.length()+1, 0) == -1)
    {
        Logger::log(std::string{"Error in TCP_Stream::send(): "} +
                std::to_string(errno), LogLevel::Error);
    }
}
std::string TCP_Stream::recv() const
{
    char recv_msg[MAX_LENGTH];

    // Receive request type.
    if(int len = ::recv(_con_fd, recv_msg, MAX_LENGTH, 0))
    {
        recv_msg[len] = '\0';
        return std::string{recv_msg};
    }
    else
    {
        Logger::log(std::string{"Error in TCP_Stream::recv(): "} + 
                std::to_string(errno), LogLevel::Error);

    }

    return std::string{};
}

#endif
