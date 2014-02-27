#ifndef ESO_GLOBAL_CONFIG_MESSAGE_CONFIG
#define ESO_GLOBAL_CONFIG_MESSAGE_CONFIG

#include "types.h"

/*
 * Some constants for connections.
 */

// Request that a new permission be created.
// Should be followed by the Permission to be updated at the destination.
char_vec NEW_PERM{'N','E','W','_','P','E','M'};

// Request a permission to be updated.
// Should be followed by the Permission to be updated at the destination.
char_vec UPDATE_PERM{'U','P','D','A','T','E','_','P','E','R','M'};

// Request a permission.
char_vec GET_PERM{'G','E','T','_','P','E','R','M'};

// Request a permission to be deleted.
// Should be followed by the Permission to be deleted.
char_vec DELETE_PERM{'D','E','L','E','T','E','_','P','E','R','M'};

// Request a credential. 
// Should be followed by set_name;version.
char_vec GET_CRED{'G','E','T','_','C','R','E','D'};

// Request a credential to be created.
// When sent from the appExtension to esocam it is follwed by the Credential. 
// esoca fills in the key fields.
// When sent from esoca to esod, it is followed by the serialized credential.
char_vec NEW_CRED{'N','E','W','_','C','R','E','D'};

// Used to ping one of the services.
char_vec PING{'P','I','N','G'};

// Used to request encryption services from the local daemon.
// Should be followed by the set, version, and then the actual data to encrypt.
char_vec REQUEST_ENCRYPT{'R','E','Q','U','E','S','T','_','E','N','C','R','Y','P','T'};

// Used to request decryption services from the local daemon.
// Should be followed by the set, version, and then the actual data to encrypt.
char_vec REQUEST_DECRYPT{'R','E','Q','U','E','S','T','_','D','E','C','R','Y','P','T'};

// Used to request HMAC services from the local daemon.
// Should be followed by the set, version, data, and then the hash type to use.
char_vec REQUEST_HMAC{'R','E','Q','U','E','S','T','_','H','M','A','C'};

// Used to request signing services from the local daemon.
// Should be followed by the set, version, data ,and then the hash type to use.
char_vec REQUEST_SIGN{'R','E','Q','U','E','S','T', '_','S','I','G','N'};

// Used to request verification services from the local daemon.
// Should be followed by the set, version, signature, data, and then hash type 
// to use.
char_vec REQUEST_VERIFY{'R','E','Q','U','E','S','T','_','V','E','R','I','F','Y'};

// The return value if a query is invalid for some reason. For example:
// requesting a non-existant credential from a distribution server.
char_vec INVALID_REQUEST{'I','N','V','A','L','I','D','_','R','E','Q','U','E','S','T'};


#endif
