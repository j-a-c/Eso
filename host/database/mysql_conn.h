#include <my_global.h>
#include <mysql.h>

#include "db_conn.h"
#include "mysql_config.h"

class MySQL_Conn : public DB_Conn
{

public:
    MySQL_Conn();

    virtual int create_permission() const override;
    virtual int delete_permission() const override;
    virtual int add_operation() const override;
    virtual int remove_operation() const override;

    virtual int create_credential() const override;
    virtual int delete_credential() const override;

    virtual ~MySQL_Conn() override 
    { 
        mysql_close(cred_conn);
        mysql_close(perm_conn);
    }

private:
    // Permissions database connection.
    MYSQL *perm_conn;
    // Credentials database connection.
    MYSQL *cred_conn;
    void log_and_quit(const MYSQL* conn) const;
};

MySQL_Conn::MySQL_Conn()
{
    perm_conn = mysql_init(nullptr); 
    cred_conn = mysql_init(nullptr);

    if (!perm_conn)
    {
        log_and_quit(perm_conn);
        exit(1);
    }

    if (!cred_conn)
    {
        log_and_quit(cred_conn);
    }

    if (!mysql_real_connect(perm_conn, "localhost", HOST_USER, HOST_PASS,
                PERM_LOC, 0, nullptr, 0))
    {
        log_and_quit(perm_conn); 
    }

    if (!mysql_real_connect(cred_conn, "localhost", HOST_USER, HOST_PASS,
                CRED_LOC, 0, nullptr, 0))
    {
        log_and_quit(cred_conn); 
    }
}

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

void MySQL_Conn::log_and_quit(const MYSQL* conn) const
{
    // TODO logger
    exit(1);
}
