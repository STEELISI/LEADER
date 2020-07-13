#include "scorer.h"
#include <iostream>

/**
 * Checks if a connection is an attack or not
 * @param in CSV string from leader_session
 * @return chance of attack from 0 to 1
 */
int Model::analyze_conn(const std::string &in) {
  Py_XINCREF(ml_model);

  PyObject *conn = PyTuple_New(2);
  PyTuple_SetItem(conn, 0, PyBytes_FromString(in.c_str()));
  PyTuple_SetItem(conn, 1, ml_model);


  PyObject *ret = PyObject_CallObject(test_func, conn);
  int r = static_cast<int>(PyLong_AsLong(ret));
  std::cout <<"\n\nSCORE: "<< r << " for conn: " << in.c_str();

  Py_XDECREF(conn);
  Py_XDECREF(ret);
  return r;
}

/**
 * Destructor that deletes unneeded stuff
 */
Model::~Model() {
  Py_XDECREF(load_func);
  Py_XDECREF(test_func);
  Py_XDECREF(ml_model);
  //Py_XDECREF(ml_normalization);
  //Py_XDECREF(ml_standardization);
}

/**
 * Constructor that loads the PyObjects from consumer/ml.py
 */
Model::Model(const std::string &load, const std::string &load_normalization, const std::string &load_standardization) {
  Py_Initialize();

  PyRun_SimpleString("import sys");
  PyRun_SimpleString("import os");
  PyRun_SimpleString("sys.path.append(os.getcwd())");
  PyObject *module = PyImport_Import(PyUnicode_DecodeFSDefault("scoring.ml"));

  load_func = PyObject_GetAttrString(module, "load_model");
  test_func = PyObject_GetAttrString(module, "test_model");

  PyObject *str = PyTuple_New(1);
  PyTuple_SetItem(str, 0, Py_BuildValue("y#", load.c_str(), load.size()));
  //PyTuple_SetItem(str, 1, Py_BuildValue("y#", load.c_str(), load.size()));
  //PyTuple_SetItem(str, 2, Py_BuildValue("y#", load.c_str(), load.size()));
  ml_model = PyObject_CallObject(load_func, str);

  Py_XDECREF(str);
  Py_XINCREF(ml_model);
  //Py_XINCREF(ml_normalization);
  //Py_XINCREF(ml_standardization);
}
