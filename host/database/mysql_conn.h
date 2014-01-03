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
    int create_permission(const char *set, const int op, const char *entity, 
            const int entity_type) const override;
    int delete_permission() const override;
    int add_operation() const override;
    int remove_operation() const override;

    int create_credential() const override;
    int delete_credential() const override;

    ~MySQL_Conn();

private:
    void log_error(MYSQL *conn) const;
    int perform_query(const char *query) const;

};

MySQL_Conn::MySQL_Conn()
{

}

MySQL_Conn::~MySQL_Conn()
{

}

/*
 * Creates a new permission. Returns 0 if the permission was successfully
 * created.
 */
int MySQL_Conn::create_permission(const char *set, const int op, 
        const char *entity, const int entity_type) const 
{
    Logger::log("Entering create_permission(...)", LogLevel::Debug);
	
    // Form query
    std::string query = "INSERT INTO ";
    query.append(PERM_LOC);
    query += " VALUES ('";
    query.append(set);
    query += "',";
    query.append(std::to_string(op));
    query += ",'";
    query.append(entity); 
    query += "',";
    query.append(std::to_string(entity_type));
    query += ")";

    Logger::log(query.c_str());

    int ret = perform_query(query.c_str());
        
    Logger::log("Exiting create_permission(...)", LogLevel::Debug);
    return ret;
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

/*
 * Logs any errors that occur due to the MySQL connection
 */
void MySQL_Conn::log_error(MYSQL *conn) const
{
    Logger::log("Entering log_error.", LogLevel::Debug);
    Logger::log(mysql_error(conn), LogLevel::Error);
}

/*
 * Performs the given query in the default database as the MySQL user esod.
 * Returns 0 if no error occured, else the error is logged.
 */
int MySQL_Conn::perform_query(const char* query) const 
{
    MYSQL* conn = nullptr;
    conn = mysql_init(nullptr);

    // Connect to database
    if(!mysql_real_connect(conn, LOC, 
            HOST_USER, HOST_PASS, DB_LOC, 0, nullptr, 0))
    {
        log_error(conn);
        return 1;
    }

    // Perform query
    if(mysql_query(conn, query))
    {
        log_error(conn);
        return 2;
    }

    mysql_close(conn);
    return 0;
}


#endif
