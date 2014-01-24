#ifndef ESO_SOCKET_EXCEPTION
#define ESO_SOCKET_EXCEPTION

#include <exception>

/*
 * Includes the exceptions that may be thrown by sockets or streams.
 */

struct connect_exception : public std::exception
{
    const char * what() const throw()
    {
        return "Error connecting.";
    }
};

#endif
