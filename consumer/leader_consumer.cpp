#include "leader_consumer.h"

/**
 * Checks if a connection is an attack or not
 * @param in CSV string from leader_session
 * @return chance of attack from 0 to 1
 */
bool Model::analyze_conn(const std::string &in) {}

/**
 * Loads a model into ml_model
 * @param ret void
 */
void Model::load_model(std::string load) {
  PyObject* str = Py_BuildValue("y#", load.c_str(), load.size());
  ml_model = PyObject_CallObject(load_func, str);
}

/**
 * Destructor that deletes the PyObjects
 */
Model::~Model() {
  delete ml_model;
  delete load_func;
  delete test_func;
}

/**
 * Constructor that loads the PyObjects from consumer/ml.py
 */
Model::Model() {
  Py_Initialize();
  PyObject *module = PyImport_Import(PyUnicode_DecodeFSDefault("consumer.ml"));
  load_func = PyObject_GetAttrString(module, "load_model");
  test_func = PyObject_GetAttrString(module, "test_model");
}
