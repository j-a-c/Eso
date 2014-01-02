#ifndef ESO_HOST_DATABASE_MYSQL_CONN
#define ESO_HOST_DATABASE_MYSQL_CONN


#include "mysql_config.h"
#include "../../logger/logger.h"

// Include these after all other files because of the mix/max macro problems
#include <my_global.h>
#include <mysql.h>

/*
CREATE TABLE permissions (
    set_name VARCHAR(255) NOT NULL,
    op INT UNSIGNED NOT NULL,
    entity VARCHAR(32) NOT NULL,
    entity_type INT UNSIGNED NOT NULL,
    primary KEY (set_name, entity)
);

CREATE TABLE credentials (
    set_name VARCHAR(255) NOT NULL,
    version INT UNSIGNED NOT NULL,
    type INT UNSIGNED NOT NULL,
    algo INT UNSIGNED NOT NULL,
    size INT UNSIGNED NOT NULL,
    expiration DATE NOT NULL,
    edata VARBINARY(8192) NOT NULL,
    mdata VARBINARY(8192) NOT NULL,
    enonce VARBINARY(1024) NOT NULL,
    mnonce VARBINARY(1024) NOT NULL,
    PRIMARY KEY (set_name, version)
);
*/

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
    void log_error(MYSQL *conn) const;

};

MySQL_Conn::MySQL_Conn()
{
    MYSQL* cred_conn = mysql_init(nullptr);
    MYSQL* perm_conn = mysql_init(nullptr);

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
    std::string query = "INSERT INTO ";
    query.append(PERM_LOC);
    query += " VALUES(";
    query += "set_name";
    query += ",";
    query += "1";
    query += ",";
    query += "entity";
    query += ",";
    query += "2";
    query += (")");
    

    if (mysql_query(perm_conn, query.c_str())) 
        log_error(perm_conn);

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

void MySQL_Conn::log_error(MYSQL *conn) const
{
    Logger::log(mysql_error(conn), LogLevel::Error);
}


#endif
