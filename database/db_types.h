#ifndef ESO_DATABASE_DB_TYPES
#define ESO_DATABASE_DB_TYPES

/*
 * Credential types.
 */
const int USERPASS      = 1;
const int SYMMETRIC     = 2;
const int ASYMMETRIC    = 3;

/*
 * Entity-type types.
 */


/*
 * Operation types.
 */
const int RETRIEVE_OP   = 1;
const int SIGN_OP       = 2;
const int VERIFY_OP     = 4;
const int ENCRYPT_OP    = 8;
const int DECRYPT_OP    = 16;
const int HMAC_OP       = 32;

#endif
