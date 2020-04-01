#include "mitigator.h"

mitigator::mitigator() {
  mitigator_process = boost::process::child(
      "/usr/bin/python3", boost::process::args({"mitigator/blacklistd.py"}));
  writer.open("/tmp/blacklistpipe");
}

mitigator::~mitigator() {
  mitigator_process.terminate();
  writer.close();
}

void mitigator::blacklist_ip(std::string addr) {
  writer << addr << std::endl;
  writer.flush();
}
