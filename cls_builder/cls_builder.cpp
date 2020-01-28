#include "cls_builder.h"
#include <boost/tokenizer.hpp>
#include <exception>
#include <iostream>
#include <regex>
#include <vector>

// enum for the CSV inputs made by Systemtap
enum CSV {
  func,
  req,
  mem,
  fault,
  file,
  cycles,
  global_time,
  curr_time,
  call_time,
  tid,
  pid,
  addr,
  port
};

/**
 * The constructor spins off a new thread and runs start_stap() on that thread
 */
Session::Session() {
  // Initialize message queue
  boost::interprocess::message_queue::remove("conns");
  mq = new boost::interprocess::message_queue(
      boost::interprocess::create_only, "conns", 100, 4096);

  // Start stap process and scanning
  stap_process = boost::process::child(
      "/usr/local/bin/stap",
      boost::process::args({"-g", "./cls_builder/conn.stp",
                            "--suppress-handler-errors", "-DMAXMAPENTRIES=8096",
                            "-s4095", "-DINTERRUPTIBLE=0", "-DMAXTRYLOCK=10000",
                            "-DSTP_OVERLOAD_THRESHOLD=50000000000",
                            "--suppress-time-limits"}),
      boost::process::std_out > stap_out);
  this->t_scanner = std::thread(&Session::scan, this, &stap_out);
}

/**
 * The destructor joins t_scanner and removes the message queue
 */
Session::~Session() {
  boost::interprocess::message_queue::remove("conns");
  stap_process.terminate();
  t_scanner.join();
}

/**
 * This session creates a thread which scans a given stream for formatted input
 * and adds it to the corresponding connection.
 * @param in An istream to read from
 */
void Session::scan(std::istream *in) {
  std::string line;
  std::regex csv_match(
      "((?:[a-zA-Z0-9_]*))(,)(\\d+)(,)(\\d+)(,)(\\d+)(,)(\\d+)(,)(\\d+"
      ")(,)(\\d+)(,)(\\d+)(,)(\\d+)(,)(\\d+)(,)(\\d+)(,)(.*?)(,)(.*?)");

  while (std::getline(*in, line)) {
    std::cout << "line in: " << line << std::endl;
    // Only match if it is a data line and not extra stuff
    if (std::regex_match(line, csv_match)) {
      std::cout << "line match" << std::endl;
      // Call to add to a connection
      Call this_call;
      unsigned int this_tid, this_pid, conn_port;
      bool has_port = false;
      std::string ip;

      // Turn the CSV into a boost::tokenizer for easy parsing
      unsigned int count = 0;
      boost::tokenizer<boost::escaped_list_separator<char>> tk(
          line, boost::escaped_list_separator<char>('\\', ',', '\"'));

      // Add each element of the line into a Call object
      for (boost::tokenizer<boost::escaped_list_separator<char>>::iterator i(
               tk.begin());
           i != tk.end(); ++i) {

        if (count == func)
          this_call.syscall_name = *i;
        else if (count == mem)
          this_call.mem_alloc = std::stoi(*i);
        else if (count == file)
          this_call.descriptors = std::stoi(*i);
        else if (count == fault)
          this_call.page_faults = std::stoi(*i);
        else if (count == mem)
          this_call.mem_alloc = std::stoi(*i);
        else if (count == tid)
          this_tid = std::stoi(*i);
        else if (count == pid)
          this_pid = std::stoi(*i);
        else if (count == addr && *i != "-1") {
          ip = *i;
          has_port = true;
        } else if (count == port && *i != "-1") {
          conn_port = std::stoi(*i);
          has_port = true;
        }

        count++;
      }
      std::cout << "stap call - tid: " << this_tid << std::endl;

      // Get accessors for hash maps
      tbb::concurrent_hash_map<unsigned int, Connection>::accessor ac_1;
      tbb::concurrent_hash_map<unsigned int, tbb::concurrent_unordered_map<unsigned int, Connection>>::accessor ac_2;

      // Add Call object into correct location in Session
      if (this->conns.find(ac_2, this_tid)) {
        // PID and TID pair does not exist, so create both and add to conns
        Connection c;
        c.pid = pid;
        c.tid = tid;
        c.syscall_list.push_back(this_call);

        tbb::concurrent_hash_map<unsigned int, Connection> pid_map;
        pid_map.insert(ac_1, this_pid);
        ac_1->second = c;

        this->conns.insert(ac_2, this_tid);
        ac_2->second = pid_map;

      } else if (this->conns.find(ac_2, this_tid) && !this->conns.find(this_tid)->find(this_pid)) {
        // TID exists but PID does not, so create PID
        Connection c;
        c.pid = pid;
        c.tid = tid;
        c.syscall_list.push_back(this_call);

        std::vector<Connection> vec;
        vec.push_back(c);

        auto pid_map = this->conns.at(this_tid);
        pid_map.emplace(this_pid, vec);
      } else {
        // TID and PID pair already exist, so add to it
        auto conn = &this->conns.at(this_tid).at(this_pid);
        if (conn->empty() ||
            conn->back().syscall_list.back().syscall_name.compare(
                "SyS_shutdown") == 0 ||
            conn->back().syscall_list.back().syscall_name.compare(
                "sock_destroy_inode") == 0) {
          this->mq->send(conn->back().toString().c_str(), conn->back().toString().length(), 0);
          std::cout << "msg send" << std::endl;
          // Last Connection ended, this is a new Connection, so create a new
          // one and add to syscall_list
          Connection c;
          c.pid = pid;
          c.tid = tid;
          c.syscall_list.push_back(this_call);
          conn->push_back(c);
        } else {
          // Connection still ongoing, so just add to last Connection in
          // syscall_list
          conn->back().syscall_list.push_back(this_call);
        }
      }

      // Link Connection to IP address and Port
      // TODO: only if IP is 10.10.1.*
      if (has_port) {
        Connection *this_conn = &this->conns.at(this_tid).at(this_pid).back();
        // Only link if connection does not have a port yet
        if (this_conn->port == 0) {
          this_conn->port = conn_port;
          this_conn->ip_addr = ip;
        }
      }
    }
  }
}

/*!
 * This function returns a string for each connection readable by the ML
 * module
 * @return A string that the consumer can parse
 */
std::string Connection::toString() {
  std::string ret;
  std::unordered_map<std::string, int> functions;
  // Add each syscall function into the functions object if it doesn't exist, or
  // add one to the count
  for (auto &it : this->syscall_list) {
    auto l = functions.find(it.syscall_name);
    if (l == functions.end())
      functions.emplace(it.syscall_name, 1);
    else
      l->second += 1;
  }

  const std::vector<std::string> vect = {
      "sock_poll",         "sock_write_iter",    "sockfd_lookup_light",
      "sock_alloc_inode",  "sock_alloc",         "sock_alloc_file",
      "move_addr_to_user", "SYSC_getsockname",   "SyS_getsockname",
      "SYSC_accept4",      "sock_destroy_inode", "sock_read_iter",
      "sock_recvmsg",      "sock_sendmsg",       "__sock_release",
      "SyS_accept4",       "SyS_shutdown",       "sock_close"};
  for (const auto &entry : vect) {
    if (functions.find(entry) != functions.end())
      ret += std::to_string(functions[entry]) + ",";
    else
      ret.append("0,");
  }
  return ret.substr(0, ret.size() - 1);
}
