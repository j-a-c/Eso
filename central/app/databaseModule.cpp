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
 * secondary, type, algo, and size. All inputs should be strings. 
 * They will be converted on this side.
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
    char * algo;
    char * input_size;

    // Parse input
    if (!PyArg_ParseTuple(args, "ssssssss", &setName, &input_version, 
                &expiration, &primary, &secondary, &input_type, &algo, &input_size))
        return nullptr;

    // atoi cannot return something in the range of an unsigned int, so we will
    // parse it as an unsigned long.
    unsigned int version = strtol(input_version, nullptr, 0);
    unsigned int type = strtol(input_type, nullptr, 0);
    unsigned int size = strtol(input_size, nullptr, 0);

    // Attempt to update database.
    MySQL_Conn conn;
    int status = conn.create_credential(setName, version, expiration, 
            primary, secondary, type, algo, size);

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
 * Function to be called from Python.
 * Attempt to create a permission on a set.
 *
 * Python arguments in order are set_name, entity, entity_type, op.
 *
 * Returns 0 if everything went ok, nonzero otherwise.
 */
static PyObject* create_permission(PyObject* self, PyObject* args)
{
    char *set_name;
    char *entity;
    char *input_entity_type;
    char *input_op;

    // Parse input.
    if (!PyArg_ParseTuple(args, "ssss", &set_name, &entity, &input_entity_type,
                &input_op))
        return nullptr;

    unsigned int entity_type = strtol(input_entity_type, nullptr, 0);
    unsigned int op = strtol(input_op, nullptr, 0);
    
    // Attempt to update database.
    MySQL_Conn conn;
    int status = conn.create_permission(set_name, entity, entity_type, op);

    // Return status (0 if ok, nonzero if error).
	return Py_BuildValue("i", status);

}

/*
 * Function to be called from Python.
 * Attempt to update a permission on a set.
 *
 * Python arguments in order are set_name, entity, entity_type, op.
 *
 * Returns 0 if everything went ok, nonzero otherwise.
 */
static PyObject* update_permission(PyObject* self, PyObject* args)
{
    char *set_name;
    char *entity;
    char *input_op;

    // Parse input.
    if (!PyArg_ParseTuple(args, "sss", &set_name, &entity, &input_op))
        return nullptr;

    unsigned int op = strtol(input_op, nullptr, 0);
    
    // Attempt to update database.
    MySQL_Conn conn;
    int status = conn.update_permission(set_name, entity, op);

    // Return status (0 if ok, nonzero if error).
	return Py_BuildValue("i", status);

}

/*
 * Bind Python function names to our C/C++ functions
 */
static PyMethodDef databaseModule_methods[] = {
	{"create_credential", create_credential, METH_VARARGS},
    {"get_credentials", get_credentials, METH_VARARGS},
    {"get_permissions", get_permissions, METH_VARARGS},
    {"create_permission", create_permission, METH_VARARGS},
    {"update_permission", update_permission, METH_VARARGS},
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
