#ifndef ESO_LOCAL_ESOL_LOCAL_DAEMON
#define ESO_LOCAL_ESOL_LOCAL_DAEMON

#include <string>
#include <thread>
#include <unistd.h>

#include "../config/esol_config.h"
#include "../config/mysql_config.h"
#include "../../daemon/daemon.h"
#include "../../global_config/global_config.h"
#include "../../logger/logger.h"
#include "../../socket/tcp_socket.h"
#include "../../socket/tcp_stream.h"
#include "../../socket/uds_socket.h"
#include "../../socket/uds_stream.h"
#include "../../util/parser.h"

#include "../../database/mysql_conn.h"

/* 
 * Local daemon implementation
 */
class LocalDaemon : public Daemon
{
    public:
        int start() const;
    private:
        int work() const;
        const char * lock_path() const;
        void handleTCP() const;
        void handleUDS() const;
};

int LocalDaemon::start() const
{
    return Daemon::start();
}

/*
 * Return the path for the lock file.
 */
const char * LocalDaemon::lock_path() const
{
    // TODO create config file?
    std::string path = "/home/bose/Desktop/eso/local/esol/esol_lock";
    return path.c_str();
}

/*
 * Handle incoming TCP connections.
 */
void LocalDaemon::handleTCP() const
{
    TCP_Socket tcp_socket;
    if(tcp_socket.listen(std::to_string(ESOL_PORT)) != 0)
    {
        Logger::log("Socket creation failed in LocalDaemon::handleTCP()");
        exit(1);
    }

    Logger::log("esol is listening successfully for TCP.");

    // TODO multithread?
    while(true)
    {
        Logger::log("esol is waiting for a new TCP connection.", 
                LogLevel::Debug);
        TCP_Stream incoming_stream = tcp_socket.accept();
        Logger::log("esol accepted new TCP connection.", LogLevel::Debug);

        std::string received_string = incoming_stream.recv();
        Logger::log(std::string{"Requested from esol: "} + received_string);

        if (received_string == UPDATE_PERM)
        {
            received_string = incoming_stream.recv();
            Logger::log(std::string{"esol received: "} + received_string);

            auto values = split_string(received_string, MSG_DELIMITER);

            // Update distribution server database.
            MySQL_Conn conn;
            conn.insert_permission(values[0].c_str(), values[1].c_str(), 
                    std::stol(values[2]), std::stol(values[3]), 
                    values[4].c_str());

            Logger::log("esol is closing TCP connection.", LogLevel::Debug);
        }
        else
        {
            // TODO
            // Invalid request.
        }

    }

    Logger::log("TCP accept() error", LogLevel::Error);
}

/*
 * Handle incoming UDS connections.
 */
void LocalDaemon::handleUDS() const
{
    UDS_Socket uds_in_socket{std::string{ESOL_SOCKET_PATH}};
    if (uds_in_socket.listen())
    {
        Logger::log("Error in esol attempting to listen to UDS.");
        exit(1);
    }

    Logger::log("esol is listening successfully for UDS.", LogLevel::Debug);

    // TODO multithread?
    while (true)
    {
        Logger::log("esol is waiting for UDS connection.", LogLevel::Debug);

        UDS_Stream uds_stream = uds_in_socket.accept();

        Logger::log("esol accepted new UDS connection.", LogLevel::Debug);

        // TODO authenticate by checking pid

        // Implement protocol

        std::string received_string = uds_stream.recv();
        Logger::log(std::string{"Requested from esol: "} + received_string);

        if (received_string == PING)
        {
            uds_stream.send(PING);
        }
        else
        {
            // TODO 
            // Invalid request.
        }
        Logger::log("esol is closing UDS connection.");
    }
    Logger::log("UDS accept() error", LogLevel::Error);
}

int LocalDaemon::work() const
{
    std::thread udp_thread(&LocalDaemon::handleUDS, this);
    std::thread tcp_thread(&LocalDaemon::handleTCP, this);

    // The threads should loop infinitely, so we will keep the main thread
    // waiting.
    udp_thread.join();
    tcp_thread.join();
}

#endif
