// From https://csl.name/post/c-functions-python/

#include <Python.h>

#include <stdio.h>

// These are wrapper functions that the Python server is calling into
// in order to launch classification.

double run_classification(int* samples, int n, double** keep_output);

static PyObject* py_run_cnn_classifier(PyObject* self, PyObject* args)
{
  PyObject *input = PyList_New(0);

  if (!PyArg_ParseTuple(args, "O!", &PyList_Type, &input)) {
    return NULL;
  }

  int* samples = (int*)malloc(sizeof(int)*PyList_Size(input));
  for (int i = 0; i < PyList_Size(input); i++) {
    samples[i] = (int)PyInt_AsLong(PyList_GetItem(input, (Py_ssize_t)i));
  }

  double dt = run_classification(samples, PyList_Size(input), NULL);

  for (int i = 0; i < PyList_Size(input); i++) {
    PyList_SetItem(input, (Py_ssize_t)i, PyInt_FromLong(samples[i]));
  }

  free(samples);

  return Py_BuildValue("d", dt);
}

static PyMethodDef myModule_methods[] = {
  {"RunCNNClassifier", py_run_cnn_classifier, METH_VARARGS},
  {NULL, NULL}
};

void initcnnModule()
{
  (void) Py_InitModule("cnnModule", myModule_methods);
}
