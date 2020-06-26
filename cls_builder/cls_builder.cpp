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
  timestamp,
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
  //std::string sockfd_lookup_light ("sockfd_lookup_light");
  std::string sockfd_lookup_light("SYSC_accept4");
  std::string sockname("SYSC_getsockname");
  std::string seq_read_send("seq_read_send");
  std::regex csv_match("((?:[a-zA-Z0-9_]*))(,)(\\d+)(,)(\\d+)(,)(\\d+)(,)(.*?)(,)(.*?)");

  while (std::getline(*in, line)) {
    // Only match if it is a data line and not extra stuff
    std::cout << "Line: " << line << std::endl;
    if (std::regex_match(line, csv_match)) {

      //std::cout << "Line: " << line << std::endl;
      // Call to add to a connection
      unsigned int this_pid = -1, this_tid = -1;
      int conn_port = -1;
      long long this_time = 0;
      bool has_port = false;
      std::string ip, call;
      int ip_flag = 0;

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
        else if (count == timestamp)
          this_time = std::stoll(*i);
        else if (count == addr && *i != "-1") {
          ip_flag = 1;
          ip = *i;
          if (ip.find(':') != std::string::npos) {
            unsigned char buf[sizeof(struct in6_addr)];
            int s;
            char str[INET6_ADDRSTRLEN];
            s = inet_pton(AF_INET6, ip.c_str(), buf);
            if (s > 0) {
              if (inet_ntop(AF_INET6, buf, str, INET6_ADDRSTRLEN) != nullptr) {
                ip = str;
                ip = ip.substr(ip.find_last_of(':') + 1);
                std::cout << "IP ADDRESS: " << ip << std::endl;
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
      if (this->conns.find(this_pid) == this->conns.end()) {
        // PID and TID pair does not exist, so create both and add to conns
        Connection c;

        // Only increment call if it's useful
        if (useful_calls.find(call) != useful_calls.end()) {
          //if(strcmp("sockfd_lookup_light",call) == 0)
	  if(!(sockfd_lookup_light.compare(call)) || !(sockname.compare(call)))
          {
          std::cout << "LMN: " << line << std::endl;
          c.syscall_list_count.insert({call, 1});
          c.syscall_list_time.insert({call, 0});
	  c.syscall_list_count.insert({seq_read_send, 0});
          c.syscall_list_time.insert({seq_read_send, 0});
          c.last_call = call; 
	  c.prev = this_time;
	  c.first_timestamp = this_time / 1000000;
	  std::cout << "\nFIRST: "<<c.first_timestamp<<"\n";
	  }
	  if(ip_flag == 1)
          {		  
	  	c.ip_addr = ip;
		if(conn_port > 0)
                {
                  c.port = conn_port;
                }		  
	  }
        }

        // Create a TID entry...
        tbb::concurrent_unordered_map<unsigned int, Connection> temp;
        temp.insert({this_tid, c});

        // ... and put it into the PID entry
        conns.insert({this_pid, temp});
      } else {
        // PID does exist, check if TID does
        auto pid_map = &conns.at(this_pid);
        if (pid_map->find(this_tid) != pid_map->end()) {
          // TID exists too, add to existing connection
          // Only increment call if we deem it useful and not ending call
          if (useful_calls.find(call) != useful_calls.end()) {
            auto c = &pid_map->at(this_tid);

            // Set new count and time for this call
            if (c->syscall_list_count.find(call) == c->syscall_list_count.end()){
              //if(strcmp("sockfd_lookup_light",call) == 0)
	      if((!(sockfd_lookup_light.compare(call)) /*&& c->syscall_list_count.size() ==  0*/) || c->syscall_list_count.size() > 0 || (!(sockname.compare(call)) /*&& c->syscall_list_count.size() ==  0*/))
              {
	      if(!(sockfd_lookup_light.compare(call)) /*&& c->syscall_list_count.size() ==  0*/ || (!(sockname.compare(call)) /*&& c->syscall_list_count.size() ==  0*/))
	      {
	      c->syscall_list_count.clear();
              c->syscall_list_time.clear();
	      c->prev = 0;
	      c->ip_addr = "";
              c->port = 0;
	      c->prev = 0;
	      c->first_timestamp = -1;
	      c->last_call = "";
              c->syscall_list_count.insert({seq_read_send, 0});
              c->syscall_list_time.insert({seq_read_send, 0});

              }	      
              std::cout << "LMN1: " << line << std::endl;		      
              c->syscall_list_count.insert({call, 1});
              long long diff = (c->prev == 0) ? 0 : (this_time - c->prev);
              c->syscall_list_time.insert({call,diff});
	      c->prev = this_time;
	      c->first_timestamp = this_time / 1000000;
	      }
	      if(ip_flag == 1)
              {		      
	      c->ip_addr = ip;

                if(conn_port > 0)
                {
                  c->port = conn_port;
                }


	      }
              if(call.compare("sock_sendmsg") == 0)
	      {
		     if( c->last_call.compare("sock_read_iter") == 0)
	             {
                        c->syscall_list_count.insert({seq_read_send, 1000});
                        c->syscall_list_time.insert({seq_read_send, 1000});			     

	             }
                     else
	             {
                        c->syscall_list_count.insert({seq_read_send, -1000});
                        c->syscall_list_time.insert({seq_read_send, -1000});

                     }

               }

	      c->last_call = call;
	      
            } else {
	     
              if(!(sockfd_lookup_light.compare(call)) /*&& c->syscall_list_count.size() ==  0*/ || (!(sockname.compare(call)) /*&& c->syscall_list_count.size() ==  0*/))
              {
              c->syscall_list_count.clear();
              c->syscall_list_time.clear();
              c->syscall_list_count.insert({seq_read_send, 0});
              c->syscall_list_time.insert({seq_read_send, 0});
              c->prev = 0;
              c->ip_addr = "";
              c->port = 0;
              c->prev = 0;
	      c->first_timestamp = -1;
	      c->last_call = "";
              std::cout << "LMN1: " << line << std::endl;
              c->syscall_list_count.insert({call, 1});
              long long diff = (c->prev == 0) ? 0 : (this_time - c->prev);
              c->syscall_list_time.insert({call,diff});
	      c->last_call = call;
              c->prev = this_time;
               
              if(ip_flag == 1)
              { 
              c->ip_addr = ip;

                if(conn_port > 0)
                {
                  c->port = conn_port;
                }
              }
              } 
              else{
              c->syscall_list_count.at(call) += 1;
              long long diff = (c->prev == 0) ? 0 : (this_time - c->prev);
              c->syscall_list_time.at(call) += (diff);
	      c->prev = this_time;
	      if(ip_flag == 1){		      
	      c->ip_addr = ip;
                if(conn_port > 0)
                {
                  c->port = conn_port;
                }

              if(call.compare("sock_sendmsg") == 0)
              {
                     if( c->last_call.compare("sock_read_iter") == 0)
                     {  
                        c->syscall_list_count.at(seq_read_send) += 1000;
                        c->syscall_list_time.at(seq_read_send) += 1000;

                     }
                     else
                     { 
                        c->syscall_list_count.at(seq_read_send) -= 1000;
                        c->syscall_list_time.at(seq_read_send) -= 1000; 

                     }

               }



	      }
             }
            }
           if(c->syscall_list_count.find("SyS_shutdown") != c->syscall_list_count.end() || c->syscall_list_count.find("sock_destroy_inode") != c->syscall_list_count.end() || c->syscall_list_count.find("__sock_release")!= c->syscall_list_count.end() || c->syscall_list_count.find("sock_close")!= c->syscall_list_count.end() )
            {
              mq->send(c->toString().c_str(), c->toString().length(), 0); 
              c->syscall_list_count.clear();
              c->syscall_list_time.clear();
              c->port = -1;
              c->ip_addr = "";
              c->prev = 0;
              c->port = 0;
	      c->last_call = "";
            } 
          }
        } else {
          // TID doesn't exist, create connection and add to pid_map
          Connection c;

          // Only increment call if we deem it useful
          if (useful_calls.find(call) != useful_calls.end()) {
            if(!(sockfd_lookup_light.compare(call)) || !(sockname.compare(call)))
            {
	    std::cout << "LMN2: " << line << std::endl;
            c.syscall_list_count.insert({call, 1});
            c.syscall_list_time.insert({call, 0});
            c.syscall_list_count.insert({seq_read_send, 0});
            c.syscall_list_time.insert({seq_read_send, 0});

	    c.prev = this_time;
	    c.first_timestamp = this_time / 1000000;
	    std::cout << "\nFIRST: "<<c.first_timestamp<<"\n";
	    }
	    if(ip_flag == 1){
	    c.ip_addr = ip;
                if(conn_port > 0)
                {
                  c.port = conn_port;
                }

	    }
          }

          pid_map->insert({this_tid, c});
        }
      }

      // Link Connection to IP address and Port
      // TODO: only if IP is 10.10.1.*
      if (has_port) {
        // Find the connection in the big hash map
        if (this->conns.find(this_pid) != this->conns.end()) {
          auto tid_map = &this->conns.at(this_pid);
          if (tid_map->find(this_tid) != tid_map->end()) {
            auto c = &tid_map->at(this_tid);
            // Only link if connection does not have a port yet
            if (c->port == 0) {
              c->port = conn_port;
	      if(ip_flag == 1){
              c->ip_addr = ip;
	      }
            }
          }
        }
      }
    }
  }
}

/**
 * This function pushes all current connections through the msgqueue every second
 */
void Session::push() {
  // run forever
  while (true) {
    // loop through each connection
    for (auto conn_list : conns)
      for (auto conn : conn_list.second)
        mq->send(conn.second.toString().c_str(), conn.second.toString().length(), 0);

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
  std::string ret1, ret2;

  // Order the vects correctly
/*  const std::vector<std::string> vect = {
      "sock_write_iter", "sockfd_lookup_light",
      "sock_alloc_inode", "sock_alloc", "sock_alloc_file",
      "move_addr_to_user", "SYSC_getsockname", "SyS_getsockname",
      "SYSC_accept4", "sock_destroy_inode", "sock_read_iter",
      "sock_recvmsg", "__sock_release",
      "SyS_accept4", "SyS_shutdown", "sock_close"
  }; */


  const std::vector<std::string> vect = {"sock_write_iter", "sock_read_iter", "sock_recvmsg","seq_read_send"};


  for (const auto &entry : vect) {
    // Add all freqs
    if (syscall_list_count.find(entry) != syscall_list_count.end())
      ret1 += std::to_string(syscall_list_count.at(entry)) + ",";
    else
      ret1.append("0,");

    // Add all times
    if (syscall_list_time.find(entry) != syscall_list_time.end())
      ret2 += std::to_string(syscall_list_time.at(entry)) + ",";
    else
      ret2.append("0,");
  }

  ret1.append("1|");
  ret1.append(ip_addr);
  ret1.append(":");
  ret1.append(std::to_string(port));
  ret1.append("=");
  ret1.append(std::to_string(first_timestamp));
  ret1.append("$\0");

  //ret2.back() = '|';
  //ret2.push_back('\0');
  return ret2 + ret1;
}

