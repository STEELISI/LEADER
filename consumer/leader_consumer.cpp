#include "leader_consumer.h"

/**
 * Checks if a connection is an attack or not
 * @param in CSV string from leader_session
 * @return chance of attack from 0 to 1
 */
bool model::analyze_conn(const std::string &in) {}

/**
 * Loads a model into ml_model
 * @param ret void
 */
void model::load_model(FILE *ret) {}

/**
 * Destructor that deletes the PyObjects
 */
model::~model() {
  delete ml_model;
  delete load_func;
  delete test_func;
}

/**
 * Constructor that loads the PyObjects from consumer/ml.py
 */
model::model() {
  Py_Initialize();
  PyObject *module = PyImport_Import(PyUnicode_DecodeFSDefault("consumer/ml"));
  load_func = PyObject_GetAttrString(module, "load_model");
  test_func = PyObject_GetAttrString(module, "test_model");
}
