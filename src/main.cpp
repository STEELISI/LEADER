#include "cls_builder/cls_builder.h"
#include "scoring/scorer.h"
#include <fstream>
#include <iostream>
#include <signal.h>
#include <string>
#include <unordered_set>

std::string blacklistpipe("/tmp/blacklistpipe");
std::unordered_set<unsigned int> blacklist;
std::ofstream piper;

void handle_sigint(int s) {
  std::cout << "Caught signal: " << s << std::endl;
  std::cout << "Exiting!" << std::endl;
  exit(1);
}

void blacklistIP(unsigned int userIP) {
  bool ok = blacklist.insert(userIP).second;
  std::cout << std::endl << " Blacklist " << ok;
  struct timeval t;
  gettimeofday(&t, NULL);
  if (ok) {
    std::cout << "Put into pipe " << userIP << " time " << t.tv_sec
              << std::endl;
    std::string l = std::to_string(userIP);
    std::cout << std::endl << l << std::endl;
    piper << l + "\n" << std::flush;
  }
}

/**
 * Main method. The connections are stored in sess, and all data can be accessed
 * through its helper methods.
 */
int main(int argc, char *argv[]) {
  if (argc != 2) {
    std::cout << "Usage: leader <model>" << std::endl;
    return 0;
  }

  // Create ML model from argument
  Model model(argv[1]);

  // Set pipe
  try {
    piper.open(blacklistpipe);
  } catch (std::exception const &e) {
    std::cout << "PIPE not set. Set it in " << blacklistpipe << ".\n";
    exit(1);
  }

  // Set up session and message queues
  Session sess;
  boost::interprocess::message_queue mq_data(boost::interprocess::open_only,
                                             "conns");

  // Set up handler to capture Ctrl-C signals
  struct sigaction sigint_handler;
  sigint_handler.sa_handler = handle_sigint;
  sigemptyset(&sigint_handler.sa_mask);
  sigint_handler.sa_flags = 0;

  // Read from message queue
  while (true) {
    char conn[4096];

    unsigned int priority;
    boost::interprocess::message_queue::size_type recvd_size;

    mq_data.receive(conn, 4096, recvd_size, priority);

    std::cout << "recieved message" << std::endl;

    // If message is readable analyze and output result
    int conn_flag = 0;
    int ip_flag = 0;
    int port_flag = 0;
    if (conn[0] != 0) {
      unsigned int ip_addr;
      int return_val;
      char conn_ex[4096];
      char ip[16];
      char port[6];
      char start_time[11];
      int j = 0;
      int k = 0;
      int l = 0;
      for (int i = 0; i < 4096; i++) {
        if (conn[i] != '|' && conn_flag == 0)
          conn_ex[i] = conn[i];
        else {
          if (conn[i] == '|') {
            conn_ex[i] = '\0';
            conn_flag = 1;
          } else if (conn[i] != ':' && ip_flag == 0) {
            ip[j] = conn[i];
            j++;
          } else {
            if (conn[i] == ':') {
              ip_flag = 1;
              ip[j] = '\0';
            } else if (conn[i] != '=' && port_flag == 0) {
              port[k] = conn[i];
              k++;
            } else {
              if (conn[i] == '=') {
                port[k] = '\0';
                port_flag = 1;
              } else if (conn[i] != '$') {
                start_time[l] = conn[i];
                l++;
              } else {
                start_time[l] = '\0';
                break;
              }
            }
          }
        }
      }
      std::string empty("0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,"
                        "0,0,0,0,0,0,0,0,0,1\0");
      std::string connectionstr(conn_ex);
      if (empty.compare(conn_ex)) {
        return_val = model.analyze_conn(conn_ex);
        std::cout << "Connection: " << conn_ex << " " << return_val << " IP "
                  << ip << ":" << port << " = " << start_time << std::endl;
        if (return_val == -1) {
          char *c = &ip[0];
          int st = atoi(start_time);
          if (inet_pton(AF_INET, c, &ip_addr) != 0) {
            std::cout << "\nBlacklisting IP " << ip << " " << ip_addr
                      << std::endl;
            blacklistIP(ip_addr);
            struct timeval t;
            gettimeofday(&t, NULL);
            int dur = t.tv_sec - st;
            std::cout << "\nBlacklisted IP " << ip << " " << ip_addr << " in "
                      << dur << " seconds " << std::endl;
          }
        }
      }
    } else
      std::cout << "nullptr conn" << std::endl;
  }

  // Should never be called
  return 0;
}

