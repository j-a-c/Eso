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
    // Send data.
    void send(char_vec msg) const;
    void send(std::string msg) const;
    char_vec recv();
private:
    int _con_fd;
    // Max length of data we will read in at a time.
    const int MAX_LENGTH = 1024;
    // Size of the message header. Contains the size of the following message.
    int MSG_HEADER_SIZE = 2;
    // Buffer holding partially constructed messages.
    char_vec msg_buffer{};
};

TCP_Stream::TCP_Stream(int con_fd) : _con_fd{con_fd}
{

}

TCP_Stream::~TCP_Stream()
{
    close(_con_fd);
}

/**
 * Send data. The first two chars are the message size.
 */
void TCP_Stream::send(char_vec msg) const
{
    // Position in buffer.
    char *pos = &msg[0];
    // Length of data to send
    size_t len = msg.size();
    // Number of characters sent.
    ssize_t n;

    char msg_header[MSG_HEADER_SIZE];
    msg_header[0] = len >> 8;   // Upper 8 bits.
    msg_header[1] = len & 0xFF; // Lower 8 bits.
    ::send(_con_fd, msg_header, MSG_HEADER_SIZE, 0);

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

/**
 * Included for backwards compatibility. 
 * Delegates to send(char_vec).
 */
void TCP_Stream::send(std::string msg) const
{
    TCP_Stream::send(char_vec{msg.begin(), msg.end()});
}

/**
 * Returns a completed message. The first two chars should be the message size.
 */
char_vec TCP_Stream::recv()
{
    char recv_msg[MAX_LENGTH];
    // The total message size and the amount we have left to read.
    int total, remaining;

    // We need to recv() until we have MSG_HEADER_SIZE bytes.
    while (msg_buffer.size() < MSG_HEADER_SIZE)
    {
        // Read the size of the message.
        if(int len = ::recv(_con_fd, recv_msg, MAX_LENGTH, 0))
        {
            msg_buffer.insert(msg_buffer.end(), &recv_msg[0], &recv_msg[len]);
       }
        else
        {
            std::string error_msg{"Something went wrong in TCP_Stream::recv() "};
            error_msg += std::to_string(len);
            Logger::log(error_msg, LogLevel::Error);
        }
    }

    // Compute the message size.
    total = remaining = (msg_buffer[0] << 8) + msg_buffer[1];
    msg_buffer = char_vec{msg_buffer.begin()+2, msg_buffer.end()};
    remaining -= msg_buffer.size();

    // Read until we find a complete message.
    while (remaining > 0)
    {
        if(int len = ::recv(_con_fd, recv_msg, MAX_LENGTH, 0))
        {
            msg_buffer.insert(msg_buffer.end(), &recv_msg[0], &recv_msg[len]);
            remaining -= len;
        }
        else
        {
            std::string error_msg{"Something went wrong in TCP_Stream::recv() "};
            error_msg += std::to_string(len);
            Logger::log(error_msg, LogLevel::Error);
        }
    }

    // Return message.
    char_vec ret_msg = char_vec{msg_buffer.begin(), msg_buffer.begin()+total};

    // Update message buffer to exclude return message.
    msg_buffer = char_vec{msg_buffer.begin()+total, msg_buffer.end()};

    return ret_msg;
}

#endif
