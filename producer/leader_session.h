#ifndef LEADER_SESSION_H
#define LEADER_SESSION_H

#include <algorithm>
#include <boost/tokenizer.hpp>
#include <iostream>
#include <istream>
#include <regex>
#include <string>
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
  std::unordered_map<std::string,
                     std::unordered_map<int, std::vector<Connection*>>>
      connections;
  std::unordered_map<int, std::unordered_map<int, std::vector<Connection>>> conns;
public:
  void scan(std::istream* in);
  std::vector<int> get_connection_ports(std::string ip);
  std::vector<Connection> get_connection(std::string ip, int port);
  int get_size();
};

#endif
