#include "leader_consumer.h"

/**
 * Checks if a connection is an attack or not
 * @param in CSV string from leader_session
 * @return chance of attack from 0 to 1
 */
int Model::analyze_conn(const std::string &in) {
  PyObject* conn = Py_BuildValue("y#", in.c_str(), in.size());
  PyObject* ret = PyObject_CallObject(test_func, conn);
  int r = PyNumber_Check(ret);

  Py_DECREF(conn);
  Py_DECREF(ret);
  return r;
}

/**
 * Destructor that deletes unneeded stuff
 */
Model::~Model() {
  Py_DECREF(load_func);
  Py_DECREF(test_func);
  Py_DECREF(ml_model);
}

/**
 * Constructor that loads the PyObjects from consumer/ml.py
 */
 Model::Model(const std::string& load) {
  Py_Initialize();

  PyRun_SimpleString("import sys");
  PyRun_SimpleString("import os");
  PyRun_SimpleString("sys.path.append(os.getcwd())");
  PyObject *module = PyImport_Import(PyUnicode_DecodeFSDefault("consumer.ml"));

  load_func = PyObject_GetAttrString(module, "load_model");
  test_func = PyObject_GetAttrString(module, "test_model");

  PyObject* str = PyTuple_New(1);
  PyTuple_SetItem(str, 0, Py_BuildValue("y#", load.c_str(), load.size()));
  ml_model = PyObject_CallObject(load_func, str);

  Py_DECREF(str);
  Py_DECREF(module);
  PyErr_Print();
}
