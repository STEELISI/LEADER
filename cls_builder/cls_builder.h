#ifndef LEADER_SESSION_H
#define LEADER_SESSION_H

#include <boost/interprocess/ipc/message_queue.hpp>
#include <boost/process.hpp>
#include <cstdio>
#include <memory>
#include <string>
#include <thread>
#include <unistd.h>
#include "tbb/concurrent_unordered_map.h"
#include "tbb/concurrent_vector.h"

/**
 * The Call class stores a single system call and data related to it.
 */
struct Call {
  std::string syscall_name;
  int page_faults;
  int descriptors;
  int mem_alloc;
};

/**
 * The Connection class stores system calls related to a single connection.
 */
struct Connection {
  std::vector<Call> syscall_list;
  int port = -1, tid = -1, pid = -1;
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
  std::thread t_scanner;

  tbb::concurrent_unordered_map<std::string,
                     tbb::concurrent_unordered_map<int, tbb::concurrent_vector<Connection *>>>
      connections;
  tbb::concurrent_unordered_map<int, tbb::concurrent_unordered_map<int, tbb::concurrent_vector<Connection>>>
      conns;
  boost::process::ipstream stap_out;
  boost::process::child stap_process;

public:
  boost::interprocess::message_queue *mq = nullptr;
  void scan(std::istream *in);

  Session();
  ~Session();

  tbb::concurrent_vector<int> get_connection_ports(const std::string &ip);
  tbb::concurrent_vector<Connection> get_connection(const std::string &ip, int port);
  int get_size();
  void remove_conn(Connection *c);
};

#endif
