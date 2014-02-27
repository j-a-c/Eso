// Need to include <Python.h> first!
#include <Python.h>

#include <exception>

#include "../config/esoca_config.h"
#include "../config/mysql_config.h"
#include "../../database/mysql_conn.h"
#include "../../global_config/global_config.h"
#include "../../global_config/message_config.h"
#include "../../socket/uds_socket.h"
#include "../../socket/uds_stream.h"


/*
 * Python extension module for the web app.
 *
 * Provide a MySQL databse interface for the web app.
 * The only functions permitted are creating a new set, editing an existing
 * set, and viewing info about a set.
 *
 * This allows the database functions (most importantly key generation) to be
 * implemented in C++.
 */

/**
 * Sends a request to esoca.
 * @param msg_type The type of the message (ex: NEW_CRED).
 * @param msg The actual message to send.
 */
int request_daemon(uchar_vec msg_type, std::string msg)
{
    // Socket to the CA daemon.
    UDS_Socket uds_socket{std::string{ESOCA_SOCKET_PATH}};

    int status;
    // Send Credential to esoca for completion.
    try
    {
        UDS_Stream uds_stream = uds_socket.connect();

        // Send request type.
        uds_stream.send(msg_type);

        Logger::log(std::string{"Sending to esoca: "} + msg, LogLevel::Debug);

        // Send the actual message.
        uds_stream.send(msg);

        status = 0;
    }
    catch (std::exception& e)
    {
        status = 1;
    }

    return status;
}

/*
 * Function to be called from Python.
 *
 * Requests that esoca created a new credential with the given parameters.
 *
 * Python arguments (in order) are set name, version, expiration date, primary,
 * secondary, type, algo, and size. All inputs should be strings. 
 * They will be converted on this side.
 *
 * Return value is 0 if everything went ok, nonzero otherwise.
 */
static PyObject* create_credential(PyObject* self, PyObject* args)
{
	char *set_name;
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
    if (!PyArg_ParseTuple(args, "ssssssss", &set_name, &input_version, 
                &expiration, &primary, &secondary, &input_type, &algo, &input_size))
        return nullptr;

    // atoi cannot return something in the range of an unsigned int, so we will
    // parse it as an unsigned long.
    unsigned int version = strtol(input_version, nullptr, 0);
    unsigned int type = strtol(input_type, nullptr, 0);
    unsigned int size = strtol(input_size, nullptr, 0);

    // Fill out credential fields.
    Credential cred;
    cred.set_name = set_name;
    cred.version = version;
    cred.expiration = expiration;
    cred.p_owner = primary;
    cred.s_owner = secondary;
    cred.type = type;
    cred.algo = algo;
    cred.size = size;

    int status = request_daemon(NEW_CRED, cred.serialize()); 

    // Return status (0 if ok, nonzero if error).
	return Py_BuildValue("i", status);
}

/*
 * Function to be called from Python.
 *
 * The function argument in Python should be a string containing the set name
 * to be queried.
 *
 * Returns a list of tuples: 
 *  (set_name, version, type, expiration, p_owner, s_owner).
 */
static PyObject* get_all_credentials(PyObject* self, PyObject* args)
{
    char *set_name;

    // Parse input.
    if (!PyArg_ParseTuple(args, "s", &set_name))
        return nullptr;

    // Attempt to get credentials.
    MySQL_Conn conn;
    std::vector<Credential> results = conn.get_all_credentials(set_name);

    // List to return.
    PyObject *pyList = PyList_New(0);
    // Tuple to append.
    PyObject *pyTup;
    
    // Populate the list.
    for (auto &cred : results) 
    {
        // TODO check for initialization
        pyTup = Py_BuildValue("(siissssi)", cred.set_name.c_str(), cred.version, 
                cred.type, cred.expiration.c_str(), cred.p_owner.c_str(), 
                cred.s_owner.c_str(), cred.algo.c_str(), cred.size);

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
 * Returns a list of tuples (entity, entity_type, operation, location).
 */
static PyObject* get_all_permissions(PyObject* self, PyObject* args)
{
    char *set_name;

    // Parse input
    if (!PyArg_ParseTuple(args, "s", &set_name))
        return nullptr;

    // Attempt to get permissions
    MySQL_Conn conn;
    std::vector<Permission> results = conn.get_all_permissions(set_name);

    // List to return.
    PyObject *pyList = PyList_New(0);
    // Tuple to append.
    PyObject *pyTup;
    
    // Populate the list.
    for (auto &perm : results) 
    {
        // TODO check for initialization
        pyTup = Py_BuildValue("(siis)", perm.entity.c_str(), perm.entity_type,
                perm.op, perm.loc.c_str());

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
    char *loc;

    // Parse input.
    if (!PyArg_ParseTuple(args, "sssss", &set_name, &entity, &input_entity_type,
                &input_op, &loc))
        return nullptr;

    unsigned int entity_type = strtol(input_entity_type, nullptr, 0);
    unsigned int op = strtol(input_op, nullptr, 0);

    // Populate the Permission.
    Permission perm;
    perm.set_name = std::string{set_name};
    perm.entity = std::string{entity};
    perm.entity_type = entity_type;
    perm.op = op;
    perm.loc = std::string{loc};

    // Request permission creation from esoca.
    int status = request_daemon(NEW_PERM, perm.serialize());

    // Return status (0 if ok, nonzero if error).
	return Py_BuildValue("i", status);

}

/*
 * Function to be called from Python.
 * Attempt to update a permission on a set.
 *
 * Python arguments in order are set_name, entity, op, loc.
 *
 * Returns 0 if everything went ok, nonzero otherwise.
 */
static PyObject* update_permission(PyObject* self, PyObject* args)
{
    char *set_name;
    char *entity;
    char *input_op;
    char *loc;

    // Parse input.
    if (!PyArg_ParseTuple(args, "ssss", &set_name, &entity, &input_op, &loc))
        return nullptr;

    unsigned int op = strtol(input_op, nullptr, 0);

    // Populate the permission.
    Permission perm;
    perm.set_name = std::string{set_name};
    perm.entity = std::string{entity};
    perm.op = op;
    perm.loc = std::string{loc};
 
    // Request update from esoca.
    int status = request_daemon(UPDATE_PERM, perm.serialize());

    // Return status (0 if ok, nonzero if error).
	return Py_BuildValue("i", status);
}

/*
 * Function to be called from Python.
 * Attempt to remove a permission on a set.
 *
 * Python arguments in order are set_name, entity, loc.
 *
 * Returns 0 if everything went ok, nonzero otherwise.
 */
static PyObject* remove_permission(PyObject* self, PyObject* args)
{
    char *set_name;
    char *entity;
    char *loc;

    // Parse input.
    if (!PyArg_ParseTuple(args, "sss", &set_name, &entity, &loc))
        return nullptr;

    // Create permission object.
    Permission perm;
    perm.set_name = std::string{set_name};
    perm.entity = std::string{entity};
    perm.loc = std::string{loc};

    int status = request_daemon(DELETE_PERM, perm.serialize());

    // Return status (0 if ok, nonzero if error).
	return Py_BuildValue("i", status);

}

/*
 * Bind Python function names to our C/C++ functions
 */
static PyMethodDef appExtension_methods[] = {
	{"create_credential", create_credential, METH_VARARGS},
    {"get_all_credentials", get_all_credentials, METH_VARARGS},
    {"get_all_permissions", get_all_permissions, METH_VARARGS},
    {"create_permission", create_permission, METH_VARARGS},
    {"update_permission", update_permission, METH_VARARGS},
    {"remove_permission", remove_permission, METH_VARARGS},
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
    void initappExtension()
    {
        (void) Py_InitModule("appExtension", appExtension_methods);
    }
}
