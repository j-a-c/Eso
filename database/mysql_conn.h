#ifndef ESO_DATABASE_MYSQL_CONN
#define ESO_DATABASE_MYSQL_CONN

#include <string.h>
#include <vector>

#include "credential.h"
#include "db_error.h"
#include "db_types.h"
#include "permission.h"
#include "../logger/logger.h"

// Include these after all other files because of the min/max macro problems
#include <my_global.h>
#include <mysql.h>

/*
 * Table format:

CREATE TABLE permissions (
    set_name VARCHAR(255) NOT NULL,
    entity VARCHAR(32) NOT NULL,
    entity_type INT UNSIGNED NOT NULL,
    op INT UNSIGNED NOT NULL,
	loc VARCHAR(300) NOT NULL,
    primary KEY (set_name, entity, loc)
);

CREATE TABLE credentials (
    set_name VARCHAR(255) NOT NULL,
    version INT UNSIGNED NOT NULL,
    type INT UNSIGNED NOT NULL,
    algo VARCHAR(255) NOT NULL,
    size INT UNSIGNED NOT NULL,
	p_owner VARCHAR(32) NOT NULL,
	s_owner VARCHAR(32) NOT NULL,
    expiration VARCHAR(32) NOT NULL,
	symKey VARBINARY(8192),
	priKey VARBINARY(8192),
	pubKey VARBINARY(8192),
	user VARBINARY(8192),
	pass VARBINARY(8192),
    PRIMARY KEY (set_name, version)
);

*/


class MySQL_Conn
{
public:
    MySQL_Conn();

    // Create a new permission.
    int create_permission(const Permission perm) const;

    // Attempts to insert a new permission, but updates the op value if the
    // permission already exists.
    int insert_permission(const Permission perm) const;

    // Update the permission with the given set name and entity with the
    // specified operation value.
    int update_permission(const Permission perm) const;

    // Delete the permission with the given set name and entity.
    // This is the primary key for the permissions table.
    int delete_permission(const Permission perm) const;

    // Creates a new credential, using all the input fields from the given
    // credential.
    int create_credential(const Credential) const;
    
    // TODO
    int delete_credential() const;

    // Get the credential with the given primary key.
    Credential get_credential(const Credential cred) const;

    // Get all credentials associated with the given set name. 
    // Only the set_name, version, type, and expiration fields will be filled.
    std::vector<Credential> get_all_credentials(const char *set_name) const;

    // Get the permission with the given set name, enitity, and location.
    // This is the primary key for the permissions table.
    Permission get_permission(const Permission perm) const;

    // Get all permissions associated with the given set name.
    std::vector<Permission> get_all_permissions(const char *set_name) const;

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
 * Creates a new permission. Does not do "ON DUPLICATE KEY UPDATE" because we
 * want to create a credential only once, and not update it if someone else
 * creates a credential with the same primary key.
 */
int MySQL_Conn::create_permission(const Permission perm) const
{
    Logger::log("Entering MySQL_Conn::create_permission()", LogLevel::Debug);
	
    // Form query
    std::string query{"INSERT INTO "};
    query.append(PERM_LOC);
    query += "(set_name, entity, entity_type, op, loc) VALUES ('";
    query.append(perm.set_name);
    query += "', '";
    query.append(perm.entity); 
    query += "', ";
    query.append(std::to_string(perm.entity_type));
    query += ", ";
    query.append(std::to_string(perm.op));
    query += ", '";
    query.append(perm.loc);
    query +="')";

    Logger::log(query.c_str());

    int ret = perform_query(query.c_str());
        
    std::string log_msg{"Exiting MySQL_Conn::create_permission() with return = "};
    log_msg += std::to_string(ret);
    Logger::log(log_msg, LogLevel::Debug);
    return ret;


}

/*
 * Updates a permission. Returns 0 if the permission was updated.
 */
int  MySQL_Conn::update_permission(const Permission perm) const
{
    Logger::log("Entering MySQL_Conn::update_permission()", LogLevel::Debug);
	
    // Form query
    std::string query{"UPDATE "};
    query.append(PERM_LOC);
    query += " SET op=";
    query.append(std::to_string(perm.op));
    query += " WHERE set_name='";
    query.append(perm.set_name);
    query += "' and entity='";
    query.append(perm.entity);
    query += "' AND loc='";
    query.append(perm.loc);
    query += "'";

    Logger::log(query.c_str());

    int ret = perform_query(query.c_str());
        
    std::string log_msg{"Exiting MySQL_Conn::update_permission() with return = "};
    log_msg += std::to_string(ret);
    Logger::log(log_msg, LogLevel::Debug);
    return ret;
}

/*
 * Insert a permission. Updates the op value if the key already exists. 
 * Returns 0 if the permission was inserted/updated successfully.
 */
int MySQL_Conn::insert_permission(const Permission perm) const 
{
    Logger::log("Entering MySQL_Conn::insert_permission()", LogLevel::Debug);

    // Form query
    std::string query{"INSERT INTO "};
    query.append(PERM_LOC);
    query += "(set_name, entity, entity_type, op, loc) VALUES ('";
    query.append(perm.set_name);
    query += "', '";
    query.append(perm.entity); 
    query += "', ";
    query.append(std::to_string(perm.entity_type));
    query += ", ";
    query.append(std::to_string(perm.op));
    query += ", '";
    query.append(perm.loc);
    query += "') ON DUPLICATE KEY UPDATE op=VALUES(op)";

    Logger::log(query.c_str());

    int ret = perform_query(query.c_str());

    std::string log_msg{"Exiting MySQL_Conn::insert_permission() with return = "};
    log_msg += std::to_string(ret);
    Logger::log(log_msg, LogLevel::Debug);
    return ret;

}

/*
 * Deletes the permission with the given set and entity names.
 * (set, entity) is the primary key for the permissions tables.
 */
int MySQL_Conn::delete_permission(const Permission perm) const
{
    Logger::log("Entering MySQL_Conn::delete_permission()", LogLevel::Debug);

    // Form query
    std::string query{"DELETE FROM "};
    query.append(PERM_LOC);
    query += " WHERE set_name='";
    query.append(perm.set_name);
    query += "' and entity='";
    query.append(perm.entity);
    query += "' AND loc='";
    query.append(perm.loc);
    query += "';";

    Logger::log(query.c_str());

    int ret = perform_query(query.c_str());
    
    std::string log_msg{"Exiting MySQL_Conn::delete_permission() with return = "};
    log_msg += std::to_string(ret);
    Logger::log(log_msg, LogLevel::Debug);
    return ret;
}

/**
 * Creates a new credential in the database from the given credential.
 * Returns 0 on success.
 */
int MySQL_Conn::create_credential(const Credential cred) const
{
    Logger::log("Entering MySQL_Conn::create_credential()", LogLevel::Debug);

    // TODO query to see if set_name already exists
	
    // Form query
    // TODO securely encrypt and mac inserted values.
    std::string query{"INSERT INTO "};
    query.append(CRED_LOC);
    query += "(set_name, version, type, algo, size, p_owner, s_owner, ";
    query += "expiration, ";
    switch (cred.type)
    {
        case USERPASS:
            query += "user, pass";
            break;
        case SYMMETRIC:
            query += "symKey";
            break;
        case ASYMMETRIC:
            query += "priKey, pubKey";
            break;
    }
    query += ") VALUES('";
    query += cred.set_name;
    query += "', ";
    query.append(std::to_string(cred.version));
    query += ", ";
    query.append(std::to_string(cred.type));
    query += ", '";
    query += cred.algo;
    query += "', ";
    query.append(std::to_string(cred.size));
    query += ", '";
    query += cred.p_owner;
    query += "', '";
    query += cred.s_owner;
    query += "', '";
    query += cred.expiration;
    query += "', ";
    switch (cred.type)
    {
        case USERPASS:
            query += "'";
            query += cred.user;
            query += "', '";
            query += cred.pass;
            query += "'";
            break;
        case SYMMETRIC:
            query += "'";
            query += cred.symKey;
            query += "'";
            break;
        case ASYMMETRIC:
            query += "'";
            query += cred.priKey;
            query += "', '";
            query += cred.pubKey;
            query += "'";
            break;
    }
    query += ")";

    Logger::log(query);

    int ret = perform_query(query.c_str());

    query.clear();
        
    std::string log_msg{"Exiting MySQL_Conn::create_credential() with return = "};
    log_msg += std::to_string(ret);
    Logger::log(log_msg, LogLevel::Debug);

    return ret;
}


/**
 * TODO
 */
int MySQL_Conn::delete_credential() const
{
    // TODO
    return 0;

}

/**
 * Returns the credential with the given primary key (set_name, version).
 * May return an empty credential if no such credential exists in the current
 * database.
 */
Credential MySQL_Conn::get_credential(const Credential cred) const
{
    Logger::log("Entering MySQL_Conn::get_credential()", LogLevel::Debug);

    // Build query.
    std::string query{"SELECT set_name, version, type, algo, size, expiration, "};
    query += "symKey, priKey, pubKey, user, pass, p_owner, s_owner FROM ";
    query += CRED_LOC;
    query += " WHERE set_name='";
    query.append(cred.set_name);
    query += "' AND version=";
    query.append(std::to_string(cred.version));
    query +=";";

    Logger::log(query);

    // Get results.
    MYSQL_RES* mysqlResult = get_result(query.c_str());

    Credential ret;
    // Pack query results into return value.
    MYSQL_ROW mysqlRow;
    // Row pointer in the result set
    // There may be 0 or 1 results.
    while ( (mysqlRow = mysql_fetch_row(mysqlResult)) )
    {
        ret.set_name = std::string{mysqlRow[0]};
        ret.version = strtol(mysqlRow[1], nullptr, 0);
        ret.type = strtol(mysqlRow[2], nullptr, 0);
        ret.algo = std::string{mysqlRow[3]};
        ret.size = strtol(mysqlRow[4], nullptr, 0);
        ret.expiration = std::string{mysqlRow[5]};
        switch (ret.type)
        {
            case SYMMETRIC:
                ret.symKey = std::string{mysqlRow[6]};
                break;
            case ASYMMETRIC:
                ret.priKey = std::string{mysqlRow[7]};
                ret.pubKey = std::string{mysqlRow[8]};
                break;
            case USERPASS:
                ret.user = std::string{mysqlRow[9]};
                ret.pass = std::string{mysqlRow[10]};
                break;
        }
        ret.p_owner = std::string{mysqlRow[11]};
        ret.s_owner = std::string{mysqlRow[12]};
    }
    
    mysql_free_result(mysqlResult); 

    Logger::log("Exiting MySQL_Conn::get_credential()", LogLevel::Debug);

    return ret;
}

/*
 * Returns a vector of Credentials representing the entries from query.
 */
std::vector<Credential> MySQL_Conn::get_all_credentials(const char * set_name) const
{
    Logger::log("Entering MySQL_Conn::get_all_credentials()", LogLevel::Debug);

    // Return value.
    std::vector<Credential> results;

    // Build query.
    std::string query{"SELECT set_name, version, type, expiration, p_owner, s_owner FROM "};
    query += CRED_LOC;
    query += " WHERE set_name='";
    query.append(set_name);
    query += "';";

    Logger::log(query);

    // Get results.
    MYSQL_RES* mysqlResult = get_result(query.c_str());

    // Pack query results into return value.
    MYSQL_ROW mysqlRow;
    // Row pointer in the result set
    while ( (mysqlRow = mysql_fetch_row(mysqlResult)) )
    {
        Credential cred;
        cred.set_name = std::string{mysqlRow[0]};
        cred.version = strtol(mysqlRow[1], nullptr, 0);
        cred.type = strtol(mysqlRow[2], nullptr, 0);
        cred.expiration = std::string{mysqlRow[3]};
        cred.p_owner = std::string{mysqlRow[4]};
        cred.s_owner = std::string{mysqlRow[5]};

        results.push_back(cred);
    }
    
    mysql_free_result(mysqlResult); 
    
    Logger::log("Exiting MySQL_Conn::get_all_credentials()", LogLevel::Debug);

    return results;
}

/*
 * Returns a Permission representing the entries from the query.
 * There should only be one Permission because (set_name, entity, loc) is the 
 * primary key for the permissions table.

    set_name VARCHAR(255) NOT NULL,
    entity VARCHAR(32) NOT NULL,
    entity_type INT UNSIGNED NOT NULL,
    op INT UNSIGNED NOT NULL,
    loc VARCHAR(300) NOT NULL
*/
Permission MySQL_Conn::get_permission(const Permission perm) const
{
    Logger::log("Entering MySQL_Conn::get_permissions()", LogLevel::Debug);

    // Build query.
    std::string query{"SELECT set_name, entity, entity_type, op, loc FROM "}; 
    query += PERM_LOC;
    query += " WHERE set_name='";
    query.append(perm.set_name);
    query += "' AND entity='";
    query.append(perm.entity);
    query += "' AND loc='";
    query.append(perm.loc);
    query += "';";

    Logger::log(query);

    // Get results.
    MYSQL_RES* mysqlResult = get_result(query.c_str());

    // Return value.
    Permission result;

    // Row pointer in the result set.
    MYSQL_ROW mysqlRow;
    // Pack query results into return value.
    // There should only be one return value, since (set_name,entity, loc) is the
    // primary key for the permissions table.
    // There may be 0 or 1 results.
    while ( (mysqlRow = mysql_fetch_row(mysqlResult)) )
    {
        result.set_name= std::string{mysqlRow[0]};
        result.entity = std::string{mysqlRow[1]};
        result.entity_type = strtol(mysqlRow[2], nullptr, 0);
        result.op = strtol(mysqlRow[3], nullptr, 0);
        result.loc = std::string{mysqlRow[4]};
    }
    
    mysql_free_result(mysqlResult); 

    Logger::log("Exiting MySQL_Conn::get_permissions()", LogLevel::Debug);

    return result;

}


/*
 * Returns a vector of Permission representing the entries from query.
 */
std::vector<Permission>
        MySQL_Conn::get_all_permissions(const char * set_name) const
{
    Logger::log("Entering MySQL_Conn::get_all_permissions()", LogLevel::Debug);

    // Build query.
    std::string query{"SELECT entity, entity_type, op, loc FROM "}; 
    query += PERM_LOC;
    query += " WHERE set_name='";
    query.append(set_name);
    query += "';";

    Logger::log(query);

    // Get results.
    MYSQL_RES* mysqlResult = get_result(query.c_str());

    // Return value.
    std::vector<Permission> results;

    // Row pointer in the result set
    MYSQL_ROW mysqlRow;
    // Pack query results into return value.
    while ( (mysqlRow = mysql_fetch_row(mysqlResult)) )
    {
        Permission perm;
        perm.entity = std::string{mysqlRow[0]};
        perm.entity_type = strtol(mysqlRow[1], nullptr, 0);
        perm.op = strtol(mysqlRow[2], nullptr, 0);
        perm.loc = std::string{mysqlRow[3]};

        results.push_back(perm);
    }
    
    mysql_free_result(mysqlResult); 

    Logger::log("Exiting MySQL_Conn::get_all_permissions()", LogLevel::Debug);

    return results;
}

/*
 * Logs any errors that occur due to the MySQL connection
 */
void MySQL_Conn::log_error(MYSQL *conn) const
{
    Logger::log("Entering MySQL_Conn::log_error().", LogLevel::Debug);
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
