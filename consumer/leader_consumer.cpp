#include "leader_consumer.h"
#include <python3.7m/Python.h>

float analyze_conn(const std::string &in) {
  Py_SetProgramName(
      reinterpret_cast<const wchar_t *>("leader_ellipticenvelope"));
  Py_Initialize();


}
