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
     * TODO see the following comment.
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

    // TODO read config file and choose port based on location
    TCP_Socket tcp_in_socket;
    if(tcp_in_socket.listen(std::string{"4344"}) != 0)
    {
        Logger::log("Socket creation failed in DistroDaemon::work()");
        exit(1);
    }

    Logger::log("esod listening to TCP successfully.");

    // TODO multithread
    // ex: std::thread(handle(incoming_stream);
    while(true)
    {
        Logger::log("esod is waiting for new TCP connection.", LogLevel::Debug);
        TCP_Stream incoming_stream = tcp_in_socket.accept();
        Logger::log("esod accepted new TCP connection.", LogLevel::Debug);

        std::string received_string = incoming_stream.recv();
        Logger::log(std::string{"Requested from esod: "} + received_string);

        if (received_string == UPDATE_PERM)
        {
            received_string = incoming_stream.recv();
            Logger::log(std::string{"esod received: "} + received_string);

            // set, entity, entity_type, op, loc
            auto values = split_string(received_string, MSG_DELIMITER);

            // Update distribution server database.
            MySQL_Conn conn;
            conn.insert_permission(values[0].c_str(), values[1].c_str(), 
                    std::stol(values[2]), std::stol(values[3]), 
                    values[4].c_str());

            // Send update to the local daemon.
            // TODO This obvious assumes the local daemon is running...
            TCP_Socket tcp_out_socket;
            TCP_Stream local_stream = tcp_out_socket.connect(
                    values[4], std::to_string(ESOL_PORT));

            std::string log_msg{"esod to esol: "};
            log_msg += received_string;
            Logger::log(log_msg, LogLevel::Debug);

            local_stream.send(UPDATE_PERM);
            local_stream.send(received_string);

            Logger::log("esod is closing TCP connection.", LogLevel::Debug);
        }
        else if (received_string == GET_PERM)
        {
            // Receive primary key
            // Query our database
            // Package and send results
        
        }
        else if (received_string == GET_CRED)
        {
            Credential cred;
            
            // TODO Ensure that location is authorized to receive this cred.

            // Parse request parameters. See the parameter order in
            // message_config.h
            received_string = incoming_stream.recv();
            auto params = split_string(received_string, MSG_DELIMITER);

            std::string log_msg{"In esod, cred params: "};
            log_msg += received_string;
            Logger::log(log_msg);

            // Query our database.
            MySQL_Conn conn;
            cred = conn.get_credential(params[0].c_str(), stoi(params[1]));
            // If the result is valid, serialize and send it.
            if (!cred.set_name.empty())
            {
                log_msg = std::string{"esod to esol: cred serialized: "};
                log_msg += cred.serialize();
                Logger::log(log_msg, LogLevel::Debug);
                incoming_stream.send(cred.serialize());
            }
            else
            {
                Logger::log("Invalid cred requested from esod.");
                incoming_stream.send(INVALID_REQUEST);
            }

        }
        else if (received_string == NEW_CRED)
        {
            // Receive serialized Credential.
            received_string = incoming_stream.recv();

            std::string log_msg{"In esod, new cred: "};
            log_msg += received_string;
            Logger::log(log_msg, LogLevel::Debug);

            // Insert Credential into database.
            Credential cred = Credential{received_string};
            MySQL_Conn conn;
            conn.create_credential(cred);
        }
        else if (received_string == PING)
        {
            incoming_stream.send(PING);
        }
        else
        {
            // TODO
            // Invalid request.
            std::string log_msg{"esod invalid request: "};
            log_msg += received_string;
            Logger::log(log_msg);

        }
    }

}

#endif
