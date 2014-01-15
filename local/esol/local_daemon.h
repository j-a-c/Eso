#ifndef ESO_LOCAL_ESOL_LOCAL_DAEMON
#define ESO_LOCAL_ESOL_LOCAL_DAEMON

#include <string>
#include <unistd.h>

#include "esol_config.h"
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
};

int LocalDaemon::start() const
{
    return Daemon::start();
}

const char * LocalDaemon::lock_path() const
{
    // TODO create config file?
    std::string path = "/home/bose/Desktop/eso/local/esol/esol_lock";
    return path.c_str();
}

int LocalDaemon::work() const
{
    UDS_Socket uds_socket{std::string{ESOL_SOCKET_PATH}};
    if (uds_socket.listen())
    {
        Logger::log("Error in esol attempting to listen.");
        exit(1);
    }

    Logger::log("esol is listening successfully.", LogLevel::Debug);

    // TODO multithread
    // Accept client connections.
    while (true)
    {
        UDS_Stream uds_stream = uds_socket.accept();

        Logger::log("esol accepted new connection.", LogLevel::Debug);

        // TODO authenticate by checking pid

        // TODO Implement protocol


        Logger::log("Daemon is closing connection.");
    }

    Logger::log("accept() error", LogLevel::Error);
}

#endif
