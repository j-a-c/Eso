#include "db_conn.h"

class MySQL_Conn : public DB_Conn
{

public:
    MySQL_Conn(){};

    virtual int create_permission() const override;
    virtual int delete_permission() const override;
    virtual int add_operation() const override;
    virtual int remove_operation() const override;

    virtual int create_credential() const override;
    virtual int delete_credential() const override;
};

int MySQL_Conn::create_permission() const 
{
    return 0;
}

int MySQL_Conn::delete_permission() const 
{
    return 0;
}

int MySQL_Conn::add_operation() const
{
    return 0;
}

int MySQL_Conn::remove_operation() const
{
    return 0;
}

int MySQL_Conn::create_credential() const 
{
    return 0;
}
int MySQL_Conn::delete_credential() const 
{
    return 0;
}
