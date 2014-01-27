#ifndef ESO_DATABASE_PERMISSION
#define ESO_DATABASE_PERMISSION

#include <string>

/**
 * Represents a Permission.
 */
class Permission
{
public:
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

#endif
