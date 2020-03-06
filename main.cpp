#include "cls_builder/cls_builder.h"
#include "scoring/scorer.h"
#include <iostream>
#include <signal.h>


void handle_sigint(int s){
    std::cout << "Caught signal: " << s << std::endl;
    std::cout << "Exiting!" << std::endl;
    exit(1);
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
    if (conn[0] != 0) {
      char conn_ex[4096];
      char ip[16];
      int j=0;
      for(int i=0;i<4096;i++) {
	      if(conn[i] != '|' && conn_flag == 0)
	        conn_ex[i] = conn[i];
	      else {
		if(conn[i] == '|')      
		{ conn_ex[i] = '\0';
		  conn_flag = 1;
		}	
		else if(conn[i] != '$')
		{
                  ip[j] = conn[i];
		  j++;
		}
	        else
		{	
		     ip[j] = '\0';
		      break;
		}
	      }
      }
      std::string  empty("0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1\0");
      std::string  connectionstr(conn_ex);
      if(empty.compare(conn_ex))
      {	      
      std::cout <<"Connection: "<< conn_ex <<" "<< model.analyze_conn(conn_ex) <<" IP "<<ip << std::endl;
      }
    } else
      std::cout << "nullptr conn" << std::endl;
  }

  // Should never be called
  return 0;
}

