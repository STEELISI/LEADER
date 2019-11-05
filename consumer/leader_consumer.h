#ifndef LEADER_CONSUMER_H
#define LEADER_CONSUMER_H
#include "python3.7m/Python.h"
#include <string>

class model {
private:
  PyObject *ml_model;
  PyObject *load_func, *test_func;

public:
  void load_model(FILE *ret);
  bool analyze_conn(const std::string &in);
  model();
  ~model();
};

#endif // LEADER_CONSUMER_H
