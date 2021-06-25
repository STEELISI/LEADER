// HOW TO RUN
// ./build/leader elliptic_envelope.mlmodel normalization.pkl standardization.pkl

#include "cls_builder/cls_builder.h"
#include "scoring/scorer.h"
#include <unordered_set>
#include <iostream>
#include <signal.h>
#include <string>
#include <fstream>

std::string blacklistpipe("/tmp/blacklistpipe");
std::unordered_set<unsigned int> blacklist;
std::ofstream piper;

// FOR DEBUG PURPOSES
std::string arr[5000];
int leg_times[5000];
int atk_times[5000];
std::string pt[5000];
int ip_count = -1;
//


void handle_sigint(int s) {
  std::cout << "Caught signal: " << s << std::endl;
  std::cout << "Exiting!" << std::endl;
  exit(1);
}

void blacklistIP(unsigned int userIP) {
  bool ok = blacklist.insert(userIP).second;
  std::cout<<std::endl<<" Blacklist "<<ok;
  struct timeval t;
  gettimeofday (&t, NULL);
  if (ok)
    {
      std::cout << "Put into pipe " << userIP << " time " << t.tv_sec <<std::endl;
      std::string l= std::to_string(userIP);
      std::cout<<std::endl<<l<<std::endl;
      piper << l + "\n" << std::flush;
    }
}


/**
 * Main method. The connections are stored in sess, and all data can be accessed
 * through its helper methods.
 */
int main(int argc, char *argv[]) {
  if (argc != 4) {
    std::cout << "Usage: leader <model>" << std::endl;
    return 0;
  }

  // Create ML model from argument
  /*const char* extension = ".txt";

  char* name_with_extension;
  name_with_extension = malloc(strlen(argv[1])+strlen(argv[2])+strlen(argv[3])+4);
  strcpy(name_with_extension, name);
  strcat(name_with_extension, extension);
 */ 

  Model model(argv[1],argv[2],argv[3]);

  // Set pipe
  try {
         piper.open(blacklistpipe);
  } catch(std::exception const& e){
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
    int ip_flag =0;
    int port_flag =0;
    if (conn[0] != 0) {
      unsigned int ip_addr;
      int return_val;
      char conn_ex[4096];
      char ip[16];
      char port[6];
      char start_time[11];
      int j=0;
      int k = 0;
      int l = 0;
      for(int i=0;i<4096;i++) {
	      if(conn[i] != '|' && conn_flag == 0)
	        conn_ex[i] = conn[i];
	      else {
		if(conn[i] == '|')      
		{ conn_ex[i] = '\0';
		  conn_flag = 1;
		}
		else if(conn[i] != ':' && ip_flag == 0 )
		{
                   ip[j] = conn[i];
		   j++;
		}
	        else
		{
                if(conn[i] == ':')
	        {
	          ip_flag = 1;
                  ip[j] = '\0';		  
                }
		else if(conn[i] != '=' && port_flag == 0)
		{
                  port[k] = conn[i];
		  k++;
		}
	        else
		{ 
		   if(conn[i] == '=')	
		   { port[k] = '\0';
	             port_flag = 1;		     
	           }
		   else if(conn[i] != '$')
	           {
		     start_time[l] = conn[i];
	             l++;
	           }
                   else
	           {		   
                     start_time[l] = '\0';
		      break;
		   }
		}
		}
	      }
      }
      //std::string  empty("0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1\0");
      //std::string  empty("0,0,0,0,0,0,1\0");
      std::string  empty("0,0,0,0,0,0,0,0,1\0");
      std::string  connectionstr(conn_ex);
      if(empty.compare(conn_ex))
      {	
          return_val = model.analyze_conn(conn_ex);	      
          std::cout <<"\n\n\nConnection: "<< conn_ex <<" CLASSIFIED: "<< return_val <<" IP "<<ip <<":"<<port<<" = " << start_time << std::endl<<std::endl;
         
         // FOR DEBUG PURPOSES
	 int exists = 0;
	 int fl = 0; 
         std::string cur_ip = std::string(ip);
	 std::string cur_port = std::string(port);
         int cur_ip_pos = -1;

	 for(int x=0; x<=ip_count; x++)
	 {
            if(!(cur_ip.compare(arr[x])))
	    {   fl = 2;
                cur_ip_pos = x;
            }		    
            if(!(cur_ip.compare(arr[x])) && (cur_port.compare(pt[x])))
	    {
                if(return_val == 1)  
                leg_times[x]++;
                else if(return_val == -1)
                atk_times[x]++;
                pt[x] = cur_port;		
		fl=1;
		struct timeval t;
		gettimeofday(&t, NULL);
		std::cout <<"\n\n\n "<<t.tv_sec<<" STATS: "<<arr[x]<<" "<<leg_times[x]<<" "<<atk_times[x];
		break;
            }
	    //if(fl == 2)
	    //   break;
	    struct timeval t;
	    gettimeofday(&t, NULL);
	    std::cout <<"\n\n\n "<<t.tv_sec<<" STATS: "<<arr[x]<<" "<<leg_times[x]<<" "<<atk_times[x];
	 }	 
         
	 if(fl == 0)
	 {
             ip_count++;
             arr[ip_count] = cur_ip;
             pt[ip_count] = cur_port;
                if(return_val == 1)
                {leg_times[ip_count] = 1;
                 atk_times[ip_count] = 0;
                }
                else if(return_val == -1)
                {
		 leg_times[ip_count] = 0;
                 atk_times[ip_count] = 1;
                }
	     struct timeval t;
	     gettimeofday(&t, NULL);
             std::cout <<"\n\n\n "<<t.tv_sec<<" STATS: "<<arr[ip_count]<<" "<<leg_times[ip_count]<<" "<<atk_times[ip_count];
         }		 
         // FOR DEBUG PURPOSES
          
	  if(cur_ip_pos >= 0)
          {		  
	  float ans = (float)atk_times[cur_ip_pos] / (float)(atk_times[cur_ip_pos] + leg_times[cur_ip_pos]);
	 // std::cout <<"Percentage" << ans <<std::endl;
	  if(ans > 0.1 && (atk_times[cur_ip_pos] + leg_times[cur_ip_pos]) >= 4)
          {
            char* c = &ip[0];
	    int st = atoi(start_time);
             if (inet_pton(AF_INET, c, &ip_addr) != 0) {
		     std::cout <<"\nBlacklisting IP "<<ip<<" "<<ip_addr<<std::endl;
                     blacklistIP(ip_addr);
		     struct timeval t;
		     gettimeofday (&t, NULL);
		     int dur = t.tv_sec - st;
		     std::cout <<"\nBlacklisted IP "<<ip<<" "<<ip_addr<<" in "<<dur<<" seconds "<<std::endl;
             }
          }
        } 	  
      }
    } else
      std::cout << "nullptr conn" << std::endl;
  }

  // Should never be called
  return 0;
}

