#ifndef eso_host_database_mysql_config
#define eso_host_database_mysql_config


/*
 * Username and password for the esod host tables.
 * It is not really a big deal for someone to know this because this is not
 * the root mysql account and this account does not have any permissions to
 * alter the state of the table in any way that the daemon cannot detect and
 * fix. Encrypted data cannot be decrypted because only the daemon knows the
 * key used for encryption.
 */
const char* HOST_USER = "esod";
const char* HOST_PASS = "esod123";

// Table names for esod.
const char* CRED_LOC = "esod_host.credentials";
const char* PERM_LOC = "esod_host.permissions";

#endif
