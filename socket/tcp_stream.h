#ifndef ESO_SOCKET_TCP_STREAM
#define ESO_SOCKET_TCP_STREAM

#include <cstring>
#include <string>

#include "../global_config/message_config.h"

/*
 * Wrapper for a TCP stream.
 */
class TCP_Stream
{
public:
    TCP_Stream(int con_fd); 
    ~TCP_Stream();
    void send(std::string msg) const;
    std::string recv();
private:
    int _con_fd;
    // Max length of data we will read in at a time.
    const int MAX_LENGTH = 1024;
    // Buffer holding partially constructed messages.
    std::string msg_buffer{};
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
    // We will manually append MSG_END here to allow the receiver to
    // distinguish between messages.
    msg.append(MSG_END);

    // Buffer to send.
    const char *buff = msg.c_str();
    // Position in buffer.
    char *pos = (char *) buff;
    // Length of data to send
    size_t len = strlen(buff);
    // Number of characters sent.
    ssize_t n;

    // Ensure that all data is sent.
    while (len > 0 && (n = ::send(_con_fd, pos, len, 0)) > 0)
    {
        pos += n;
        len -= (size_t) n;
    }
    if (len > 0 || n < 0)
    {
        std::string error_msg{"Something went wrong in TCP_Stream::send() "};
        error_msg += std::to_string(len);
        Logger::log(error_msg, LogLevel::Error);
    }
}

std::string TCP_Stream::recv()
{
    char recv_msg[MAX_LENGTH];
    std::size_t end_pos;
    // Read until we find a complete message.
    while ((end_pos = msg_buffer.find_first_of(MSG_END)) == std::string::npos)
    {
        if(int len = ::recv(_con_fd, recv_msg, MAX_LENGTH, 0))
        {
            char temp_msg[len];
            strncpy(temp_msg, recv_msg, len);
            msg_buffer.append(temp_msg, len);
        }
        else
        {
            std::string error_msg{"Something went wrong in TCP_Stream::recv() "};
            error_msg += std::to_string(len);
            Logger::log(error_msg, LogLevel::Error);
        }
    }

    // Return message does not include MSG_END
    std::string ret_msg = msg_buffer.substr(0, end_pos);
    // Update message buffer to exclude MSG_END
    msg_buffer = msg_buffer.substr(end_pos+1);

    return ret_msg;
}

#endif
