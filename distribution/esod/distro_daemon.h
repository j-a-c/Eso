#ifndef ESO_DISTRIBUTION_ESOD_DISTRO_DAEMON
#define ESO_DISTRIBUTION_ESOD_DISTRO_DAEMON

#include <string>
#include <unistd.h>

#include "esod_config.h"
#include "../../daemon/daemon.h"
#include "../../logger/logger.h"

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

}

#endif
