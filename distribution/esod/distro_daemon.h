#ifndef ESO_DISTRIBUTION_ESOD_DISTRO_DAEMON
#define ESO_DISTRIBUTION_ESOD_DISTRO_DAEMON

#include <string>
#include <unistd.h>

#include "esod_config.h"
#include "../../daemon/daemon.h"
#include "../../logger/logger.h"
#include "../../socket/tcp_socket.h"
#include "../../socket/tcp_stream.h"


/* 
 * Local daemon implementation
 */
class DistroDaemon : public Daemon
{
    public:
        int start() const;
    private:
        int work() const;
        const char * lock_path() const;
};

int DistroDaemon::start() const
{
    return Daemon::start();
}

const char * DistroDaemon::lock_path() const
{
    // TODO create config file?
    std::string path = "/home/bose/Desktop/eso/distribution/esod/esod_lock";
    return path.c_str();
}

int DistroDaemon::work() const
{
    // TODO
    TCP_Socket tcp_socket;
    if(tcp_socket.listen(std::string{"4344"}) != 0)
    {
        Logger::log("Socket creation failed in DistroDaemon::work()");
        exit(1);
    }

    Logger::log("esod listening successfully.");

    while(true)
    {
        TCP_Stream tcp_stream = tcp_socket.accept();
        Logger::log("esod accepted new connection.");
        Logger::log(std::string{"received"} + tcp_stream.recv());
        Logger::log("esod is closing connection.");
    }

}

#endif
