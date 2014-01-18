#ifndef ESO_DISTRIBUTION_ESOD_DISTRO_DAEMON
#define ESO_DISTRIBUTION_ESOD_DISTRO_DAEMON

#include <string>
#include <unistd.h>
#include <vector>

#include "../config/esod_config.h"
#include "../config/mysql_config.h"
#include "../../daemon/daemon.h"
#include "../../global_config/global_config.h"
#include "../../logger/logger.h"
#include "../../socket/tcp_socket.h"
#include "../../socket/tcp_stream.h"
#include "../../util/parser.h"

#include "../../database/mysql_conn.h"


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

    // TODO multithread
    // ex: std::thread(handle(tcp_stream);
    while(true)
    {
        Logger::log("esod is waiting for a new connection.", LogLevel::Debug);
        TCP_Stream tcp_stream = tcp_socket.accept();
        Logger::log("esod accepted new connection.", LogLevel::Debug);

        std::string received_string = tcp_stream.recv();
        Logger::log(std::string{"received: "} + received_string);

        auto values = split_string(received_string, DELIMITER);
        MySQL_Conn conn;
        conn.insert_permission(values[0].c_str(), values[1].c_str(),
                std::stol(values[2]), std::stol(values[3]));

        Logger::log("esod is closing connection.", LogLevel::Debug);
    }

}

#endif
