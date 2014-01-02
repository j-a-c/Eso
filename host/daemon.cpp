// TODO delete after testing
#include <string>

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

#include "esod_config.h"
#include "database/db_conn.h"
#include "database/mysql_conn.h"
#include "../logger/logger.h"


/*
 * Returns true if the process acquires the lock file.
 * Must be called to ensure that only one esod daemon is running at a time.
 * Returns true if this process has been given the lock.
 *
 * Reads/writes should be atomic, else helper functions may be needed in case
 * the calls are interrupted by a signal.
 */
bool has_lock(const char* path)
{
    int fd;
    bool result;
    pid_t pid;

    // Try three times before giving up.
    for (int attempt = 0; attempt < 3; ++attempt)
    {
        /* 
         * Ensure this call creates the file. 
         * If path name exists, call won't fail, meaning this process created
         * the file and we can write our pid to it.
         * Else, we will check to make sure the file has our pid.
         */
        if ((fd = open(path, O_RDWR | O_CREAT | O_EXCL, S_IRWXU)) == -1)
        {
            // The error should be that the file exists.
            if (errno != EEXIST) 
                return false;

            // Re-open file and read the pid written.
            if ((fd = open(path, O_RDONLY)) == -1)
                return false;
            result = read(fd, &pid, sizeof(pid));
            close(fd);

            if (result)
            {
                // This process has the lock!
                if (pid == getpid())
                    return true;

                // Check to see if the pid that has the lock exists.
                // The lock file might be stale and need to be deleted.
                if (kill(pid, 0) == -1)
                {
                    // ESRCH == No such process.
                    if (errno != ESRCH) 
                        return false;

                    // Lock file is stale, so delete it and attempt to acquire.
                    attempt--;
                    unlink(path);
                    continue;
                }
            }
            sleep(1);
            continue;
        }
        
        /*
         * We created the lock, so we will write our pid and check it again
         * next iteration.
         * Between the time that kill() returns failure with an ESRCH error 
         * code and the time that unlink() is called to remove the lock file, 
         * another process could successfully delete the lock file and begin 
         * creating a new one.
         */
        pid = getpid();
        if (!write(fd, &pid, sizeof(pid)))
        {
            close(fd);
            return false;
        }
        close(fd);
        attempt--;
    }

    // If we made it here, we have not acquired the lock.
    return false;
}


/*
 * Prevents the process from writing any memory dump to disk.
 */
void limit_core(void)
{
    struct rlimit rlim;

    rlim.rlim_cur = rlim.rlim_max = 0;
    setrlimit(RLIMIT_CORE, &rlim);
}


/*
 * Starts the server run by the daemonized grandchild.
 */
int start_daemon(void)
{
    // Check if daemon is already running.
    if (!has_lock(ESOD_LOCK_PATH))
    {
        // TODO log?
        return 1;
    }

    // Prevent core dumps.
    limit_core();

    // TODO save pid

    struct sockaddr_un server, client;

    // Stream-oriented, local socket.
    int socket_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (socket_fd < 0)
    {
        Logger::log("socket() failed", LogLevel::Error);
        return 1;
    }

    // Clear the address structure
    memset(&server, 0, sizeof(struct sockaddr_un));

    // Set the address parameters.
    server.sun_family = AF_UNIX;
    strcpy(server.sun_path, ESOD_SOCKET_PATH);
    // The address should not exist, but unlink() just in case.
    unlink(server.sun_path);

    // Bind the address to the address in the Unix domain.
    int len = strlen(server.sun_path) + sizeof(server.sun_family);
    if (bind(socket_fd, (struct sockaddr *) &server, len) != 0)
    {
        Logger::log("bind() failed", LogLevel::Error);
        return 1;
    }

    // Listen for incoming connections from client programs.
    if (listen(socket_fd, ESOD_QUEUE_SIZE) != 0)
    {
        Logger::log("listen() failed", LogLevel::Error);
        return 1;
    }

    Logger::log("esod is listening successfully.", LogLevel::Debug);
    // Accept client connections.
    while (int connection_fd = accept(socket_fd, (struct sockaddr *) &client, 
                (socklen_t *) &len) > -1)
    {
        Logger::log("esod accepted new connection.", LogLevel::Debug);

        // TODO authenticate

        // TODO Implement protocol
        DB_Conn* db_conn = new MySQL_Conn;
        db_conn->create_permission();

        // TODO delete this test
        std::string msg = "connecteddddd!";
        send(connection_fd, msg.c_str(), msg.length()+1, 0);

        close(connection_fd);

        // TODO delete after testing
        return 0;
    }

    Logger::log("accept() error", LogLevel::Error);
    close(socket_fd);
    unlink(server.sun_path);
    return 1;
}


/**
 * Starts the daemon that will run on the end hosts.
 */
int main(void)
{
    // Fork the parent process and have the parent exit.
    if (pid_t pid = fork())
    {
        // Did fork() fail?
        if(pid < 0)
        {
            Logger::log("First daemon fork failed.", LogLevel::Error);
            return 1;
        }
        else // Fork was successful.
        {
            return 0;
        }
    }

    // We are now in the child process.

    // Detach process from its controlling terminal and make it a 
    // process group leader.
    setsid();

    // A process inherits working directory from its parent.
    // We need to changes the root directory to avoid some problems.
    // chroot("/new/root/directory"); // requires superuser privileges
    chdir("/");

    // Clear the file mode creation mask.
    umask(0);
    
    // Fork a second time to ensure process cannot acquire a controlling
    // terminal.
    if (pid_t pid = fork())
    {
        if (pid < 0)
        {
            Logger::log("Second daemon fork failed.");
            return 1;
        }
        else
            return 0;
    }

    // We are now in the grandchild process, which is truly daemonized.
    // init is in charge of process cleanup.

    // Close the standard streams.
    // Decouples daemon from the terminal that started it.
    close(0);
    close(1);
    close(2);

    int error = start_daemon();
    return error;
}
