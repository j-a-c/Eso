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
#include "../../global_config/types.h"
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
    std::string my_hostname = get_fqdn();

    // The socket that will accept incoming connections.
    TCP_Socket tcp_in_socket;

    // Will be set to true once we have successfully set up our socket.
    bool connnected = false;

    // Read conifg file for distribution locations.
    // TODO config this location somewhere
    std::ifstream input( "/home/bose/Desktop/eso/global_config/locations_config" );
    for (std::string line; getline(input, line); )
    {
        auto values = split_string(line, LOC_DELIMITER);
        
        // If we find our FQDN in the config file, we will listen on the port
        // specified.
        if (values[0] == my_hostname)
        {
            if(tcp_in_socket.listen(values[1]) != 0)
            {
                Logger::log("Socket creation failed in DistroDaemon::work()", 
                        LogLevel::Fatal);
                exit(1);
            }
            connnected = true;
            break;
        }

    }

    // If we did not find our name in the location config file, exit and log message.
    if (!connnected)
    {
        Logger::log("Config info not found in location config file.", 
                LogLevel::Fatal);
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

        uchar_vec recv_msg = incoming_stream.recv();
        Logger::log(std::string{"Requested from esod: "} + to_string(recv_msg));

        /*
         * Occurs when the CA sends an updated Permission to this distribution
         * daemon.
         */
        if (recv_msg == UPDATE_PERM)
        {
            recv_msg = incoming_stream.recv();
            Logger::log(std::string{"esod received: "} + to_string(recv_msg));

            Permission perm = Permission{recv_msg};

            // Update distribution server database.
            MySQL_Conn conn;
            conn.insert_permission(perm); 

            // Send update to the local daemon.
            // TODO This obvious assumes the local daemon is running...
            TCP_Socket tcp_out_socket;
            TCP_Stream local_stream = 
                tcp_out_socket.connect(perm.loc, std::to_string(ESOL_PORT));

            std::string log_msg{"esod to esol: "};
            log_msg += perm.serialize();
            Logger::log(log_msg, LogLevel::Debug);

            local_stream.send(UPDATE_PERM);
            local_stream.send(perm.serialize());

            Logger::log("esod is closing TCP connection.", LogLevel::Debug);
        }
        /*
         * Occurs when the CA requests that a Permission be deleted due to a
         * deletion on the admin interface.
         */
        else if (recv_msg == DELETE_PERM)
        {
            // Create Permission object.
            Permission perm = Permission{incoming_stream.recv()};

            // Update our database
            MySQL_Conn conn;
            conn.delete_permission(perm);
            
            // Send DELETE_PERM to the local daemon.
            // TODO This obvious assumes the local daemon is running...
            TCP_Socket tcp_out_socket;
            TCP_Stream local_stream = 
                tcp_out_socket.connect(perm.loc, std::to_string(ESOL_PORT));

            std::string log_msg{"esod to esol: "};
            log_msg += perm.serialize();
            Logger::log(log_msg, LogLevel::Debug);

            local_stream.send(DELETE_PERM);
            local_stream.send(perm.serialize());

        }
        /*
         * Occurs when a local daemon queries this distribution daemon for a
         * Permission.
         */
        else if (recv_msg == GET_PERM)
        {
            recv_msg = incoming_stream.recv();
            Logger::log(std::string{"esod received: "} + 
                    std::string{recv_msg.begin(), recv_msg.end()});

            Permission perm = Permission{recv_msg};

            // TODO ensure the local daemon is the designated location for this
            // credential? esol only uses Permissions that are marked with its
            // FQDN so this may not matter.

            // Update distribution server database.
            MySQL_Conn conn;
            conn.get_permission(perm); 

            std::string log_msg{"esod to esol: "};
            log_msg += perm.serialize();
            Logger::log(log_msg, LogLevel::Debug);

            incoming_stream.send(perm.serialize());
        }
        /*
         * Occurs when a local daemon queries this distribution daemon for a
         * Credential.
         */
        else if (recv_msg == GET_CRED)
        {
            // TODO Ensure that location is authorized to receive this cred.

            // Parse request parameters. See the parameter order in
            // message_config.h
            recv_msg = incoming_stream.recv();

            std::string log_msg{"esod: GET_CRED received: "};
            log_msg += std::string{recv_msg.begin(), recv_msg.end()};
            Logger::log(log_msg, LogLevel::Debug);

            Credential cred = Credential{recv_msg};

            log_msg = std::string{"In esod, cred params: "};
            log_msg += cred.serialize();
            Logger::log(log_msg);

            // Query our database.
            MySQL_Conn conn;
            cred = conn.get_credential(cred);
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
        /*
         * Occurs when a new Credential has been created by the CA due to some
         * interaction with the admin interface.
         */
        else if (recv_msg == NEW_CRED)
        {
            // Receive serialized Credential.
            recv_msg = incoming_stream.recv();

            std::string log_msg{"In esod, new cred: "};
            log_msg += std::string{recv_msg.begin(), recv_msg.end()};
            Logger::log(log_msg, LogLevel::Debug);

            // Insert Credential into database.
            Credential cred = Credential{recv_msg};
            MySQL_Conn conn;
            conn.create_credential(cred);
        }
        else if (recv_msg == PING)
        {
            incoming_stream.send(PING);
        }
        else
        {
            // TODO
            // Invalid request.
            std::string log_msg{"esod invalid request: "};
            log_msg += std::string{recv_msg.begin(), recv_msg.end()};
            Logger::log(log_msg);

        }
    }

}

#endif
