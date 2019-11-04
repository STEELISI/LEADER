#ifndef LEADER_CONSUMER_H
#define LEADER_CONSUMER_H
#include <string>

/**
 * Checks if a connection is an attack or not
 * @param in CSV string from leader_session
 * @return chance of attack from 0 to 1
 */
float analyze_conn(const std::string& in);

#endif //LEADER_CONSUMER_H
