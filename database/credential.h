#ifndef ESO_DATABASE_CREDENTIAL
#define ESO_DATABASE_CREDENTIAL

#include <string>

/**
 * Represents a Credential from the database.
 * Not all of these fields maybe be used. Particulary, the 'type' field
 * determines which of the key fields will be used.
 */
class Credential
{
public:
    // The set name of the credential.
    std::string set_name;
    // Version of the credential.
    unsigned int version;
    // The credential type (symmetric, asymmetric, etc).
    unsigned int type;
    // The algorithm this credential should be used with.
    std::string algo;
    // The size of the credential.
    unsigned int size;
    // The primary owner of the credential.
    std::string p_owner;
    // The secondary owner of the credential.
    std::string s_owner;
    // The expiration date of the credential.
    std::string expiration;
    // The key fields.
    std::string symKey;
    std::string priKey;
    std::string pubKey;
    std::string user;
    std::string pass;
};

#endif
