#include <Python.h>

/*
 * MySQL databse interface for the web app.
 * The only function permitted are creating a new set and editing an existing
 * set.
 */

/*
 * Function to be called from Python
 */
static PyObject* py_myFunction(PyObject* self, PyObject* args)
{
	char *s = "Hello from C!";
	return Py_BuildValue("s", s);
}

/*
 * Bind Python function names to our C functions
 */
static PyMethodDef databaseModule_methods[] = {
	{"myFunction", py_myFunction, METH_VARARGS},
	{NULL, NULL}
};

/*
 * Python calls this to let us initialize our module
 */
void initdatabaseModule()
{
	(void) Py_InitModule("databaseModule", databaseModule_methods);
}
