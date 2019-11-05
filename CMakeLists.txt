cmake_minimum_required(VERSION 3.15)
project(leader)

find_package(Boost 1.69 REQUIRED)

include_directories(/usr/lib/ ${Boost_INCLUDE_DIR})

set(CMAKE_CXX_STANDARD 17)

add_library(leader_session SHARED producer/leader_session.cpp producer/leader_session.h)
add_library(leader_consumer SHARED consumer/leader_consumer.cpp consumer/leader_consumer.h)

add_executable(leader main.cpp)
target_link_libraries(leader LINK_PUBLIC leader_session LINK_PUBLIC leader_consumer LINK_PUBLIC python3 LINK_PUBLIC ${Boost_LIBRARIES})
