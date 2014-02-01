#ifndef ESO_CENTRAL_ESOCA_CA_DAEMON
#define ESO_CENTRAL_ESOCA_CA_DAEMON

#include <signal.h>
#include <sstream>
#include <string>
#include <vector>

#include "../config/esoca_config.h"
#include "../config/mysql_config.h"
#include "../../crypto/aes.h"
#include "../../crypto/base64.h"
#include "../../crypto/memory.h"
#include "../../crypto/password.h"
#include "../../crypto/rsa.h"
#include "../../daemon/daemon.h"
#include "../../database/mysql_conn.h"
#include "../../logger/logger.h"
#include "../../global_config/global_config.h"
#include "../../global_config/message_config.h"
#include "../../socket/tcp_socket.h"
#include "../../socket/tcp_stream.h"
#include "../../socket/uds_socket.h"
#include "../../socket/uds_stream.h"
#include "../../util/parser.h"

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
        // Propagates a message to the distribution servers.
        void propagate(const std::string msg_type, const std::string msg) const;
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

/**
 * Propagates a message to the distribution servers.
 *
 * @param msg_type The type of the message (ex: UPDATE_PERM).
 * @param msg The message to send
 */
void CADaemon::propagate(const std::string msg_type, 
        const std::string msg) const
{
    // Read conifg file for distribution locations.
    // Send distribution_msg to all distribution servers.
    // TODO config this location somewhere
    std::ifstream input( "/home/bose/Desktop/eso/global_config/locations_config" );
    for (std::string line; getline(input, line); )
    {
        auto values = split_string(line, LOC_DELIMITER);

        TCP_Socket tcp_socket;
        TCP_Stream tcp_stream = tcp_socket.connect(
                values[0], values[1]);

        std::string log_msg{"esoca to esod: "};
        log_msg += msg;
        Logger::log(log_msg, LogLevel::Debug);

        tcp_stream.send(msg_type);
        tcp_stream.send(msg);
    }
}

int CADaemon::work() const
{
    // TODO save pid

    UDS_Socket uds_socket{std::string{ESOCA_SOCKET_PATH}};
    if(uds_socket.listen())
    {
        Logger::log("Error in esoca attempting to listen to UDS.");
        exit(1);
    }
    
    Logger::log("esoca is listening to UDS successfully.", LogLevel::Debug);

    // TODO multithread
    // Accept client connections.
    while (true)
    {
        Logger::log("esoca is waiting for a UDS connection.", LogLevel::Debug);

        UDS_Stream uds_stream = uds_socket.accept();

        Logger::log("esoca accepted new UDS connection.", LogLevel::Debug);

        // TODO authenticate to make sure it is our web requesting access


        /*
         * Protocol starts here.
         */

        // Holds the message we receive.
        std::string recv_msg;
        // Holds the message we send.
        std::string msg;

        recv_msg = uds_stream.recv();
        Logger::log(std::string{"Requested from esoca: "} + recv_msg);

        // Check for valid request.
        if (recv_msg == NEW_PERM)
        {
            recv_msg = uds_stream.recv();
            Logger::log(recv_msg);

            Permission perm = Permission{recv_msg};

            // Attempt to update database.
            MySQL_Conn conn;
            int status = conn.create_permission(perm);

            // Propagate to distribution servers.
            propagate(UPDATE_PERM, perm.serialize());

        }
        else if (recv_msg == UPDATE_PERM)
        {
            recv_msg = uds_stream.recv();
            Logger::log(recv_msg);

            Permission perm = Permission{recv_msg};

            // Attempt to update database.
            MySQL_Conn conn;
            int status = conn.update_permission(perm);

            /*
             * Propagate permissions.
             */

            // Get the most recent result from the database.
            // This is needed because the distribution server will call
            // instert_permission because it may have been offline before.
            perm = conn.get_permission(perm);

            // Propagate to distribution servers.
            propagate(UPDATE_PERM, perm.serialize());
        }
        else if (recv_msg == DELETE_PERM)
        {
            Permission perm = Permission{uds_stream.recv()};

             // Update esoca's database.
            MySQL_Conn conn;
            conn.delete_permission(perm) ;

            // Propagate to distribution servers.
            propagate(DELETE_PERM, perm.serialize());
        }
        else if (recv_msg == NEW_CRED)
        {
            // Receive the serialized credential.
            recv_msg = uds_stream.recv();
            
            std::string log_msg{"esoca: Serialized credential: "};
            log_msg += recv_msg;
            Logger::log(log_msg, LogLevel::Debug);

            Credential cred = Credential(recv_msg);

            // Generate keys for this credential.
            if (cred.type == USERPASS)
            {
                // TODO implement
                // TODO encrypt + mac
            }
            else if (cred.type == ASYMMETRIC)
            {
                int size = cred.size;

                // Get keys
                auto key_store = get_new_RSA_pair(size);
                unsigned char *pubKey = std::get<0>(key_store);
                int pubLen = std::get<1>(key_store);
                unsigned char *priKey = std::get<2>(key_store);
                int priLen = std::get<3>(key_store);

                // Encode keys
                unsigned char *pubKey_enc = base64_encode(pubKey, pubLen);
                unsigned char *priKey_enc = base64_encode(priKey, priLen);

                cred.pubKey = std::string{(char*) pubKey_enc};
                cred.priKey = std::string{(char*) priKey_enc};

                // Add to query
                // TODO encrypt + mac
                // Free keys
                free((void*)secure_memset(pubKey, 0, pubLen));
                free((void*)secure_memset(pubKey_enc, 0, 
                            strlen(reinterpret_cast<const char *>(pubKey_enc))));
                free((void*)secure_memset(priKey, 0, priLen));
                free((void*)secure_memset(priKey_enc, 0, 
                            strlen(reinterpret_cast<const char *>(priKey_enc))));
            }
            else if (cred.type == SYMMETRIC)
            {
                int size = cred.size;

                // Get key and encode.
                unsigned char * key = get_new_AES_key(size);
                unsigned char * enc = base64_encode(key, size/8);

                // TODO encrypt + mac
                cred.symKey = std::string{(char*) enc};

                // Securely erase key and free
                free((void*)secure_memset(key, 0, size/8)); // size is in bits
                free((void*)secure_memset(enc, 0, 
                            strlen(reinterpret_cast<const char *>(enc))));
            }

            // Update esoca's database.
            MySQL_Conn conn;
            conn.create_credential(cred) ;

            // Propagate to distribution servers.
            propagate(NEW_CRED, cred.serialize());
        }
        else if (recv_msg == PING)
        {
            uds_stream.send(PING);
        }
        else
        {
            std::string log_msg{"esoca invalid request: "};
            log_msg += recv_msg;
            Logger::log(log_msg);
        }

        Logger::log("esoca is closing UDS connection.", LogLevel::Debug);
    }

    Logger::log("accept() error", LogLevel::Error);
    return 1;
}

#endif
