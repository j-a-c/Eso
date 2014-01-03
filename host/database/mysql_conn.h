#ifndef ESO_HOST_DATABASE_MYSQL_CONN
#define ESO_HOST_DATABASE_MYSQL_CONN

#include <cstdlib>

#include "mysql_config.h"
#include "../../logger/logger.h"

// Include these after all other files because of the mix/max macro problems
#include <my_global.h>
#include <mysql.h>

/*
 * Table format:

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
    int delete_permission(const char* set, const char* entity) const override;
    int add_operation(const char *set, const char * entity, 
            const int op) const override;
    int remove_operation() const override;

    int create_credential() const override;
    int delete_credential() const override;

    ~MySQL_Conn();

private:
    void log_error(MYSQL *conn) const;
    int perform_query(const char *query) const;
    MYSQL_RES* get_result(const char *query) const;

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

/*
 * Deletes the permission with the given set and entity names.
 * (set, entity) is the primary key for the permissions tables.
 */
int MySQL_Conn::delete_permission(const char *set, const char *entity) const
{
    Logger::log("Entering delete_permission(...)", LogLevel::Debug);

    // Form query
    std::string query = "DELETE FROM ";
    query.append(PERM_LOC);
    query += " WHERE set_name='";
    query.append(set);
    query +="' and entity='";
    query.append(entity);
    query += "';";

    Logger::log(query.c_str());

    int ret = perform_query(query.c_str());
    
    Logger::log("Exiting delete_permission(...)", LogLevel::Debug);
    return ret;
}

/*
 * Add an operation to the permission specified by (set, entity).
 * Assumes the operation value is a valid operation.
 */
int MySQL_Conn::add_operation(const char *set, const char *entity, 
        const int op) const 
{
    Logger::log("Entering add_operation(...)", LogLevel::Debug);

    int ret = 3;

    // Form select query to get the original operation value.
    std::string query = "SELECT op FROM ";
    query.append(PERM_LOC);
    query += " WHERE set_name='";
    query.append(set);
    query +="' and entity='";
    query.append(entity);
    query += "';";

    Logger::log(query.c_str());

    // Get the original operation value.
    MYSQL_RES* mysqlResult = get_result(query.c_str());
    if (!mysqlResult)
        return ret;

    /* Get the number of rows in the result set. Then, we will make sure there
     * is only one result. This error should never occur since (set, entity) is
     * the primary key, but we will check just in case. */
    int numRows = mysql_num_rows(mysqlResult);
    if (numRows != 1)
        return 2;

    MYSQL_ROW mysqlRow = mysql_fetch_row(mysqlResult);
    // This is the original operation value.
    int opValue = atoi(mysqlRow[0]);

    // Update the operation value.
    opValue |= op;

    // Free the result set.
    mysql_free_result(mysqlResult); 

    // Form update query to update the operation value.
    query.clear();
    query = "UPDATE ";
    query.append(PERM_LOC);
    query += " SET op=";
    query.append(std::to_string(opValue));
    query += " WHERE set_name='";
    query.append(set);
    query +="' and entity='";
    query.append(entity);
    query += "';";

    Logger::log(query.c_str());

    // Update the operation value.
    ret = perform_query(query.c_str());

    Logger::log("Exiting add_operation(...)", LogLevel::Debug);
    return ret;
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

/*
 * Returns the results from the given query.
 * nullptr is return if there was an error or no results.
 */
MYSQL_RES* MySQL_Conn::get_result(const char* query) const
{
    MYSQL* conn = nullptr;
    conn = mysql_init(nullptr);
    MYSQL_RES* mysqlResult = nullptr;

    // Connect to database
    if(!mysql_real_connect(conn, LOC, 
            HOST_USER, HOST_PASS, DB_LOC, 0, nullptr, 0))
    {
        log_error(conn);
        return mysqlResult;
    }

    // Perform query
    if(mysql_query(conn, query))
    {
        log_error(conn);
        return mysqlResult;
    }

    // Get result set
    mysqlResult = mysql_store_result(conn);

    mysql_close(conn);

    return mysqlResult;
}

#endif
