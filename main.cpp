#include <iostream>
#include <string>
#include "cls_builder/cls_builder.h"
#include "scoring/scorer.h"

/**
 * Main method. The connections are stored in sess, and all data can be accessed
 * through its helper methods.
 */
int main(int argc, char *argv[]) {
  if(argc != 2) {
    std::cout << "Usage: leader <model>" << std::endl;
    return 0;
  }
  Model model(argv[1]);
  Session sess;
  //boost::interprocess::message_queue mq(boost::interprocess::open_only, "conns");
  // Stores all the connections
  std::cout << model.analyze_conn("1987,9,51,13,22,0,10043209,7,7,16,14,32,850,459,17,8,59,5,3,1,3,1,2,0,2,1,1,1,1,4,4,2,1,1,1,1,1");
  return 0;
}

