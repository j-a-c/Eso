#ifndef ESO_CENTRAL_DATABASE_MYSQL_CONFIG
#define ESO_CENRTAL_DATABASE_MYSQL_CONFIG

const char* LOC = "localhost";
const char* DB_LOC = "esod_host";

/*
 * It is ok for these to be hardcoded since the database user 'esod' does not
 * actually have permissions to do anything to the tables that cannot be
 * detected and fixed by the daemon.
 */
const char* HOST_USER = "esod";
const char* HOST_PASS = "esod123";

const char* CRED_LOC = "credentials";
const char* PERM_LOC = "permissions";

#endif
