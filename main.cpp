#include <iostream>
#include <string>
#include "producer/leader_session.h"

/**
 * Main method. The connections are stored in sess, and all data can be accessed
 * through its helper methods.
 */
int main() {
  // Stores all the connections
  Session sess;
  // Loop forever, getting new lines and putting them into the right connection
  sess.scan(&std::cin);
  freopen("/home/harry/Documents/isi/LEADER/producer/a.csv","r",stdin);
  std::cout << sess.get_size();
}

