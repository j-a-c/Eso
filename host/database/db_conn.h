#ifndef ESO_HOST_DATABASE_DB_CONN
#define ESO_HOST_DATABASE_DB_CONN

class DB_Conn
{
public:
    // Insert a new permission
    virtual int create_permission(const char *set, const int op, 
            const char *entity, const int entity_type) const = 0;

    /* 
     * Delete the permission with the given attributes.
     * (set, entity) should be a unique identified for a permission.
     */
    virtual int delete_permission(const char *set, 
            const char *entity) const = 0;

    // Add an operation to the permission
    virtual int add_operation(const char *set, 
            const char *entity, const int op) const = 0;

    virtual int remove_operation() const = 0;

    virtual int create_credential() const = 0;

    virtual int delete_credential() const = 0;

    virtual ~DB_Conn(){};
};


#endif
