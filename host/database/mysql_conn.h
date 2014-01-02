#ifndef ESO_HOST_DATABASE_MYSQL_CONN
#define ESO_HOST_DATABASE_MYSQL_CONN

#include <my_global.h>
#include <mysql.h>

#include "mysql_config.h"

class MySQL_Conn : public DB_Conn
{
public:
    MySQL_Conn();
    int create_permission() const override;
    int delete_permission() const override;
    int add_operation() const override;
    int remove_operation() const override;

    int create_credential() const override;
    int delete_credential() const override;

    ~MySQL_Conn();

private:
    MYSQL* cred_conn;
    MYSQL* perm_conn;
    void log_error(const MYSQL *conn) const;

};

MySQL_Conn::MySQL_Conn()
{
    MYSQL* cred_conn = mysql_init(nullptr);
    MYSQL* perm_conn = mysql_init(nullptr);

    if (!cred_conn) 
        log_error(cred_conn);

    if (!mysql_real_connect(cred_conn, LOC, HOST_USER, HOST_PASS, 
                PERM_LOC, 0, nullptr, 0))
        log_error(cred_conn);

    if (!perm_conn) 
        log_error(perm_conn);

    if (!mysql_real_connect(perm_conn, LOC, HOST_USER, HOST_PASS, 
                PERM_LOC, 0, nullptr, 0)) 
        log_error(perm_conn);
}

MySQL_Conn::~MySQL_Conn()
{
    mysql_close(cred_conn);
    mysql_close(perm_conn);
}

int MySQL_Conn::create_permission() const
{
    // TODO
    return 0;
}

int MySQL_Conn::delete_permission() const
{
    // TODO
    return 0;

}

int MySQL_Conn::add_operation() const 
{
    // TODO
    return 0;

}

int MySQL_Conn::remove_operation() const
{
    // TODO
    return 0;

}

int MySQL_Conn::create_credential() const
{
    // TODO
    return 0;

}

int MySQL_Conn::delete_credential() const
{
    // TODO
    return 0;

}

void MySQL_Conn::log_error(const MYSQL *conn) const
{
    // TODO
}


#endif
