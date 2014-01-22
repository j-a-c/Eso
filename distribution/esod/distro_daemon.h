#ifndef ESO_DISTRIBUTION_ESOD_DISTRO_DAEMON
#define ESO_DISTRIBUTION_ESOD_DISTRO_DAEMON

#include <string>
#include <unistd.h>
#include <vector>

#include "../config/esod_config.h"
#include "../config/mysql_config.h"
#include "../../daemon/daemon.h"
#include "../../global_config/global_config.h"
#include "../../global_config/message_config.h"
#include "../../logger/logger.h"
#include "../../socket/tcp_socket.h"
#include "../../socket/tcp_stream.h"
#include "../../util/parser.h"
#include "../../util/network.h"

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
    /*
    std::vector<std::string> locations;
    std::vector<std::string> ports;
    // Read conifg file for distribution locations.
    // TODO config this location somewhere
    std::ifstream input( "/home/bose/Desktop/eso/global_config/locations_config" );
    for (std::string line; getline(input, line); )
    {
        auto values = split_string(line, LOC_DELIMITER);

        locations.push_back(values[0]);
        ports.push_back(values[1]);
    }
    */

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
        Logger::log(std::string{"Requested from esod: "} + received_string);

        if (received_string == UPDATE_PERM)
        {
            received_string = tcp_stream.recv();
            Logger::log(std::string{"esod received: "} + received_string);

            auto values = split_string(received_string, MSG_DELIMITER);
            MySQL_Conn conn;
            conn.insert_permission(values[0].c_str(), values[1].c_str(), 
                    std::stol(values[2]), std::stol(values[3]), 
                    values[4].c_str());

            Logger::log("esod is closing connection.", LogLevel::Debug);
        }
        else if (received_string == GET_PERM)
        {
            // Receive primary key
            // Query our database
            // Package and send results
        
        }
        else
        {
            // TODO
            // Invalid request.
        }
    }

}

#endif
