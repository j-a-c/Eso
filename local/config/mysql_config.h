#ifndef ESO_CENTRAL_CONFIG_MYSQL_CONFIG
#define ESO_CENTRAL_CONFIG_MYSQL_CONFIG

/*
 * The configuration file for the central authority's MySQL database.
 */

// Address.
const char* LOC = "localhost";
// Table.
const char* DB_LOC = "eso_l";

/*
 * Harcoded user and password. It is advised that the central authority is
 * running on a dedicated machine.
 */
const char* HOST_USER = "eso_l";
const char* HOST_PASS = "esod123";

const char* CRED_LOC = "credentials";
const char* PERM_LOC = "permissions";

#endif
