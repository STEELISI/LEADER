cmake_minimum_required(VERSION 3.10)
project(leader)

find_package(Boost 1.65 REQUIRED system thread)
find_package(Python3 COMPONENTS Interpreter Development)
find_package(Threads REQUIRED)
find_package(TBB COMPONENTS tbbmalloc tbbmalloc_proxy tbb_preview)

include_directories(/usr/lib/ ${Boost_INCLUDE_DIR} ${Python3_INCLUDE_DIR})

set(CMAKE_CXX_STANDARD 14)

add_library(leader_session SHARED cls_builder/cls_builder.cpp cls_builder/cls_builder.h)
target_link_libraries(leader_session ${Python3_LIBRARIES} ${Boost_LIBRARIES} tbb)

add_library(leader_scorer SHARED scoring/scorer.cpp scoring/scorer.h)

add_library(leader_mitigation SHARED mitigation/mitigator.cpp mitigation/mitigator.h)
target_link_libraries(leader_mitigation ${Python3_LIBRARIES})

add_executable(leader main.cpp)
target_link_libraries(leader rt leader_session leader_scorer leader_mitigation ${CMAKE_THREAD_LIBS_INIT})
