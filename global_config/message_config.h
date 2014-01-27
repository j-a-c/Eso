#ifndef ESO_GLOBAL_CONFIG_MESSAGE_CONFIG
#define ESO_GLOBAL_CONFIG_MESSAGE_CONFIG

#include <string>

/*
 * Some constants for connections.
 */

// Request preceding permissions that need to be updated.
std::string UPDATE_PERM{"update_permission"};

// Request preceding permissions that want to be updated.
std::string GET_PERM{"get_permission"};

// Used to ping one of the services.
std::string PING{"ping"};

std::string REQUEST_ENCRYPT("request_encrypt");

// Terminates the current message.
// Message should NOT ever contain this character, as it is used
// internally by the socket streams to delimiter buffered messages.
std::string MSG_END{"!"};

#endif
