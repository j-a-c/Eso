#ifndef ESO_LOCAL_ESOL_LOCAL_DAEMON
#define ESO_LOCAL_ESOL_LOCAL_DAEMON

#include <string>
#include <thread>
#include <unistd.h>

#include "../config/esol_config.h"
#include "../../daemon/daemon.h"
#include "../../logger/logger.h"
#include "../../socket/uds_socket.h"
#include "../../socket/uds_stream.h"

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
        void handleUDP() const;
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

}

/*
 * Handle incoming UDP connections.
 */
void LocalDaemon::handleUDP() const
{
    UDS_Socket uds_socket{std::string{ESOL_SOCKET_PATH}};
    if (uds_socket.listen())
    {
        Logger::log("Error in esol attempting to listen.");
        exit(1);
    }

    Logger::log("esol is listening successfully.", LogLevel::Debug);

    // TODO multithread?
    while (true)
    {
        UDS_Stream uds_stream = uds_socket.accept();

        Logger::log("esol accepted new connection.", LogLevel::Debug);

        // TODO authenticate by checking pid

        // TODO Implement protocol


        Logger::log("esol is closing connection.");
    }

    Logger::log("accept() error", LogLevel::Error);

}

int LocalDaemon::work() const
{
    std::thread udp_thread(&LocalDaemon::handleUDP, this);
    std::thread tcp_thread(&LocalDaemon::handleTCP, this);

    // The threads should loop infinitely, so we will keep the main thread
    // waiting.
    udp_thread.join();
    tcp_thread.join();
}

#endif
