#include <Python.h>

#include "database/mysql_conn.h"

/*
 * MySQL databse interface for the web app.
 * The only function permitted are creating a new set and editing an existing
 * set.
 */

/*
 * Function to be called from Python.
 * Creates a new set.
 *
 * Python arguments (in order) are set name, version, expiration date, primary,
 * and secondary. All inputs should be strings. They will be converted on this
 * side.
 *
 * Return value is 0 if everything went ok, nonzero otherwise.
 */
static PyObject* create_set(PyObject* self, PyObject* args)
{
	char *setName;
    // Version input is a string, but we need an unsigned int
    char *input_version;
    char *expiration;
    char *primary;
    char *secondary;

    // Parse input
    if (!PyArg_ParseTuple(args, "sssss", &setName, &input_version, 
                &expiration, &primary, &secondary))
        return nullptr;

    // atoi cannot return something in the range of an unsigned int.
    unsigned int version = strtol(input_version, nullptr, 0);

    // Attempt to update database.
    MySQL_Conn conn;
    int status = conn.create_credential(setName, version, expiration, 
            primary, secondary);

    // Return status (0 if ok, nonzero if error).
	return Py_BuildValue("i", status);
}

/*
 * Bind Python function names to our C functions
 */
static PyMethodDef databaseModule_methods[] = {
	{"create_set", create_set, METH_VARARGS},
	{NULL, NULL}
};



/*
 * Note: Functions that will be called by the Python interpreter
 * (in particular, module initialization functions) have to be 
 * declared using extern "C".
 */
extern "C"
{
    /*
     * Python calls this to let us initialize our module
     */
    void initdatabaseModule()
    {
        (void) Py_InitModule("databaseModule", databaseModule_methods);
    }
}
