#ifndef ESO_GLOBAL_CONFIG_MESSAGE_CONFIG
#define ESO_GLOBAL_CONFIG_MESSAGE_CONFIG

#include <string>

/*
 * Some constants for connections.
 */

// Request that a new permission be created.
// Should be followed by the Permission to be updated at the destination.
std::string NEW_PERM{"NEW_PERM"};

// Request a permission to be updated.
// Should be followed by the Permission to be updated at the destination.
std::string UPDATE_PERM{"UPDATE_PERM"};

// Request a permission.
std::string GET_PERM{"GET_PERM"};

// Request a permission to be deleted.
// Should be followed by the Permission to be deleted.
std::string DELETE_PERM{"DELETE_PERM"};

// Request a credential. 
// Should be followed by set_name;version.
std::string GET_CRED{"GET_CRED"};

// Request a credential to be created.
// When sent from the appExtension to esocam it is follwed by the Credential. 
// esoca fills in the key fields.
// When sent from esoca to esod, it is followed by the serialized credential.
std::string NEW_CRED("NEW_CRED");

// Used to ping one of the services.
std::string PING{"PING"};

// Used to request encryption services from the local daemon.
// Should be followed by the Credential (with set_name and version specified) 
// and then the actual data to encrypt.
std::string REQUEST_ENCRYPT("REQUEST_ENCRYPT");

// Used to request decryption services from the local daemon.
// Should be followed by the Credential (with set_name and version specified) 
// and then the actual data to encrypt.
std::string REQUEST_DECRYPT("REQUEST_DECRYPT");

// Used to request HMAC services from the local daemon.
// Should be followed by the set, data, version, and hash type to use.
std::string REQUEST_HMAC("REQUEST_HMAC");

// The return value if a query is invalid for some reason. For example:
// requesting a non-existant credential from a distribution server.
std::string INVALID_REQUEST{"INVALID_REQUEST"};

// TODO - we can't send 'Hello World!' Using this schema.. untill we encrypt
// transmissions.
// Terminates the current message.
// Message should NOT ever contain this character, as it is used
// internally by the socket streams to delimiter buffered messages.
std::string MSG_END{"!"};

#endif
