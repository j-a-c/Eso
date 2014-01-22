#ifndef ESO_UTIL_NETWORK
#define ESO_UTIL_NETWORK

#include <arpa/inet.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <vector>

/*
 * Returns IP's associated with this machine.
 */
std::vector<std::string> get_network_interfaces()
{
    std::vector<std::string> results;

    struct ifaddrs *addrs;
    struct ifaddrs *tmp;

    getifaddrs(&addrs);
    tmp = addrs;

    while (tmp) 
    {
        if (tmp->ifa_addr && tmp->ifa_addr->sa_family == AF_INET)
        {
            struct sockaddr_in *pAddr = (struct sockaddr_in *)tmp->ifa_addr;
            results.push_back(std::string{inet_ntoa(pAddr->sin_addr)});
        }

        tmp = tmp->ifa_next;
    }

    freeifaddrs(addrs);  

    return results;
}

/*
 * Returns IP addresses
std::vector<std::string> get_ip()
{
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

}
*/

#endif
