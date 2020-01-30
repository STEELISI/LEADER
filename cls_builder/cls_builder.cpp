#include "cls_builder.h"
#include <boost/tokenizer.hpp>
#include <exception>
#include <iostream>
#include <regex>
#include <utility>

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
      unsigned int this_tid = -1, this_pid = -1, conn_port = -1;
      bool has_port = false;
      std::string ip, call;

      // Turn the CSV into a boost::tokenizer for easy parsing
      unsigned int count = 0;
      boost::tokenizer<boost::escaped_list_separator<char>> tk(
          line, boost::escaped_list_separator<char>('\\', ',', '\"'));

      // Add each element of the line into a Call object
      for (boost::tokenizer<boost::escaped_list_separator<char>>::iterator i(tk.begin()); i != tk.end(); ++i) {
        if (count == func)
          call = *i;
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

      // Add Call object into correct location in Session
      if (this->conns.find(this_tid) == this->conns.end()) {
        // PID and TID pair does not exist, so create both and add to conns

      } else if (this->conns.find(this_tid)->second.find(this_pid) != this->conns.find(this_tid)->second.end()) {
        // TID exists but PID does not, so create PID

      } else {
        // TID and PID pair already exist, so add this syscall to it
        this->conns[this_tid][this_pid].syscall_list.push_back(call);
      }

      // Link Connection to IP address and Port
      // TODO: only if IP is 10.10.1.*
      if (has_port) {
        Connection *this_conn = &this->conns.at(this_tid).at(this_pid);
        // Only link if connection does not have a port yet
        if (this_conn->port == 0) {
          this_conn->port = conn_port;
          this_conn->ip_addr = ip;
        }
      }
    }
  }
}

/**
 * This function runs in a separate thread and checks each connection every second
 */
void Session::analyze() {

}

/**
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
    auto l = functions.find(it);
    if (l == functions.end())
      functions.emplace(it, 1);
    else
      l->second += 1;
  }

  // For each entry we care about, add into the string CSV
  for (const auto &entry : this->vect) {
    if (functions.find(entry) != functions.end())
      ret += std::to_string(functions.find(entry)->second) + ",";
    else
      ret.append("0,");
  }
  return ret.substr(0, ret.size() - 1);
}

