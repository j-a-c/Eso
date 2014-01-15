#ifndef ESO_SOCKET_SOCKET
#define ESO_SOCKET_SOCKET

#include <string>
#include "socket_stream.h"

class Socket
{
    // Listen for incoming connections. Must be called before accept().
    virtual int listen() const = 0;
    // Accept an incoming connection.
    virtual Socket_Stream accept() const = 0;
    // Connection to somwhere.
    virtual Socket_Stream connect() const = 0;
};

#endif
