#ifndef ESO_LOCAL_ESOL_LOCAL_DAEMON
#define ESO_LOCAL_ESOL_LOCAL_DAEMON

#include <string>
#include <thread>
#include <unistd.h>

#include "../config/esol_config.h"
#include "../config/mysql_config.h"
#include "../../crypto/aes.h"
#include "../../crypto/base64.h"
#include "../../crypto/rsa.h"
#include "../../daemon/daemon.h"
#include "../../database/db_types.h"
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
        // Retrieves the requested credential.
        Credential get_credential(const char *, const int) const;
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

/**
 * Returns the Credential with the given set_name and version.
 * First checks the local database, and then queries the distribution servers
 * if the Credential was not found locally.
 */
Credential LocalDaemon::get_credential(const char * set_name, 
        const int version) const
{
    MySQL_Conn conn;
    Credential cred = conn.get_credential(set_name, version);
    // If credential is empty, we will request it, update our database,
    // and then proceed.
    if (cred.set_name.empty())
    {
        Logger::log("esol: Credential not found, contacting esod.");

        // Try requesting all distribution locations.
        // Read config file for distribution locations.
        // TODO config this location somewhere
        std::ifstream input( "/home/bose/Desktop/eso/global_config/locations_config" );
        for (std::string line; getline(input, line); )
        {
            auto dist_info = split_string(line, LOC_DELIMITER);

            TCP_Socket tcp_socket;
            TCP_Stream tcp_stream = tcp_socket.connect(
                    dist_info[0], dist_info[1]);

            // Form message to send.
            // set_name;version
            std::string distribution_msg{set_name};
            distribution_msg += MSG_DELIMITER;
            distribution_msg += std::to_string(version);

            std::string log_msg{"esol to esod: "};
            log_msg += distribution_msg;
            Logger::log(log_msg, LogLevel::Debug);

            tcp_stream.send(GET_CRED);
            tcp_stream.send(distribution_msg);

            std::string tcp_received = tcp_stream.recv();
            // Our request was successful. Update our database.
            if (tcp_received != INVALID_REQUEST)
            {
                cred = Credential(tcp_received);
                conn.create_credential(cred);
                break;
            }
        }
        // TODO If not valid, throw exception.
    }
    return cred;
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
        else if (received_string == REQUEST_ENCRYPT)
        {
            received_string = uds_stream.recv();

            std::string log_msg{"esol: Encrypt params: "};
            log_msg += received_string;
            Logger::log(log_msg, LogLevel::Debug);

            // According to message_config.h, we should receive:
            // set_name;version;data_to_encrypt.
            auto values = split_string(received_string, MSG_DELIMITER);
            std::string data_to_encrypt = values[2];

            // Check permissions to see if encrypt is allowed. (check
            // permission function?)

            // TODO get_credential should throw an exception if the request was
            // not valid.
            Credential cred = get_credential(values[0].c_str(), 
                    stoi(values[1]));
            
            // The length of the data to be encrypted.
            int len;
            // Encrypt and return ciphertext.
            if (cred.type == USERPASS)
            {
                // TODO throw exception
            }
            else if (cred.type == SYMMETRIC)
            {
                unsigned char *encyption;
                len = cred.symKey.length();
                unsigned char *key = base64_decode((unsigned char *)cred.symKey.c_str(), (size_t *)&len);
                // We add 1 to data_to_encrypt.length() because we want to
                // preserve the null terminator.
                len = data_to_encrypt.length() + 1;
                encyption = 
                    aes_encrypt(key, 
                            (unsigned char *) data_to_encrypt.c_str(), 
                            &len, cred.size);
                uds_stream.send(std::string{(char*)encyption});
                free(encyption);
                free(key);
            }
            else if (cred.type == ASYMMETRIC)
            {
                // TODO
            }
        }
        else if (received_string == REQUEST_DECRYPT)
        {
            received_string = uds_stream.recv();

            std::string log_msg{"esol: Decrypt params: "};
            log_msg += received_string;
            Logger::log(log_msg, LogLevel::Debug);

            // According to message_config.h, we should receive:
            // set_name;version;data_to_encrypt.
            auto values = split_string(received_string, MSG_DELIMITER);
            std::string data_to_decrypt = values[2];

            // Check permissions to see if encrypt is allowed. (check
            // permission function?)

            // TODO get_credential should throw an exception if the request was
            // not valid.
            Credential cred = get_credential(values[0].c_str(), 
                    stoi(values[1]));
            
            // The length of the data to be encrypted.
            int len;
            // Encrypt and return ciphertext.
            if (cred.type == USERPASS)
            {
                // TODO throw exception
            }
            else if (cred.type == SYMMETRIC)
            {
                unsigned char *decyption;
                len = cred.symKey.length();
                unsigned char *key = base64_decode((unsigned char *)cred.symKey.c_str(), (size_t *)&len);
                // We add 1 to data_to_encrypt.length() because we want to
                // preserve the null terminator.
                len = data_to_decrypt.length() + 1;
                decyption = 
                    aes_decrypt(key, 
                            (unsigned char *) data_to_decrypt.c_str(), 
                            &len, cred.size);
                uds_stream.send(std::string{(char*)decyption});
                free(decyption);
                free(key);
            }
            else if (cred.type == ASYMMETRIC)
            {
                // TODO
            }
        }
        else
        {
            // TODO 
            // Invalid request.
            std::string log_msg{"esol invalid request: "};
            log_msg += received_string;
            Logger::log(log_msg);
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
