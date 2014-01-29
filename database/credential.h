#ifndef ESO_DATABASE_CREDENTIAL
#define ESO_DATABASE_CREDENTIAL

#include <string>

#include "../global_config/global_config.h"
#include "../global_config/message_config.h"
#include "../util/parser.h"

/**
 * Represents a Credential from the database.
 * Not all of these fields maybe be used. Particulary, the 'type' field
 * determines which of the key fields will be used.
 */
class Credential
{
public:
    // Create an empty credential.
    Credential();
    // Create a credential from a string returned from serialize().
    Credential(std::string);
    // Serialize the credential.
    std::string serialize() const;

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

/**
 * Default constructor for a Credential.
 */
Credential::Credential()
{

}

/**
 * Create a Credential from a string return from serialize().
 * The serialization should be the exact order of the member above.
 */
Credential::Credential(std::string serialization)
{
    auto values = split_string(serialization, MSG_DELIMITER);

    set_name = values[0];
    version = stol(values[1]);
    type = stol(values[2]);
    algo = values[3];
    size = stol(values[4]);
    p_owner = values[5];
    s_owner = values[6];
    expiration = values[7];
    symKey = values[8];
    priKey = values[9];
    pubKey = values[10];
    user = values[11];
    pass = values[12];
}

/**
 * Serialize the credential. The credential should be serialized in the exact
 * order as the members are listed above.
 */
std::string Credential::serialize() const
{
    std::string serialization{};

    serialization += set_name;
    serialization += MSG_DELIMITER;
    serialization.append(std::to_string(version));
    serialization += MSG_DELIMITER;
    serialization.append(std::to_string(type));
    serialization += MSG_DELIMITER;
    serialization += algo;
    serialization += MSG_DELIMITER;
    serialization.append(std::to_string(size));
    serialization += MSG_DELIMITER;
    serialization += p_owner;
    serialization += MSG_DELIMITER;
    serialization += s_owner;
    serialization += MSG_DELIMITER;
    serialization += expiration;
    serialization += MSG_DELIMITER;
    serialization += symKey;
    serialization += MSG_DELIMITER;
    serialization += priKey;
    serialization += MSG_DELIMITER;
    serialization += pubKey;
    serialization += MSG_DELIMITER;
    serialization += user;
    serialization += MSG_DELIMITER;
    serialization += pass;
    serialization += MSG_DELIMITER;

    return serialization;
}

#endif
