#ifndef ESO_DATABASE_PERMISSION
#define ESO_DATABASE_PERMISSION

#include <string>

#include "../global_config/global_config.h"
#include "../global_config/message_config.h"
#include "../util/parser.h"

/**
 * Represents a Permission.
 */
class Permission
{
public:
    // Creates an empty Permission.
    Permission();
    // Creates a Permission from a serialized Permission.
    Permission(std::string);
    // Returns the serialized form of this Permission.
    std::string serialize() const;

    // The name of the set.
    std::string set_name;
    // The entity corresponding to the set.
    std::string entity;
    // The type of the entity. (user vs POSIX).
    int entity_type;
    // The operations the entity is allowed to invoke.
    int op;
    // The location of the entity.
    std::string loc;
};

/** 
 * The default constructor for an empty Permission.
 */
Permission::Permission()
{

}

/**
 * Creates a Permission from a serialized Permission. The values of the
 * serialization should be the exact same order as the members above.
 */
Permission::Permission(std::string serialization)
{

    auto values = split_string(serialization, MSG_DELIMITER);

    set_name = values[0];
    entity = values[1];
    entity_type = stol(values[2]);
    op = stol(values[3]);
    loc = values[4];
}

/**
 * Returns the serialized form of this Permission. The member values should be
 * serialized in the exact same order as they are listed above.
 */
std::string Permission::serialize() const
{
    std::string serialization{};

    serialization += set_name;
    serialization += MSG_DELIMITER;
    serialization += entity;
    serialization += MSG_DELIMITER;
    serialization += std::to_string(entity_type);
    serialization += MSG_DELIMITER;
    serialization += std::to_string(op);
    serialization += MSG_DELIMITER;
    serialization += loc;
    serialization += MSG_DELIMITER;

    return serialization;
}

#endif
