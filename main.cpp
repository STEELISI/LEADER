#include <iostream>
#include <string>
#include "producer/leader_session.h"
#include "consumer/leader_consumer.h"

/**
 * Main method. The connections are stored in sess, and all data can be accessed
 * through its helper methods.
 */
int main() {
  // Stores all the connections
  Model model("/home/harry/Documents/isi/leader/consumer/elliptic_envelope.mlmodel");
  std::cout << model.analyze_conn("1987,9,51,13,22,0,10043209,7,7,16,14,32,850,459,17,8,59,5,3,1,3,1,2,0,2,1,1,1,1,4,4,2,1,1,1,1,1");
  return 0;
}

