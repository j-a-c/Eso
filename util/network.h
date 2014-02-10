#ifndef ESO_UTIL_NETWORK
#define ESO_UTIL_NETWORK

#include <cstring>
#include <netdb.h>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

/**
 * Returns the fully qualified domain name associated with this machine.
 */
std::string get_fqdn()
{
    struct addrinfo hints, *info, *p;

    // SUSv2 guarantees that "Host names are limited to 255 bytes".
    char hostname[256];
    hostname[255] = '\0';
    // We will first attempt to get the hostname of this machine.
    gethostname(hostname, 255);

    memset(&hints, 0, sizeof hints);
    //either IPV4 or IPV6
    hints.ai_family = AF_UNSPEC; 
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_CANONNAME;

    // Now we attempt to get Internet addresses associated with the hostname of
    // this machine.
    int gai_result;
    if ((gai_result = getaddrinfo(hostname, "http", &hints, &info)) != 0) {
        // TODO Throw an exception.
        exit(1);
    }
    
    // The FQDN that we will return.
    std::string result;

    // Look for the canonical names returned.
    // (TODO) In my test cases, only one was returned, but this might pose a problem
    // in enterprise situations.
    for(p = info; p; p = p->ai_next) {
        result = std::string{p->ai_canonname};
    }

    // Free the address structure.
    freeaddrinfo(info);

    return result;
}

#endif
