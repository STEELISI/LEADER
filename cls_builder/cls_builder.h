#ifndef LEADER_SESSION_H
#define LEADER_SESSION_H

#include <boost/interprocess/ipc/message_queue.hpp>
#include <boost/process.hpp>
#include <cstdio>
#include <memory>
#include <string>
#include <thread>
#include <unistd.h>
#include <unordered_map>
#include <vector>

/**
 * The Call class stores a single system call and data related to it.
 */
struct Call {
  std::string syscall_name;
  int page_faults;
  int descriptors;
  int mem_alloc;
  int call_time;
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

  std::unordered_map<std::string,
                     std::unordered_map<int, std::vector<Connection *>>>
      connections;
  std::unordered_map<int, std::unordered_map<int, std::vector<Connection>>>
      conns;
  boost::process::ipstream stap_out;
  boost::process::child stap_process;

public:
  boost::interprocess::message_queue *mq = nullptr;
  void scan(std::istream *in);

  Session();
  ~Session();

  std::vector<int> get_connection_ports(const std::string &ip);
  std::vector<Connection> get_connection(const std::string &ip, int port);
  int get_size();
  void remove_conn(Connection *c);
};

#endif
