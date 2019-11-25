cmake_minimum_required(VERSION 3.12)
project(leader)

find_package(Boost 1.67 REQUIRED system thread)
find_package(PythonLibs REQUIRED)
find_package(Threads REQUIRED)

include_directories(/usr/lib/ ${Boost_INCLUDE_DIR} ${PYTHON_INCLUDE_DIRS})

set(CMAKE_CXX_STANDARD 14)

add_library(leader_session SHARED cls_builder/cls_builder.cpp cls_builder/cls_builder.h)
target_link_libraries(leader_session LINK_PUBLIC ${PYTHON_LIBRARIES} LINK_PUBLIC ${Boost_LIBRARIES})

add_library(leader_consumer SHARED scoring/scorer.cpp scoring/scorer.h)

add_executable(leader main.cpp)
target_link_libraries(leader LINK_PUBLIC rt LINK_PUBLIC leader_session LINK_PUBLIC leader_consumer ${CMAKE_THREAD_LIBS_INIT})
