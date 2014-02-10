#ifndef ESO_SOCKET_UDS_SOCKET
#define ESO_SOCKET_UDS_SOCKET

#include <errno.h>
#include <pwd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "exception.h"
#include "uds_stream.h"
#include "../logger/logger.h"

/*
 * Abstraction over a Unix Domain Socket.
 */
class UDS_Socket
{
public:
    UDS_Socket(std::string location);
    // Must be called before accept().
    int listen();
    // Accept an incoming connection.
    UDS_Stream accept();
    // Connect to the location this socket was created with.
    UDS_Stream connect();
private:
    int sock_len; // sock_info 
    int socket_fd;
    struct sockaddr_un sock_info;
    const int MAX_QUEUE_SIZE = 5;
    std::string recvCredentials(int sfd);
};

/*
 * The location specified will be the location to bind to if listen() is
 * called, or the location to connect to if connect() is called.
 */
UDS_Socket::UDS_Socket(std::string location)
{
    // Stream-oriented, local socket.
    if ((socket_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) 
    {
        // Error creating socket.
        Logger::log("Error creating socket in UDS_Socket().",
                LogLevel::Error);
        exit(1);
    }

    // Clear address structure
    memset(&sock_info, 0, sizeof(struct sockaddr_un));

    // Set the address parameters
    sock_info.sun_family = AF_UNIX;
    strcpy(sock_info.sun_path, location.c_str());
    sock_len = strlen(sock_info.sun_path) + sizeof(sock_info.sun_family);
}

/*
 * Listen to the specified connection.
 * Returns 0 if everything went ok.
 */
int UDS_Socket::listen()
{
    unlink(sock_info.sun_path);

    // Bind the address to the address in the Unix domain. 
    if (bind(socket_fd, (struct sockaddr *) &sock_info, sock_len) != 0)
    {
        Logger::log("Bind() failed in UDS_Socket::listen().", LogLevel::Error);
        close(socket_fd);
        return 1;
    }

    // Listen for incoming connections from client programs.
    if (::listen(socket_fd, MAX_QUEUE_SIZE) != 0)
    {
        Logger::log("Listen() failed in UDS_Socket::listen().", 
                LogLevel::Error);
        close(socket_fd);
        return 1;
    }

    return 0;
}

/**
 * Attempts to receive credentials from the specified socket file descriptor.
 */
std::string UDS_Socket::recvCredentials(int sfd)
{
    // We must set the SO_PASSCRED socket option in order to receive 
    // credentials.
    int optval = 1;
    if (setsockopt(sfd, SOL_SOCKET, SO_PASSCRED, &optval, 
                sizeof(optval)) == -1)
    {
        Logger::log("Error in setsockopt() in UDS_Socket::recvCredentials()", 
                LogLevel::Error);
    }

    // Return status and
    // Junk data we will receive.
    int nr, data;

    struct msghdr msgh;
    struct iovec iov;
    // Will hold the credential data.
    struct ucred *ucredp, ucred;

    union 
    {
        struct cmsghdr cmh;
        char   control[CMSG_SPACE(sizeof(struct ucred))];
    } control_un;

    struct cmsghdr *cmhp;

    // Set 'control_un' to describe ancillary data that we want to receive.
    control_un.cmh.cmsg_len = CMSG_LEN(sizeof(struct ucred));
    control_un.cmh.cmsg_level = SOL_SOCKET;
    control_un.cmh.cmsg_type = SCM_CREDENTIALS;

    // Set 'msgh' fields to describe 'control_un'.
    msgh.msg_control = control_un.control;
    msgh.msg_controllen = sizeof(control_un.control);

    // Set fields of 'msgh' to point to buffer used to receive (real)
    // data read by recvmsg()
    msgh.msg_iov = &iov;
    msgh.msg_iovlen = 1;
    iov.iov_base = &data;
    iov.iov_len = sizeof(int);

    // Some more fields...
    msgh.msg_name = nullptr; 
    msgh.msg_namelen = 0;

    // Receive real plus ancillary data.
    // The real data is just junk that we send in order to receive the
    // ancillary data.
    nr = recvmsg(sfd, &msgh, 0);
    if (nr == -1)
        Logger::log("recvmsg", LogLevel::Error);


    // Extract credentials information from received ancillary data.
    cmhp = CMSG_FIRSTHDR(&msgh);
    if (!cmhp || cmhp->cmsg_len != CMSG_LEN(sizeof(struct ucred)))
        Logger::log("bad cmsg header / message length in UDS_Socket::recvCredentials()",
                LogLevel::Error);
    if (cmhp->cmsg_level != SOL_SOCKET)
        Logger::log("cmsg_level != SOL_SOCKET in UDS_Socket::recvCredentials()",
                LogLevel::Error);
    if (cmhp->cmsg_type != SCM_CREDENTIALS)
        Logger::log("cmsg_type != SCM_CREDENTIALS in UDS_Socket::recvCredentials()",
                LogLevel::Error);

    ucredp = (struct ucred *) CMSG_DATA(cmhp);

    // Store the received credentials.
    pid_t pid = ucredp->pid;
    uid_t uid = ucredp->uid;
    gid_t gid = ucredp->gid;

    std::string log_msg{"Receive credentials (pug): "};
    log_msg += std::to_string(pid);
    log_msg.append(" ");
    log_msg += std::to_string(uid);
    log_msg.append(" ");
    log_msg += std::to_string(gid);
    log_msg.append(" : ");

    // Get the username of the connected user.
    struct passwd *pws;
    pws = getpwuid(uid);

    // The username of the user we are currently connected to.
    std::string user;

    if (pws)
    {
        user = std::string{pws->pw_name};
        log_msg += user;
    }
    else
    {
        // TODO Throw some sort of exception or close exception.
    }

    Logger::log(log_msg);

    return user;
}

/*
 * Accept an incoming connection.
 */
UDS_Stream UDS_Socket::accept()
{
    struct sockaddr_un client;
    int client_len;

    int connection_fd = ::accept(socket_fd, (struct sockaddr *) &client,
                (socklen_t *) &client_len);
    if (connection_fd == -1) 
    {
        int err = errno;
        Logger::log("Error accepting connection.", LogLevel::Error);
        
        // For error-reporting purposes.
        switch(err)
        {
            case EBADF:
                Logger::log("Invalid descriptor.");
                break;
            case ECONNABORTED:
                Logger::log("Connection aborted.");
                break;
            case ENOTSOCK:
                Logger::log("Descriptor references a file.");
                break;
            case EMFILE:
                Logger::log("Process file descriptor limit reached.");
                break;
            default:
                Logger::log("Something else went wrong.");
        }

        close(socket_fd);
        exit(1);
    }

    std::string user = recvCredentials(connection_fd);

    // Initialize a new stream and set the corresponding user.
    return UDS_Stream{connection_fd, client, client_len};
}

/*
 * Connect to the location specified when the UDS_Socket was created.
 *
 * @throws connect_exception
 */
UDS_Stream UDS_Socket::connect()
{
    struct msghdr msgh;
    struct iovec iov;

    // Junk data to send along with credentials.
    int data = 12345;

    msgh.msg_iov = &iov;
    msgh.msg_iovlen = 1;
    iov.iov_base = &data;
    iov.iov_len = sizeof(int);

    msgh.msg_name = nullptr;
    msgh.msg_namelen = 0;

    // We are sending our real credentials, so we don't specify an explicit
    // credential structure.
    msgh.msg_control = nullptr;
    msgh.msg_controllen = 0;

    // Attempt to connect to the socket.
    if (::connect(socket_fd, (struct sockaddr *)&sock_info, sock_len) == -1)
    {
        // Error connecting to socket.
        Logger::log("Error connecting to host in UDS_Socket::connect().", 
                LogLevel::Fatal);
        throw connect_exception();
    }

    // Send our credentials.
    int ns = sendmsg(socket_fd, &msgh, 0);
    if (ns == -1)
        Logger::log("Error sending credentials in UDS_Socket::connect()",
                LogLevel::Fatal);

    return UDS_Stream{socket_fd, sock_info, sock_len};
}

#endif
