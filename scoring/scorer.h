#ifndef LEADER_CONSUMER_H
#define LEADER_CONSUMER_H
#include "python3.6/Python.h"
#include <string>

class Model {
private:
  PyObject *ml_model;
  //PyObject *ml_normalization;
  //PyObject *ml_standardization;
  PyObject *load_func, *test_func;

public:
  int analyze_conn(const std::string &in);
  explicit Model(const std::string &load_model, const std::string &load_normalization, const std::string &load_standardization);
  ~Model();
};

#endif // LEADER_CONSUMER_H
