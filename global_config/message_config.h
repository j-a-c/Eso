#ifndef ESO_GLOBAL_CONFIG_MESSAGE_CONFIG
#define ESO_GLOBAL_CONFIG_MESSAGE_CONFIG

#include <string>

/*
 * Some constants for connections.
 */

// Request a permission to be updated.
// Should be followed by the Permission to be updated at the destination.
std::string UPDATE_PERM{"update_permission"};

// Request a permission.
std::string GET_PERM{"get_permission"};

// Request a credential. 
// Should be followed by set_name;version.
std::string GET_CRED{"get_credential"};

// Request a credential to be created.
// When sent from the appExtension to esocam it is follwed by the Credential. 
// esoca fills in the key fields.
// When sent from esoca to esod, it is followed by the serialized credential.
std::string NEW_CRED("new_credential");

// Used to ping one of the services.
std::string PING{"ping"};

// Used to request encryption services from the local daemon.
// Should be followed by set_name;version;data_to_encrypt.
std::string REQUEST_ENCRYPT("request_encrypt");

// The return value if a query is invalid for some reason. For example:
// requesting a non-existant credential from a distribution server.
std::string INVALID_REQUEST{"invalid_request"};

// Terminates the current message.
// Message should NOT ever contain this character, as it is used
// internally by the socket streams to delimiter buffered messages.
std::string MSG_END{"!"};

#endif
