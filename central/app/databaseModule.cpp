#include <Python.h>

#include "database/mysql_conn.h"
#include "../crypto/rsa.h"

/*
 * MySQL databse interface for the web app.
 * The only functions permitted are creating a new set, editing an existing
 * set, and viewing info about a set.
 *
 * This allows the database functions (most importantly key generation) to be
 * implemented in C++.
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
static PyObject* create_credential(PyObject* self, PyObject* args)
{
	char *setName;
    // Version input is a string, but we need an unsigned int
    char *input_version;
    char *expiration;
    char *primary;
    char *secondary;
    // Type input is a string, but we need an unsigned int
    char * input_type;

    // Parse input
    if (!PyArg_ParseTuple(args, "ssssss", &setName, &input_version, 
                &expiration, &primary, &secondary, &input_type))
        return nullptr;

    // atoi cannot return something in the range of an unsigned int, so we will
    // parse it as an unsigned long.
    unsigned int version = strtol(input_version, nullptr, 0);
    unsigned int type = strtol(input_type, nullptr, 0);

    // Attempt to update database.
    MySQL_Conn conn;
    int status = conn.create_credential(setName, version, expiration, 
            primary, secondary, type);

    // Return status (0 if ok, nonzero if error).
	return Py_BuildValue("i", status);
}

/*
 * Function to be called from Python.
 *
 * The function argument in Python should be a string containing the set name
 * to be queried.
 *
 * Returns a list of tuples (set_name, version, type, expiration).
 */
static PyObject* get_credentials(PyObject* self, PyObject* args)
{
    char *set_name;

    // Parse input.
    if (!PyArg_ParseTuple(args, "s", &set_name))
        return nullptr;

    // Attempt to get credentials.
    MySQL_Conn conn;
    std::vector<std::tuple<char*, unsigned int, unsigned int, char *>> 
        results = conn.get_credentials(set_name);

    // List to return.
    PyObject *pyList = PyList_New(0);
    // Tuple to append.
    PyObject *pyTup;
    
    // Populate the list.
    for (auto &tup : results) 
    {
        // TODO check for initialization
        pyTup = Py_BuildValue("(siis)", std::get<0>(tup), std::get<1>(tup), 
                std::get<2>(tup), std::get<3>(tup));

        // TODO check for initialization
        PyList_Append(pyList, pyTup);
        Py_DECREF(pyTup);
    }

    return pyList;
}

/*
 * Function to be called from Python.
 *
 * The function argument in Python should be a string containing the set name
 * to be queried.
 *
 * Returns a list of tuples (entity, entity_type, operation).
 */
static PyObject* get_permissions(PyObject* self, PyObject* args)
{
    char *set_name;

    // Parse input
    if (!PyArg_ParseTuple(args, "s", &set_name))
        return nullptr;

    // Attempt to get permissions
    MySQL_Conn conn;
    std::vector<std::tuple<char*, unsigned int, unsigned int>> results = 
        conn.get_permissions(set_name);

    // List to return.
    PyObject *pyList = PyList_New(0);
    // Tuple to append.
    PyObject *pyTup;
    
    // Populate the list.
    for (auto &tup : results) 
    {
        // TODO check for initialization
        pyTup = Py_BuildValue("(sii)", std::get<0>(tup), std::get<1>(tup), 
                std::get<2>(tup));

        // TODO check for initialization
        PyList_Append(pyList, pyTup);
        Py_DECREF(pyTup);
    }

    return pyList;
}

/*
 * Bind Python function names to our C functions
 */
static PyMethodDef databaseModule_methods[] = {
	{"create_credential", create_credential, METH_VARARGS},
    {"get_credentials", get_credentials, METH_VARARGS},
    {"get_permissions", get_permissions, METH_VARARGS},
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
