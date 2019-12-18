#include "cls_builder/cls_builder.h"
#include "scoring/scorer.h"
#include <iostream>

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

  // Set up session and message queue
  Session sess;
  // boost::interprocess::message_queue mq(boost::interprocess::open_only,
  //                                     "conns");

  // Read from message queue
  while (true) {
    /*
    char conn[4096];

    unsigned int priority;
    boost::interprocess::message_queue::size_type recvd_size;

    mq.receive(conn, 4096, recvd_size, priority);

    std::cout << "recieved message" << std::endl;

    // If message is readable analyze and output result
    if (conn[0] != 0) {
      std::cout << conn << std::endl;
      std::cout << model.analyze_conn(conn) << std::endl;
    } else
      std::cout << "nullptr conn" << std::endl;
    */
  }
  return 0;
}

