#ifndef ESO_CENTRAL_APP_DATABASE_DB_CONN
#define ESO_CENTRAL_APP_DATABASE_DB_CONN

class DB_Conn
{
public:
    // Insert a new permission.
    virtual int create_permission(const char *set, const unsigned int op, 
            const char *entity, const unsigned int entity_type) const = 0;

    /* 
     * Delete the permission with the given attributes.
     * (set, entity) should be a unique identified for a permission.
     */
    virtual int delete_permission(const char *set, 
            const char *entity) const = 0;

    // Add an operation to the permission.
    virtual int add_operation(const char *set, 
            const char *entity, const unsigned int op) const = 0;

    // Remove an operation from the permission.
    virtual int remove_operation(const char *set, 
            const char *entity, const unsigned int op) const = 0;

    // Create a new credential set.
    virtual int create_credential(const char *set_name, 
            const unsigned int version, const char *expiration, 
            const char *primary, const char *secondary, 
            const unsigned int type, const char *algo, 
            const unsigned int size) const = 0;

    // Delete a credential.
    virtual int delete_credential() const = 0;

    // Get the credentials associated with the given set.
    virtual std::vector<std::tuple<char *, unsigned int, unsigned int, 
            char *>> get_credentials(const char *) const = 0;
    // Get the permissions associated with the given set.
    virtual std::vector<std::tuple<char *, unsigned int, unsigned int>> 
            get_permissions(const char *) const = 0;

    virtual ~DB_Conn(){};
};


#endif
