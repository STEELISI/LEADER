#ifndef LEADER_SESSION_H
#define LEADER_SESSION_H

#include <boost/interprocess/ipc/message_queue.hpp>
#include <boost/process.hpp>
#include <cstdio>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <thread>
#include <unistd.h>
#include "tbb/concurrent_hash_map.h"

/**
 * The Connection class stores system calls related to a single connection.
 */
class Connection {
private:
  bool tested = false;

public:
  tbb::concurrent_hash_map<std::string, unsigned int> syscall_list;
  unsigned int port = -1, tid = -1, pid = -1;
  std::string ip_addr;

  Connection() = default;
};

/**
 * The Session class stores a map of maps which can provide connection info for
 * multiple hosts, given IPs and port numbers. It suports multiple connections
 * from the same IP and port number at different times.
 */
class Session {
private:
  std::set<std::string> useful_calls;
  std::thread t_scanner;
  boost::interprocess::message_queue *mq = nullptr;

  // We change this to a single map of connections since we're tracking in real time
  tbb::concurrent_hash_map<unsigned int, tbb::concurrent_hash_map<unsigned int, Connection>> conns;
  boost::process::ipstream stap_out;
  boost::process::child stap_process;

  void scan(std::istream *in);
public:
  Session();
  ~Session();
};

#endif
