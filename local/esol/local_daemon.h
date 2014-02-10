#ifndef ESO_LOCAL_ESOL_LOCAL_DAEMON
#define ESO_LOCAL_ESOL_LOCAL_DAEMON

#include <string>
#include <thread>
#include <unistd.h>

#include "../config/esol_config.h"
#include "../config/mysql_config.h"
#include "../../crypto/aes.h"
#include "../../crypto/base64.h"
#include "../../crypto/hmac.h"
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
        Credential get_credential(const Credential) const;
        // Retrieves the requested permission.
        Permission get_permission(const Permission) const;
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

            Permission perm = Permission{received_string};

            // Update distribution server database.
            MySQL_Conn conn;
            conn.insert_permission(perm);

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
Credential LocalDaemon::get_credential(const Credential in_cred) const
{
    MySQL_Conn conn;

    Credential cred = conn.get_credential(in_cred);
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
            Credential req_cred;
            req_cred.set_name = in_cred.set_name;
            req_cred.version = in_cred.version;

            std::string log_msg{"esol to esod: "};
            log_msg += req_cred.serialize();
            Logger::log(log_msg, LogLevel::Debug);

            tcp_stream.send(GET_CRED);
            tcp_stream.send(req_cred.serialize());

            std::string tcp_received = tcp_stream.recv();
            // Our request was successful. Update our database.
            if (tcp_received != INVALID_REQUEST)
            {
                cred = Credential{tcp_received};
                conn.create_credential(cred);
                break;
            }
        }
        // TODO If not valid, throw exception.
    }
    return cred;
}

/**
 * Returns the Permission with the given set_name, entity, and loc.
 * First checks the local database, and then queries the distribution servers
 * if the Permission was not found locally.
 */
Permission LocalDaemon::get_permission(const Permission in_perm) const
{
    MySQL_Conn conn;

    Permission perm = conn.get_permission(in_perm);
    // If permission is empty, we will request it, update our database,
    // and then proceed.
    if (perm.set_name.empty())
    {
        Logger::log("esol: Permission not found, contacting esod.");

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
            Permission req_perm;
            req_perm.set_name = in_perm.set_name;
            req_perm.entity = in_perm.entity;
            req_perm.loc = in_perm.loc;

            std::string log_msg{"esol to esod: "};
            log_msg += req_perm.serialize();
            Logger::log(log_msg, LogLevel::Debug);

            tcp_stream.send(GET_PERM);
            tcp_stream.send(req_perm.serialize());

            std::string tcp_received = tcp_stream.recv();
            // Our request was successful. Update our database.
            if (tcp_received != INVALID_REQUEST)
            {
                perm = Permission{tcp_received};
                conn.create_permission(perm);
                break;
            }
        }
        // TODO If not valid, throw exception.
    }
    return perm;
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

        // Get the username of the user we are connected to.
        std::string curr_user = uds_stream.get_user();

        Logger::log("esol accepted new UDS connection.", LogLevel::Debug);

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
            Credential cred;
            cred.set_name = values[0];
            cred.version = std::stol(values[1]);
            std::string data_to_encrypt = values[2];

            log_msg = std::string{"esol: Data to encrypt: "};
            log_msg += data_to_encrypt;
            Logger::log(log_msg, LogLevel::Debug);

            // TODO
            // Check permissions to see if encrypt is allowed. (check
            // permission function?)

            // TODO get_credential should throw an exception if the request was
            // not valid.
            cred = get_credential(cred); 
            
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

            std::string log_msg{"esol: Decrypt msg: "};
            log_msg += received_string;
            Logger::log(log_msg, LogLevel::Debug);

            // According to message_config.h, we should receive:
            // set_name;version;data_to_encrypt.
            auto values = split_string(received_string, MSG_DELIMITER);
            Credential cred;
            cred.set_name = values[0];
            cred.version = std::stol(values[1]);
            std::string data_to_decrypt = values[2];

            // TODO
            // Check permissions to see if decrypt is allowed. (check
            // permission function?)

            // TODO get_credential should throw an exception if the request was
            // not valid.
            cred = get_credential(cred); 
            
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
        else if (received_string == REQUEST_HMAC)
        {
            received_string = uds_stream.recv();

            std::string log_msg{"esol: HMAC msg: "};
            log_msg += received_string;
            Logger::log(log_msg, LogLevel::Debug);

            // According to message_config.h, we should receive:
            // set_name;version;data;hash
            auto values = split_string(received_string, MSG_DELIMITER);
            Credential cred;
            cred.set_name = values[0];
            cred.version = std::stol(values[1]);
            std::string data = values[2];
            int hash = std::stol(values[3]);

            // TODO
            // Check permissions to see if HMAC is allowed. (check
            // permission function?)

            // TODO get_credential should throw an exception if the request was
            // not valid.
            cred = get_credential(cred); 

            if (cred.type == USERPASS)
            {
                // TODO throw exception
            }
            else if (cred.type == SYMMETRIC)
            {
                std::string hmac_data = hmac(cred.symKey, data, hash);
                uds_stream.send(hmac_data);
            }
            else if (cred.type == ASYMMETRIC)
            {
                // TODO throw exception
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
