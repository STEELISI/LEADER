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
  boost::interprocess::message_queue mq(boost::interprocess::open_only,
                                        "conns");

  // Read from message queue
  while (true) {
    Connection *conn;
    boost::interprocess::message_queue::size_type recvd_size;
    unsigned int priority;
    mq.receive(conn, sizeof(Connection *), recvd_size, priority);

    if (conn != nullptr) {
      std::cout << model.analyze_conn(conn->toString());
    }
  }
  return 0;
}

