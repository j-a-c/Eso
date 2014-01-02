#ifndef ESO_HOST_DATABASE_DB_CONN
#define ESO_HOST_DATABASE_DB_CONN

class DB_Conn
{
public:
    virtual int create_permission() const = 0;
    virtual int delete_permission() const = 0;
    virtual int add_operation() const = 0;
    virtual int remove_operation() const = 0;

    virtual int create_credential() const = 0;
    virtual int delete_credential() const = 0;

    virtual ~DB_Conn(){};
};


#endif
