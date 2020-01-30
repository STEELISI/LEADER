#ifndef LEADER_SESSION_H
#define LEADER_SESSION_H

#include <boost/interprocess/ipc/message_queue.hpp>
#include <boost/process.hpp>
#include <cstdio>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <unistd.h>
#include <vector>
#include "tbb/concurrent_unordered_map.h"
#include "tbb/concurrent_vector.h"

/**
 * The Connection class stores system calls related to a single connection.
 */
class Connection {
private:
  const std::vector<std::string> vect = {
          "sock_poll",         "sock_write_iter",    "sockfd_lookup_light",
          "sock_alloc_inode",  "sock_alloc",         "sock_alloc_file",
          "move_addr_to_user", "SYSC_getsockname",   "SyS_getsockname",
          "SYSC_accept4",      "sock_destroy_inode", "sock_read_iter",
          "sock_recvmsg",      "sock_sendmsg",       "__sock_release",
          "SyS_accept4",       "SyS_shutdown",       "sock_close"};
  std::mutex lock;
  bool tested = false;

public:
  tbb::concurrent_vector<std::string> syscall_list;
  unsigned int port = -1, tid = -1, pid = -1;
  std::string ip_addr;
  std::string toString();

  Connection() = default;
};

/**
 * The Session class stores a map of maps which can provide connection info for
 * multiple hosts, given IPs and port numbers. It suports multiple connections
 * from the same IP and port number at different times.
 */
class Session {
private:
  std::thread t_scanner;
  boost::interprocess::message_queue *mq = nullptr;

  // We change this to a single map of connections since we're tracking in real time
  tbb::concurrent_unordered_map<unsigned int, tbb::concurrent_unordered_map<unsigned int, Connection>> conns;
  boost::process::ipstream stap_out;
  boost::process::child stap_process;

  void scan(std::istream *in);
  void analyze();
public:
  Session();
  ~Session();
};

#endif
