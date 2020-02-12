#include "cls_builder.h"
#include <boost/tokenizer.hpp>
#include <exception>
#include <iostream>
#include <regex>

// enum for the CSV inputs made by Systemtap
enum CSV {
  func,
  req,
  mem,
  fault,
  file,
  cycles,
  timestamp,
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
        else if (count == timestamp)
          this_call.timestamp = std::stoll(*i);
        else if (count == tid)
          this_tid = std::stoi(*i);
        else if (count == pid)
          this_pid = std::stoi(*i);
        else if (count == call_time)
          this_call.call_time = std::stoi(*i);
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
        Connection c;
        c.pid = pid;
        c.tid = tid;
        c.syscall_list.push_back(this_call);

        std::vector<Connection> vec;
        vec.push_back(c);

        std::unordered_map<int, std::vector<Connection>> pid_map;
        pid_map.emplace(this_pid, vec);

        this->conns.emplace(this_tid, pid_map);
      } else if (this->conns.find(this_tid) != this->conns.end() &&
                 this->conns.at(this_tid).find(this_pid) ==
                     this->conns.at(this_tid).end()) {
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

        // Add port to connections map
        if (this->connections.find(ip) == this->connections.end()) {
          // Connections has neither IP and port, add IP and port to list
          std::vector<Connection *> vec;
          vec.push_back(this_conn);

          std::unordered_map<int, std::vector<Connection *>> map;
          map.emplace(conn_port, vec);

          this->connections.emplace(ip, map);
        } else if (this->connections.find(ip) != this->connections.end() &&
                   this->connections.at(ip).find(conn_port) ==
                       this->connections.at(ip).end()) {
          // Connections has IP but not port, and port to list
          std::vector<Connection *> vec;
          vec.push_back(this_conn);

          this->connections.at(ip).emplace(port, vec);
        } else {
          // Connections already has both IP and port, just append
          this->connections.at(ip).at(conn_port).push_back(this_conn);
        }
      }
    }
  }
}

/**
 * This function returns the total number of connections in progress or
 * unreported in the session.
 */
int Session::get_size() {
  int ret = 0;
  for (const auto &i : this->conns)
    for (auto it : i.second)
      ret++;
  return ret;
}

/**
 * This function removes a connection given a pointer for it
 * @param c The connection to remove
 */
void Session::remove_conn(Connection *c) {}

/**
 * This function gets the ports of all connections for a given IP in a session.
 * @param ip The IP address to look up
 * @return An empty vector, or a vector of ports used
 */
std::vector<int> Session::get_connection_ports(const std::string &ip) {
  std::vector<int> ret;
  // Check if client exists in connection list, and then get the list of ports
  // for it
  if (connections.find(ip) != connections.end()) {
    for (std::pair<int, std::vector<Connection *>> element :
         connections.at(ip)) {
      ret.push_back(element.first);
    }
  }
  return ret;
};

/*!
 * This function returns a vector of connections given an IP and port number.
 * @param ip The IP address to look up
 * @param port The port to look up
 * @return A vector of Connections with the given port and IP, or nullptr
 */
std::vector<Connection> Session::get_connection(const std::string &ip,
                                                int port) {
  std::vector<Connection> ret;
  // Check if the connection vector exists and add to ret, then remove
  if (connections.find(ip) != connections.end() &&
      connections.at(ip).find(port) != connections.at(ip).end()) {
    for (auto i : connections.at(ip).at(port)) {
      ret.push_back(*i);

      // remove connection from conns vector
      std::vector<Connection> *conn_vec = &conns.at(i->tid).at(i->pid);
      for (auto it = conn_vec->begin(); it != conn_vec->end(); it++)
        if (i == &*it)
          conn_vec->erase(it);

      // remove connection from connections vector
      for (auto it = connections.at(ip).at(port).begin();
           it != connections.at(ip).at(port).end(); it++)
        if (&i == &*it)
          connections.at(ip).at(port).erase(it);
    }
  }
  return ret;
};

/*!
 * This function returns a string for each connection readable by the ML
 * module
 * @return A string that the consumer can parse
 */
std::string Connection::toString() {
  std::string ret_1, ret_2;
  std::unordered_map<std::string, int> functions;
  std::unordered_map<std::string, int> times;
  // Add each syscall function into the functions object if it doesn't exist, or
  // add one to the count
  int counter = 0;
  long long difference = 0;
  long long prev_timestamp = 0;
  for (auto &it : this->syscall_list) {
    
    std::cout << "Connection: " << it.syscall_name <<" "<<it.timestamp << std::endl;
    auto l = functions.find(it.syscall_name);
    if (l == functions.end())
      functions.emplace(it.syscall_name, 1);
    else
      l->second += 1;

    auto l_2 = times.find(it.syscall_name);
    if( counter == 0)
    {	    
    //if (l_2 == times.end())
      times.emplace(it.syscall_name, it.call_time);
   // else
   //   l_2->second += it.call_time;
    counter = 1;
    prev_timestamp = it.timestamp;
    }
    else
    {
      difference =  it.timestamp - prev_timestamp;
      if (l_2 == times.end())
        times.emplace(it.syscall_name, difference);
      else
        l_2->second += difference;
      prev_timestamp = it.timestamp;
    }	    
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
      ret_1 += std::to_string(functions[entry]) + ",";
    else
      ret_1.append("0,");

    if (functions.find(entry) != functions.end())
      ret_2 += std::to_string(times[entry]) + ",";
    else
      ret_2.append("0,");
  }
  std::cout << "Durations : "<<ret_2<<std::endl;
  std::cout << "Frequencies : "<<ret_1<<std::endl;

  ret_2 += ret_1;
  ret_2 = ret_2.substr(0, ret_2.size() - 1);
  ret_2 += "\0";
  std::cout << "\n\n\n Concatenated : "<<ret_2<<std::endl;
  return ret_2;
  //return ret_2.substr(0, ret_2.size() - 1);
}
