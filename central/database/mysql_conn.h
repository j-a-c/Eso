#ifndef ESO_CENTRAL_APP_DATABASE_MYSQL_CONN
#define ESO_CENTRAL_APP_DATABASE_MYSQL_CONN

#include <string.h>
#include <tuple>
#include <vector>

#include "db_conn.h"
#include "db_error.h"
#include "db_types.h"
#include "mysql_config.h"
#include "../crypto/aes.h"
#include "../crypto/base64.h"
#include "../crypto/memory.h"
#include "../crypto/rsa.h"
#include "../../logger/logger.h"

// Include these after all other files because of the mix/max macro problems
#include <my_global.h>
#include <mysql.h>

/*
 * Table format:

CREATE TABLE permissions (
    set_name VARCHAR(255) NOT NULL,
    entity VARCHAR(32) NOT NULL,
    entity_type INT UNSIGNED NOT NULL,
    op INT UNSIGNED NOT NULL,
    primary KEY (set_name, entity)
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


class MySQL_Conn : public DB_Conn
{
public:
    MySQL_Conn();

    int create_permission(const char *set_name, const char *entity,
            const unsigned int entity_type, 
            const unsigned int op) const override;

    int update_permission(const char *set_name, const char *entity,
            const unsigned int op) const override;

    int delete_permission(const char* set_name, const char* entity) 
        const override;

    int create_credential(const char *set_name, 
            const unsigned int version, const char *expiration, 
            const char *primary, const char *secondary, 
            const unsigned int type, const char *algo, 
            const unsigned int size) const override;
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
 * Creates a new permission
 */
int MySQL_Conn::create_permission(const char *set_name, const char *entity,
        const unsigned int entity_type, const unsigned int op) const
{
    Logger::log("Entering create_permission(...)", LogLevel::Debug);
	
    // Form query
    std::string query = "INSERT INTO ";
    query.append(PERM_LOC);
    query += "(set_name, entity, entity_type, op) VALUES ('";
    query.append(set_name);
    query += "', '";
    query.append(entity); 
    query += "', ";
    query.append(std::to_string(entity_type));
    query += ", ";
    query.append(std::to_string(op));
    query += ")";

    Logger::log(query.c_str());

    int ret = perform_query(query.c_str());
        
    Logger::log("Exiting create_permission(...)", LogLevel::Debug);
    return ret;


}

/*
 * Updates a permission. Returns 0 if the permission was updated.
 */
int  MySQL_Conn::update_permission(const char *set_name, const char *entity,
            const unsigned int op) const
{
    Logger::log("Entering update_permission(...)", LogLevel::Debug);
	
    // Form query
    std::string query = "UPDATE ";
    query.append(PERM_LOC);
    query += " SET op=";
    query.append(std::to_string(op));
    query += " WHERE set_name='";
    query.append(set_name);
    query +="' and entity='";
    query.append(entity);
    query += "'";

    Logger::log(query.c_str());

    int ret = perform_query(query.c_str());
        
    Logger::log("Exiting update_permission(...)", LogLevel::Debug);
    return ret;

}

/*
 * Deletes the permission with the given set and entity names.
 * (set, entity) is the primary key for the permissions tables.
 */
int MySQL_Conn::delete_permission(const char *set_name, 
        const char *entity) const
{
    Logger::log("Entering delete_permission(...)", LogLevel::Debug);

    // Form query
    std::string query = "DELETE FROM ";
    query.append(PERM_LOC);
    query += " WHERE set_name='";
    query.append(set_name);
    query +="' and entity='";
    query.append(entity);
    query += "';";

    Logger::log(query.c_str());

    int ret = perform_query(query.c_str());
    
    Logger::log("Exiting delete_permission(...)", LogLevel::Debug);
    return ret;
}

/*
 * Creates a new credential.
 */
int MySQL_Conn::create_credential(const char *set_name, 
        const unsigned int version, const char *expiration, 
        const char *primary, const char *secondary, 
        const unsigned int type, const char *algo, 
        const unsigned int size) const
{
    
    Logger::log("Entering create_credential(...)", LogLevel::Debug);

    // TODO query to see if set_name already exists
	
    // Form query
    // TODO securely encrypt and mac inserted values.
    std::string query = "INSERT INTO ";
    query.append(CRED_LOC);
    query += "(set_name, version, expiration, p_owner, s_owner, type,";
    query += " algo, size, ";
    // Credential type
    switch(type)
    {
        case USERPASS:
            // TODO
            break;
        case SYMMETRIC:
            query += "pubKey, priKey";
            break;
        case ASYMMETRIC:
            query += "symKey";
            break;
        default:
            Logger::log("Invalid type: " + std::to_string(type));
            return INVALID_PARAMS;
    }
    query += ")";
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
    query += ", '";
    query.append(algo);
    query += "', ";
    query.append(std::to_string(size));
    query += ", ";
    // Credential data - no default case since an invalid param was handled
    // previously.
    if (type == USERPASS)
    {
        // TODO implement
        // TODO encrypt + mac
    }
    else if (type == SYMMETRIC)
    {
        // Get keys
        auto key_store = get_new_RSA_pair(size);
        unsigned char *pubKey = std::get<0>(key_store);
        int pubLen = std::get<1>(key_store);
        unsigned char *priKey = std::get<2>(key_store);
        int priLen = std::get<3>(key_store);

        // Encode keys
        unsigned char *pubKey_enc = base64_encode(pubKey, pubLen);
        unsigned char *priKey_enc = base64_encode(priKey, priLen);

        // Add to query
        // TODO encrypt + mac
        query += "'";
        query.append(reinterpret_cast<const char *>(pubKey_enc));
        query += "', ";
        query += "'";
        query.append(reinterpret_cast<const char *>(priKey_enc));
        query += "'";

        // Free keys
        free((void*)secure_memset(pubKey, 0, pubLen));
        free((void*)secure_memset(pubKey_enc, 0, 
                    strlen(reinterpret_cast<const char *>(pubKey_enc))));
        free((void*)secure_memset(priKey, 0, priLen));
        free((void*)secure_memset(priKey_enc, 0, 
                    strlen(reinterpret_cast<const char *>(priKey_enc))));
    }
    else if (type == ASYMMETRIC)
    {
        query += "'";
        // Get key and encode.
        unsigned char * key = get_new_AES_key(size);
        unsigned char * enc = base64_encode(key, size/8);

        // TODO encrypt + mac
        query.append(reinterpret_cast<const char *>(enc));

        // Securely erase key and free
        free((void*)secure_memset(key, 0, size/8)); // size is in bits
        free((void*)secure_memset(enc, 0, 
                    strlen(reinterpret_cast<const char *>(enc))));

        query += "'";
    }
    query += ")";

    Logger::log(query.c_str());

    int ret = perform_query(query.c_str());

    query.clear();
        
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
        results.push_back(std::make_tuple(mysqlRow[0], 
                    strtol(mysqlRow[1], nullptr, 0), 
                    strtol(mysqlRow[2], nullptr, 0), mysqlRow[3]));
    
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
        results.push_back(std::make_tuple(mysqlRow[0], 
                    strtol(mysqlRow[1], nullptr, 0), 
                    strtol(mysqlRow[2], nullptr, 0)));
    
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
