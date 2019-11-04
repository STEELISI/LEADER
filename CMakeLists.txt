cmake_minimum_required(VERSION 3.15)
project(leader)

set(CMAKE_CXX_STANDARD 14)

add_executable(leader main.cpp producer/leader_session.cpp producer/leader_session.h consumer/leader_consumer.cpp consumer/leader_consumer.h)