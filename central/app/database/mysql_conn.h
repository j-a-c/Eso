#ifndef ESO_CENTRAL_APP_DATABASE_MYSQL_CONN
#define ESO_CENTRAL_APP_DATABASE_MYSQL_CONN

#include <tuple>
#include <vector>

#include "db_conn.h"
#include "db_error.h"
#include "mysql_config.h"
#include "../../../logger/logger.h"

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
    p_owner VARCHAR(32) NOT NULL,
    s_owner VARCHAR(32) NOT NULL,
    expiration DATE NOT NULL,
    edata VARBINARY(8192),
    mdata VARBINARY(8192),
    enonce VARBINARY(1024),
    mnonce VARBINARY(1024),
    PRIMARY KEY (set_name, version)
);

*/


class MySQL_Conn : public DB_Conn
{
public:
    MySQL_Conn();
    int create_permission(const char *set, const unsigned int op, 
            const char *entity, const unsigned int entity_type) const override;
    int delete_permission(const char* set, const char* entity) const override;
    int add_operation(const char *set, const char * entity, 
            const unsigned int op) const override;
    int remove_operation(const char *set, const char *entity, 
            const unsigned int op) const override; 

    int create_credential(const char *set_name, 
            const unsigned int version, const char *expiration, 
            const char *primary, const char *secondary, 
            const unsigned int type) const override;
    int delete_credential() const override;

    std::vector<std::tuple<char *, unsigned int, unsigned int, 
            char *>> get_credentials(const char *) const override;

    std::vector<std::tuple<char *, unsigned int, unsigned int>> 
            get_permissions(const char *) const override;

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
int MySQL_Conn::create_permission(const char *set, const unsigned int op, 
        const char *entity, const unsigned int entity_type) const 
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
        const unsigned int op) const 
{
    Logger::log("Entering add_operation(...)", LogLevel::Debug);

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
        return NO_RESULTS;

    /* Get the number of rows in the result set. Then, we will make sure there
     * is only one result. This error should never occur since (set, entity) is
     * the primary key, but we will check just in case. */
    if (mysql_num_rows(mysqlResult) != 1)
        return TOO_MANY_ROWS;

    MYSQL_ROW mysqlRow = mysql_fetch_row(mysqlResult);
    // This is the original operation value.
    unsigned int opValue = atoi(mysqlRow[0]);

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
    int ret = perform_query(query.c_str());

    Logger::log("Exiting add_operation(...)", LogLevel::Debug);
    return ret;
}

int MySQL_Conn::remove_operation(const char *set, const char *entity, 
        const unsigned int op) const 
{
    Logger::log("Entering remove_operation(...)", LogLevel::Debug);

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
        return NO_RESULTS;

    /* Get the number of rows in the result set. Then, we will make sure there
     * is only one result. This error should never occur since (set, entity) is
     * the primary key, but we will check just in case. */
    if (mysql_num_rows(mysqlResult) != 1)
        return TOO_MANY_ROWS;

    MYSQL_ROW mysqlRow = mysql_fetch_row(mysqlResult);
    // This is the original operation value.
    unsigned int opValue = atoi(mysqlRow[0]);
    
    // Update the operation value.
    opValue &= ~op;

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
    int ret = perform_query(query.c_str());

    Logger::log("Exiting remove_operation(...)", LogLevel::Debug);
    return ret;

}

int MySQL_Conn::create_credential(const char *set_name, 
        const unsigned int version, const char *expiration, 
        const char *primary, const char *secondary, 
        const unsigned int type) const
{
    
    Logger::log("Entering create_credential(...)", LogLevel::Debug);

    // TODO query to see if set_name already exists
	
    // Form query
    std::string query = "INSERT INTO ";
    query.append(CRED_LOC);
    query += "(set_name, version, expiration, p_owner, s_owner, type)";
    query += " VALUES ('";
    query.append(set_name);
    query += "', ";
    query.append(std::to_string(version));
    query += ", '";
    query.append(expiration); 
    query += "', '";
    query.append(primary); 
    query += "', '";
    query.append(secondary);
    query += "', ";
    query.append(std::to_string(type));
    query += ")";

    Logger::log(query.c_str());

    int ret = perform_query(query.c_str());
        
    Logger::log("Exiting create_credential(...)", LogLevel::Debug);

    return ret;

}

int MySQL_Conn::delete_credential() const
{
    // TODO
    return 0;

}

/*
 * Returns a vector of tuples representing the entries from query.
 * The tuple entries are (set_name, version, type, operation).
 */
std::vector<std::tuple<char *, unsigned int, unsigned int, 
            char *>> MySQL_Conn::get_credentials(const char * set_name) const
{
    Logger::log("Entering get_credentials(...)", LogLevel::Debug);

    // Return value.
    std::vector<std::tuple<char *, unsigned int, unsigned int, 
        char *>> results;

    // Build query.
    std::string query = "SELECT set_name, version, type, expiration FROM ";
    query += CRED_LOC;
    query += " WHERE set_name='";
    query.append(set_name);
    query += "';";

    Logger::log(query);

    // Get results.
    MYSQL_RES* mysqlResult = get_result(query.c_str());

    // Pack query results into return value.
    MYSQL_ROW mysqlRow;
    while(mysqlRow = mysql_fetch_row(mysqlResult)) // row pointer in the result set
        results.push_back(std::make_tuple(mysqlRow[0], atoi(mysqlRow[1]), 
                    atoi(mysqlRow[2]), mysqlRow[3]));
    
    mysql_free_result(mysqlResult); 
    
    Logger::log("Exiting get_credentials(...)", LogLevel::Debug);

    return results;
}

/*
 * Returns a vector of tuples representing the entries from query.
 * The tuple entries are (entity, entity_type, operation).
 */
std::vector<std::tuple<char *, unsigned int, unsigned int>>
        MySQL_Conn::get_permissions(const char * set_name) const
{
    Logger::log("Entering get_permissions(...)", LogLevel::Debug);

    // Return value.
    std::vector<std::tuple<char *, unsigned int, 
        unsigned int>> results;

    // Build query.
    std::string query = "SELECT entity, entity_type, op FROM "; 
    query += PERM_LOC;
    query += " WHERE set_name='";
    query.append(set_name);
    query += "';";

    Logger::log(query);

    // Get results.
    MYSQL_RES* mysqlResult = get_result(query.c_str());

    // Pack query results into return value.
    MYSQL_ROW mysqlRow;
    while(mysqlRow = mysql_fetch_row(mysqlResult)) // row pointer in the result set
        results.push_back(std::make_tuple(mysqlRow[0], atoi(mysqlRow[1]), 
                    atoi(mysqlRow[2])));
    
    mysql_free_result(mysqlResult); 

    Logger::log("Exiting get_permissions(...)", LogLevel::Debug);

    return results;
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
        return CANNOT_CONNECT;
    }

    // Perform query
    if(int ret = mysql_query(conn, query))
    {
        log_error(conn);
        return ret;
    }

    mysql_close(conn);
    return OK;
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
