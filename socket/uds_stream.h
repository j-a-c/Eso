#ifndef ESO_SOCKET_UDS_STREAM
#define ESO_SOCKET_UDS_STREAM

#include <string>
#include <sys/socket.h>
#include <sys/types.h> 
#include <sys/un.h>

#include "../global_config/message_config.h"
#include "../logger/logger.h"

/*
 * Wrapper for a Unix Domain socket stream.
 */
class UDS_Stream 
{
public:
    UDS_Stream(int con_fd, sockaddr_un remote, int remote_len);
    UDS_Stream(int con_fd, sockaddr_un remote, int remote_len,
            std::string user);
    ~UDS_Stream();
    // Send data.
    void send(std::string msg) const;
    // Receive data.
    std::string recv();
    // Set the user we are currenting corresponding with.
    std::string get_user() const;
private:
    int _con_fd;
    struct sockaddr_un _remote;
    int _remote_len;
    // Max length of data we will read in at a time.
    int MAX_LENGTH = 1024;
    // Buffer holding partially constructed messages.
    std::string msg_buffer{};
    // The user we are corresponding with.
    std::string _user;
};

UDS_Stream::UDS_Stream(int con_fd, sockaddr_un remote, int remote_len)
    : _con_fd{con_fd}, _remote_len{remote_len}
{
    _remote = remote;
}

UDS_Stream::UDS_Stream(int con_fd, sockaddr_un remote, int remote_len, 
        std::string user)
    : _con_fd{con_fd}, _remote_len{remote_len}
{
    _remote = remote;
    _user = user;
}

UDS_Stream::~UDS_Stream()
{
    close(_con_fd);
}

/**
 * Send data. Includes MSG_END to allow the receiver to distinguish between 
 * messages.
 */
void UDS_Stream::send(std::string msg) const
{
    // Manually append MSG_END here to allow receiver to distinguish between
    // messages.
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
        std::string error_msg{"Something went wrong in UDS_Stream::send() "};
        error_msg += std::to_string(len);
        Logger::log(error_msg, LogLevel::Error);
    }
}

/** 
 * Returns a completed message, not including the MSG_END character.
 */
std::string UDS_Stream::recv()
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
            std::string error_msg{"Something went wrong in UDS_Stream::recv() "};
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

/**
 * Returns the user that initially requested access to this stream.
 */
std::string UDS_Stream::get_user() const
{
    return _user;
}


#endif
