#ifndef ESO_CENTRAL_APP_DATABASE_DB_ERROR
#define ESO_CENTRAL_APP_DATABASE_DB_ERROR


/* 
 * Error return codes for the database. 
 */

// Nothing went wrong.
const int OK = 0;

// Unable to connect to MySQL server.
const int CANNOT_CONNECT = 1;

// Unable to perform query.
const int CANNOT_QUERY = 2;

// More rows were returned that expected.
const int TOO_MANY_ROWS = 3;

// No results were returned, when some were expected.
const int NO_RESULTS = 4;

// A row with the same primary key already exists.
const int ALREADY_EXISTS = 5;

// Invalid parameters where given to the query.
const int INVALID_PARAMS = 6;

#endif
