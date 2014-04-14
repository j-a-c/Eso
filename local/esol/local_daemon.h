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
#include "../../crypto/memory.h"
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
#include "../../util/network.h"

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
        Permission get_permission(Permission) const;
        // Checks whether the entity has permission to execute the given
        // operation. It is assumed that the entity is on this machine.
        bool has_permission_to(const std::string entity, 
                const std::string set_name, const int op) const;
        // Returns true if the Credential is expired.
        bool is_expired(const Credential cred) const;
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
    std::string path = "/home/jac/Desktop/eso/local/esol/esol_lock";
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

        uchar_vec recv_msg = incoming_stream.recv();
        Logger::log(std::string{"Requested from esol: "} + to_string(recv_msg));

        if (recv_msg == UPDATE_PERM)
        {
            recv_msg = incoming_stream.recv();
            Logger::log(std::string{"esol received: "} + to_string(recv_msg));

            Permission perm = Permission{recv_msg};

            // Update distribution server database.
            MySQL_Conn conn;
            conn.insert_permission(perm);
        }
        else if (recv_msg == DELETE_PERM)
        {
            recv_msg = incoming_stream.recv();
            Logger::log(std::string{"esol received: "} + to_string(recv_msg));

            Permission perm = Permission{recv_msg};

            // Update distribution server database.
            MySQL_Conn conn;
            conn.delete_permission(perm);
        }
        else
        {
            // TODO
            // Invalid request.
        }

        Logger::log("esol is closing TCP connection.", LogLevel::Debug);

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
        std::ifstream input( "/home/jac/Desktop/eso/global_config/locations_config" );
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

            uchar_vec tcp_received = tcp_stream.recv();
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
 * Returns the Permission with the given set_name, entity.
 * First checks the local database, and then queries the distribution servers
 * if the Permission was not found locally.
 *
 * This method will reset in_perm.loc to the machine's current FQDN in order to
 * prevent masquerading. Thus, it does not matter is loc is not set when the
 * Permission is originally given to this function.
 */
Permission LocalDaemon::get_permission(Permission in_perm) const
{
    // Set our current FQDN.
    in_perm.loc = get_fqdn();

    MySQL_Conn conn;

    // This is the permission that we will return.
    Permission perm = conn.get_permission(in_perm);
    // If permission is empty, we will request it, update our database,
    // and then proceed.
    if (perm.set_name.empty())
    {
        Logger::log("esol: Permission not found, contacting esod.");

        // Try requesting all distribution locations.
        // Read config file for distribution locations.
        // TODO config this location somewhere
        std::ifstream input( "/home/jac/Desktop/eso/global_config/locations_config" );
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

            uchar_vec tcp_received = tcp_stream.recv();
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

/**
 * Checks to see if the entity has permission to execute the specified
 * operation on the specified set. It is implicitly assumed that the entity is
 * executing the operation on the current machine.
 *
 * @param entity The name of the user requesting the operation.
 * @param set The set the enitity wishes to perform the operation on.
 * @param op The operation the entity wishes to perform.
 * 
 * @return true if the entity can execute the operation, false otherwise.
 */
bool LocalDaemon::has_permission_to(const std::string entity, const std::string set_name,
        const int op) const
{
    Permission perm;
    perm.entity = entity;
    perm.set_name = set_name;

    // Attempt to retrieve this permission.
    perm = get_permission(perm);

    // The entity has permission if the op bit is set in the returned 
    // permission.
    return (perm.op & op); 

}

/**
 * Checks to see if the specified credential has expired.
 *
 * @param cred The credential with the expiration date that will be checked.
 *
 * @return true if the Credential has not expired.
 */
bool LocalDaemon::is_expired(const Credential cred) const
{
    // Get the current time.
    std::time_t now = std::time(nullptr);

    // Clear the tm struct before we use it.
    struct tm texp;
    memset(&texp, 0, sizeof(struct tm));
    // The Credential expiration date is in ISO 8601 date format.
    if(!strptime(cred.expiration.c_str(), "%F", &texp))
    {
        // There was an error in strptime
        Logger::log("Error in strptime in LocalDaemon::is_expired", 
                LogLevel::Error);
        // We will return true by default.
        return true;
    }
    // Get the expiration data into time_t format.
    time_t exp = mktime(&texp);

    // Compare the current time and the expiration date.
    if (difftime(exp, now) <= 0.0)
    {
        // The Credential is expired.
        return true;
    }
    else
    {
        // The Credential is not expired.
        return false;
    }

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
        std::string curr_user = uds_in_socket._user;

        std::string log_msg("esol accepted new UDS connection with: ");
        log_msg += uds_stream.get_user();
        Logger::log(log_msg, LogLevel::Debug);

        // Implement protocol

        uchar_vec recv_msg = uds_stream.recv();
        Logger::log(std::string{"Requested from esol: "} + to_string(recv_msg));

        if (recv_msg == PING)
        {
            uds_stream.send(PING);
        }
        else if (recv_msg == REQUEST_ENCRYPT)
        {
            // Receive parameters.
            recv_msg = uds_stream.recv();
            std::string set_name = to_string(recv_msg);

            recv_msg = uds_stream.recv();
            int version = std::stol(to_string(recv_msg));

            uchar_vec data = uds_stream.recv();

            Credential cred;
            cred.set_name = set_name;
            cred.version = version;

            // Check permissions to see if encrypt is allowed.
            // If the entity does not have permission, we will send an empty
            // string and continue.
            if (!has_permission_to(curr_user, cred.set_name, ENCRYPT_OP))
            {
                uds_stream.send(std::string{});
                continue; 
            }

            // TODO get_credential should throw an exception if the request was
            // not valid.
            cred = get_credential(cred); 

            // Check if the credential has expired.
            if (is_expired(cred))
            {
                uds_stream.send(std::string{});
                continue; 
            }
            
            // Encrypt and return ciphertext.
            if (cred.type == USERPASS)
            {
                // TODO throw exception
            }
            else if (cred.type == SYMMETRIC)
            {
                int len = cred.symKey.length();
                // Base64 decode the returned symmetric key.
                // TODO Error check len because base64_decode might fail.
                unsigned char *key = base64_decode((unsigned char *)cred.symKey.c_str(), (size_t *)&len);
                
                // Encrypt data.
                uchar_vec encryption = 
                    aes_encrypt(key, data, cred.size);
                
                // Send data.
                uds_stream.send(encryption);

                Logger::log("esol: Clearing encryption data.", LogLevel::Debug);
                // Securely zero out memory.
                // TODO Debug zeroing the key.
                //Logger::log("esol: Zeroing key.", LogLevel::Debug);
                //secure_memset(key, 0, cred.symKey.length() - 1);
                Logger::log("esol: Zeroing msg data.", LogLevel::Debug);
                secure_memset(&encryption[0], 0, encryption.size());
                // Free memory.
                Logger::log("esol: Freeing msg data.", LogLevel::Debug);
                Logger::log("esol: Freeing key data.", LogLevel::Debug);
                free(key);
                Logger::log("esol: Done freeing encryption data.", LogLevel::Debug);

            // This ends the symmetric encryption case.
            }
            else if (cred.type == ASYMMETRIC)
            {
                int len;
                // Base64 decode the returned public key.
                // TODO Error check len because base64_decode might fail.
                unsigned char *public_store = base64_decode((unsigned char*) cred.pubKey.c_str(), (size_t *) &len);

                // DER decode the public key.
                RSA *public_key = DER_decode_RSA_public(public_store, len);

                uchar_vec encryption = rsa_encrypt(public_key, data);

                // Send the encrypted message.
                uds_stream.send(encryption);

                // Securely zero out memory.
                // TODO Debug secure_memset calls.
                // secure_memset(public_store, 0, len);
                // secure_memset(cipher, 0, encrypted_length);
                // secure_memset(bcipher, 0, strlen(bcipher));
                // Free memory.
                RSA_free(public_key);
                free(public_store);
            // This ends the asymmetric encrypt case.
            }
        // This ends the encrypt case.
        }
        else if (recv_msg == REQUEST_DECRYPT)
        {
            // Receive parameters.
            recv_msg = uds_stream.recv();
            std::string set_name = to_string(recv_msg);

            recv_msg = uds_stream.recv();
            int version = std::stol(to_string(recv_msg));

            uchar_vec data = uds_stream.recv();

            Credential cred;
            cred.set_name = set_name;
            cred.version = version;

            // Check permissions to see if decrypt is allowed.
            // If the entity does not have permission, we will send an empty
            // string and continue.
            if (!has_permission_to(curr_user, cred.set_name, DECRYPT_OP))
            {
                uds_stream.send(uchar_vec{});
                continue; 
            }


            // TODO get_credential should throw an exception if the request was
            // not valid.
            cred = get_credential(cred); 

            // We will not check if the credential is expired because data
            // should be able to be decrypted with an expired credential.

            // The length of the data to be encrypted.
            int len;
            // Encrypt and return ciphertext.
            if (cred.type == USERPASS)
            {
                // TODO throw exception
            }
            else if (cred.type == SYMMETRIC)
            {
                len = cred.symKey.length();
                // Base64 decode the returned symmetric key.
                // TODO Error check len because base64_decode might fail.
                unsigned char *key = base64_decode((unsigned char *)cred.symKey.c_str(), (size_t *)&len);

                // Decrypt the data.
                uchar_vec decryption = aes_decrypt(key, data, cred.size);
                // Send the decryption.
                uds_stream.send(decryption);

                Logger::log("esol: Clearing decryption data", LogLevel::Debug);
                // Securely zero out memory.
                // TODO Debug zeroing the key.
                //Logger::log("esol: Zeroing key.", LogLevel::Debug);
                //secure_memset(key, 0, cred.symKey.length());
                Logger::log("esol: Zeroing msg.", LogLevel::Debug);
                secure_memset(&decryption[0], 0, decryption.size());
                // Free memory.
                Logger::log("esol: Freeing key.", LogLevel::Debug);
                free(key);
                Logger::log("esol: Done clearing decryption data.", LogLevel::Debug);

            
            // This ends the symmetric decypt case.
            }
            else if (cred.type == ASYMMETRIC)
            {
                int len;
                // Base64 decode the returned private key.
                // TODO Error check len because base64_decode might fail.
                unsigned char *private_store = base64_decode((unsigned char*) cred.priKey.c_str(), (size_t *) &len);
                // DER decode the private key.
                RSA *private_key = DER_decode_RSA_private(private_store, len);

                uchar_vec decryption = rsa_decrypt(private_key, data);

                // Send the decrypted messge.
                uds_stream.send(decryption);

                // Securely zero out memory.
                // TODO Debug secure_memset calls.
                // secure_memset(private_store, 0, len);
                // secure_memset(orig, 0, orig_size);
                // secure_memset(msg, 0, mlen); 
                // Free memory.
                RSA_free(private_key);
                free(private_store);
            // This ends the asymmetric decrypt case.
            }

        // This ends the decrypt case.
        }
        else if (recv_msg == REQUEST_HMAC)
        {
            // Receive a parameters.
            recv_msg = uds_stream.recv();
            std::string set_name = to_string(recv_msg);

            recv_msg = uds_stream.recv();
            int version = std::stol(to_string(recv_msg));

            uchar_vec data = uds_stream.recv();

            recv_msg = uds_stream.recv();
            int hash = std::stol(to_string(recv_msg));

            Credential cred;
            cred.set_name = set_name;
            cred.version = version;

            // Check permissions to see if encrypt is allowed.
            // If the entity does not have permission, we will send an empty
            // string and continue.
            if (!has_permission_to(curr_user, cred.set_name, HMAC_OP))
            {
                uds_stream.send(uchar_vec{});
                continue; 
            }

            // TODO get_credential should throw an exception if the request was
            // not valid.
            cred = get_credential(cred); 

            // Check if the credential has expired.
            if (is_expired(cred))
            {
                uds_stream.send(uchar_vec{});
                continue; 
            }

            if (cred.type == USERPASS)
            {
                // TODO throw exception
            }
            else if (cred.type == SYMMETRIC)
            {
                uchar_vec hmac_data = hmac(cred.symKey, data, hash);
                uds_stream.send(hmac_data);
            }
            else if (cred.type == ASYMMETRIC)
            {
                // TODO throw exception
            }

        }
        else if (recv_msg == REQUEST_SIGN)
        {
            // Receive parameters.
            recv_msg = uds_stream.recv();
            std::string set_name = to_string(recv_msg);

            recv_msg= uds_stream.recv();
            int version = std::stol(to_string(recv_msg));

            uchar_vec data = uds_stream.recv();

            recv_msg= uds_stream.recv();
            int hash = std::stol(to_string(recv_msg));

            Credential cred;
            cred.set_name = set_name;
            cred.version = version;

            // Check permissions to see if sign is allowed.
            // If the entity does not have permission, we will send an empty
            // string and continue.
            if (!has_permission_to(curr_user, cred.set_name, SIGN_OP))
            {
                uds_stream.send(uchar_vec{});
                continue; 
            }

            // TODO get_credential should throw an exception if the request was
            // not valid.
            cred = get_credential(cred); 

            // Check if the credential has expired.
            if (is_expired(cred))
            {
                uds_stream.send(uchar_vec{});
                continue; 
            }

            if (cred.type == ASYMMETRIC)
            {
                int len;
                // Base64 decode the returned private key.
                // TODO Error check len because base64_decode might fail.
                unsigned char *private_store = base64_decode((unsigned char*) cred.priKey.c_str(), (size_t *) &len);
                // DER decode the private key.
                RSA *private_key = DER_decode_RSA_private(private_store, len);

                // Compute the signature.
                uchar_vec sig = rsa_sign(private_key, data, hash);

                // Free allocated material.
                RSA_free(private_key);
                free(private_store);

                uds_stream.send(sig);
            }
            else
            {
                // TODO throw exception
            }
        }
        else if (recv_msg == REQUEST_VERIFY)
        {
            recv_msg = uds_stream.recv();
            std::string set_name = to_string(recv_msg);

            recv_msg= uds_stream.recv();
            int version = std::stol(to_string(recv_msg));

            uchar_vec sig = uds_stream.recv();
            uchar_vec data = uds_stream.recv();

            recv_msg= uds_stream.recv();
            int hash = std::stol(to_string(recv_msg));

            Credential cred;
            cred.set_name = set_name;
            cred.version = version;

            // Check permissions to see if verify is allowed.
            // If the entity does not have permission, we will send an empty
            // string and continue.
            if (!has_permission_to(curr_user, cred.set_name, VERIFY_OP))
            {
                uds_stream.send(uchar_vec{});
                continue; 
            }

            // TODO get_credential should throw an exception if the request was
            // not valid.
            cred = get_credential(cred); 

            // Check if the credential has expired.
            if (is_expired(cred))
            {
                uds_stream.send(uchar_vec{});
                continue; 
            }

            if (cred.type == ASYMMETRIC)
            {
                int len;
                // Base64 decode the returned public key.
                // TODO Error check len because base64_decode might fail.
                unsigned char *public_store = base64_decode((unsigned char*) cred.pubKey.c_str(), (size_t *) &len);
                // DER decode the public key.
                RSA *public_key = DER_decode_RSA_public(public_store, len);

                // Verify the signature.
                bool validity = rsa_verify(public_key, sig, data, hash);

                // Free materials.
                RSA_free(public_key);
                free(public_store);

                // Send validity.
                uchar_vec ret_msg{};
                ret_msg.push_back(validity);
                uds_stream.send(ret_msg);
            }
            else
            {
                // TODO throw exception
            }

        }
        else
        {
            // TODO 
            // Invalid request.
            std::string log_msg{"esol invalid request: "};
            log_msg += to_string(recv_msg);
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
