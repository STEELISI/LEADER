#ifndef MITIGATOR_H
#define MITIGATOR_H

#include <boost/process.hpp>
#include <fstream>
#include <string>

class mitigator {
private:
  boost::process::child mitigator_process;
  std::ofstream writer;
public:
  mitigator();
  ~mitigator();
  void blacklist_ip(std::string addr);
};

#endif
