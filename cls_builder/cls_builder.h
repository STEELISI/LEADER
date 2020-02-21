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
#include "tbb/concurrent_unordered_map.h"

/**
 * The Connection class stores system calls related to a single connection.
 */
struct Connection {
  tbb::concurrent_unordered_map<std::string, unsigned int> syscall_list_count;
  tbb::concurrent_unordered_map<std::string, unsigned int> syscall_list_time;
  unsigned int port = -1, tid = -1, pid = -1;
  std::string ip_addr;

  Connection() = default;
  std::string toString();
};

/**
 * The Session class stores a map of maps which can provide connection info for
 * multiple hosts, given IPs and port numbers. It suports multiple connections
 * from the same IP and port number at different times.
 */
class Session {
private:
  std::set<std::string> useful_calls;

  boost::interprocess::message_queue *mq = nullptr;
  std::thread t_scanner;
  std::thread msg_push;

  // We change this to a single map of connections since we're tracking in real time
  tbb::concurrent_unordered_map<unsigned int, tbb::concurrent_unordered_map<unsigned int, Connection>> conns;
  boost::process::ipstream stap_out;
  boost::process::child stap_process;

  void scan(std::istream *in);
  void push();
public:
  Session();
  ~Session();
};

#endif
