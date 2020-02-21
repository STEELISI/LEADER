#include "cls_builder.h"
#include <boost/tokenizer.hpp>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <chrono>
#include <exception>
#include <iostream>
#include <regex>

// enum for the CSV inputs made by SystemTap
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

  // Set up useful_calls with syscalls we wanna record
  useful_calls.insert("sock_poll");
  useful_calls.insert("sock_write_iter");
  useful_calls.insert("sockfd_lookup_light");
  useful_calls.insert("sock_alloc_inode");
  useful_calls.insert("sock_alloc");
  useful_calls.insert("sock_alloc_file");
  useful_calls.insert("move_addr_to_user");
  useful_calls.insert("SYSC_getsockname");
  useful_calls.insert("SyS_getsockname");
  useful_calls.insert("SYSC_accept4");
  useful_calls.insert("sock_destroy_inode");
  useful_calls.insert("sock_read_iter");
  useful_calls.insert("sock_recvmsg");
  useful_calls.insert("sock_sendmsg");
  useful_calls.insert("__sock_release");
  useful_calls.insert("SyS_accept4");
  useful_calls.insert("SyS_shutdown");
  useful_calls.insert("sock_close");

  // Start stap process
  stap_process = boost::process::child(
      "/usr/local/bin/stap",
      boost::process::args({"-g", "./cls_builder/conn.stp",
                            "--suppress-handler-errors", "-DMAXMAPENTRIES=8096",
                            "-s4095", "-DINTERRUPTIBLE=0", "-DMAXTRYLOCK=10000",
                            "-DSTP_OVERLOAD_THRESHOLD=50000000000",
                            "--suppress-time-limits"}),
      boost::process::std_out > stap_out);

  // Start scanner and message pusher
  this->t_scanner = std::thread(&Session::scan, this, &stap_out);
  this->msg_push = std::thread(&Session::push, this);
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
    // Only match if it is a data line and not extra stuff
    if (std::regex_match(line, csv_match)) {

      // Call to add to a connection
      unsigned int this_pid = -1, conn_port = -1, this_tid = -1, this_time = 0;
      bool has_port = false;
      std::string ip, call;

      // Turn the CSV into a boost::tokenizer for easy parsing
      unsigned int count = 0;
      boost::tokenizer<boost::escaped_list_separator<char>> tk(
          line, boost::escaped_list_separator<char>('\\', ',', '\"'));

      // Turn the line info into actual data
      for (boost::tokenizer<boost::escaped_list_separator<char>>::iterator i(tk.begin());
           i != tk.end(); ++i) {
        if (count == func)
          call = *i;
        else if (count == tid)
          this_tid = std::stoi(*i);
        else if (count == pid)
          this_pid = std::stoi(*i);
        else if (count == call_time)
          this_time = std::stoi(*i);
        else if (count == addr && *i != "-1") {
          ip = *i;
          if(ip.find(':') != std::string::npos) {
            unsigned char buf[sizeof(struct in6_addr)];
            int s;
            char str[INET6_ADDRSTRLEN];
            s = inet_pton(AF_INET6, ip.c_str(), buf);
            if(s > 0)
            {
                if (inet_ntop(AF_INET6, buf, str, INET6_ADDRSTRLEN) != nullptr) {
                   ip = str;
                   ip = ip.substr(ip.find_last_of(':') + 1);
                }
             }

          }
          has_port = true;
        } else if (count == port && *i != "-1") {
          conn_port = std::stoi(*i);
          has_port = true;
        }
        count++;
      }

      // Add Call object into correct location in Session
      tbb::concurrent_hash_map<std::string, unsigned int>::accessor a;
      tbb::concurrent_hash_map<unsigned int,
                               tbb::concurrent_hash_map<unsigned int, Connection>>::accessor conns_ac;
      if (!this->conns.find(conns_ac, this_pid)) {
        conns_ac.release();

        // PID and TID pair does not exist, so create both and add to conns
        Connection c;

        // Only increment call if we deem it useful
        if (useful_calls.find(call) != useful_calls.end()) {
          c.syscall_list_count.insert(a, call);
          a->second += 1;
          a.release();

          c.syscall_list_time.insert(a, call);
          a->second += this_time;
          a.release();
        }

        // Create tid entry
        tbb::concurrent_hash_map<unsigned int, Connection> temp;
        tbb::concurrent_hash_map<unsigned int, Connection>::accessor temp_ac;
        temp.insert(temp_ac, this_tid);
        temp_ac->second = c;
        temp_ac.release();

        // Put into PID entry
        conns.insert(conns_ac, this_pid);
        conns_ac->second = temp;
      } else {
        tbb::concurrent_hash_map<unsigned int, Connection>::accessor temp_ac;
        // PID does exist, check if TID does
        auto pid_map = conns_ac->second;
        if(pid_map.find(temp_ac, this_tid)) {
          // TID exists too, add to existing connection
          // Only increment call if we deem it useful and not ending call
          if (useful_calls.find(call) != useful_calls.end()) {
            temp_ac->second.syscall_list_count.insert(a, call);
            a->second += 1;
            a.release();

            temp_ac->second.syscall_list_time.insert(a, call);
            a->second += this_time;
            a.release();
          }
        } else {
          temp_ac.release();
          // TID doesn't exist, create connection and add to pid_map
          Connection c;

          // Only increment call if we deem it useful
          if (useful_calls.find(call) != useful_calls.end()) {
            c.syscall_list_count.insert(a, call);
            a->second += 1;
            a.release();

            c.syscall_list_time.insert(a, call);
            a->second += this_time;
            a.release();
          }

          pid_map.insert(temp_ac, this_tid);
          temp_ac->second = c;
        }
        temp_ac.release();
      }
      conns_ac.release();

      // Link Connection to IP address and Port
      // TODO: only if IP is 10.10.1.*
      if (has_port) {
        // Find the connection in the big hash map
        tbb::concurrent_hash_map<unsigned int, Connection>::accessor temp_ac;
        if(this->conns.find(conns_ac, this_pid)) {
          if(conns_ac->second.find(temp_ac, this_tid)) {
            // Only link if connection does not have a port yet
            if (temp_ac->second.port == 0) {
              temp_ac->second.port = conn_port;
              temp_ac->second.ip_addr = ip;
            }
          }
          temp_ac.release();
        }
        conns_ac.release();
      }
    }
  }
}

/**
 * This function pushes all current connections through the msgqueue every second
 */
void Session::push() {
  // run forever
  while(true) {
    // loop through each connection
    for(auto iter = this->conns.begin(); iter != this->conns.end(); ++iter)
      for(auto it = iter->second.begin(); it != iter->second.end(); ++it)
        mq->send(it->second.toString().c_str(), it->second.toString().length(), 0);

    // sleep one second after computation
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
}

/*!
 * This function returns a string for each connection readable by the ML
 * module
 * @return A string that the consumer can parse
 */
std::string Connection::toString() {
  std::string ret;

  // Order the vects correctly
  const std::vector<std::string> vect = {
      "sock_poll",         "sock_write_iter",    "sockfd_lookup_light",
      "sock_alloc_inode",  "sock_alloc",         "sock_alloc_file",
      "move_addr_to_user", "SYSC_getsockname",   "SyS_getsockname",
      "SYSC_accept4",      "sock_destroy_inode", "sock_read_iter",
      "sock_recvmsg",      "sock_sendmsg",       "__sock_release",
      "SyS_accept4",       "SyS_shutdown",       "sock_close"
  };

  for (const auto &entry : vect) {
    tbb::concurrent_hash_map<std::string, unsigned int>::accessor ac;

    // Add all freqs
    if (syscall_list_count.find(ac, entry))
      ret += std::to_string(ac->second) + ",";
    else
      ret.append("0,");
    ac.release();

    // Add all times
    if (syscall_list_time.find(ac, entry))
      ret += std::to_string(ac->second) + ",";
    else
      ret.append("0,");
    ac.release();
  }

  // Set the last chars to "|\0"
  ret.back() = '|';
  ret.push_back('\0');
  return ret;
}
