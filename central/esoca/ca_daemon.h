#ifndef ESO_LOCAL_ESOL_LOCALDAEMON
#define ESO_LOCAL_ESOL_LOCALDAEMON

#include <iostream>
#include <signal.h>
#include <sstream>
#include <string>
#include <vector>

#include "esoca_config.h"
#include "../../daemon/daemon.h"
#include "../../logger/logger.h"
#include "../../socket/tcp_socket.h"
#include "../../socket/tcp_stream.h"
#include "../../socket/uds_socket.h"
#include "../../socket/uds_stream.h"

/* 
 * Local daemon implementation
 */
class CADaemon : public Daemon
{
    public:
        int start() const;
    private:
        int work() const;
        const char * lock_path() const;
};

int CADaemon::start() const
{
    return Daemon::start();
}

const char * CADaemon::lock_path() const
{
    // TODO create config file?
    std::string path = "/home/bose/Desktop/eso/central/esoca/esoca_lock";
    return path.c_str();
}

int CADaemon::work() const
{
    // TODO save pid

    UDS_Socket uds_socket{std::string{ESOCA_SOCKET_PATH}};
    if(uds_socket.listen())
    {
        Logger::log("Error in esoca attempting to listen.");
        exit(1);
    }
    
    Logger::log("esoca is listening successfully.", LogLevel::Debug);

    // TODO multithread
    // Accept client connections.
    while (true)
    {
        Logger::log("esoca is waiting for a new connection.", LogLevel::Debug);

        UDS_Stream uds_stream = uds_socket.accept();

        Logger::log("esoca accepted new connection.", LogLevel::Debug);

        // TODO authenticate to make sure it is our web requesting access


        /*
         * Protocol starts here.
         */

        // Holds the message we receive.
        std::string recv_msg;
        // Holds the message we send.
        std::string msg;

        recv_msg = uds_stream.recv();
        Logger::log(std::string{"Requested: "} + recv_msg);

        // Check for valid request.
        if (strcmp(recv_msg.c_str(), "permission") == 0)
        {
            msg = "ok";
            uds_stream.send(msg);

            recv_msg = uds_stream.recv();
            Logger::log(recv_msg);

            // Tokenize the message we received.
            std::vector<std::string> strings;
            std::istringstream f{recv_msg};
            std::string s;    
            while (std::getline(f, s, ';')) 
            {
                std::cout << s << std::endl;
                strings.push_back(s);
            }

            const char *set_name = strings[0].c_str();
            const char *entity = strings[1].c_str();

            // TODO propagate permissions.
            TCP_Socket tcp_socket;
            // TODO read conifg file
            TCP_Stream tcp_stream = tcp_socket.connect(
                    std::string{"localhost"}, std::string{"4344"});
            tcp_stream.send("hello distro");
            Logger::log("Message sent to distro.", LogLevel::Debug);
            

            Logger::log("Sending bye.", LogLevel::Debug);
            msg = "bye";
            uds_stream.send(msg);
        }
        else
        {
            Logger::log("Invalid request.", LogLevel::Error);
            msg = "invalid";
            uds_stream.send(msg);
        }

        Logger::log("esoca is closing connection.", LogLevel::Debug);
    }

    Logger::log("accept() error", LogLevel::Error);
    return 1;
}

#endif
