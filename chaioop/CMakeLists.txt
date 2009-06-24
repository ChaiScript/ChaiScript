cmake_minimum_required(VERSION 2.6)

project(chaioop)

SET (CMAKE_BUILD_TYPE gdb)
SET (CMAKE_C_FLAGS_GDB " -Wall -ggdb -O3")
SET (CMAKE_CXX_FLAGS_GDB " -Wall -ggdb -O3")

include_directories(../langkit ../dispatchkit)

add_executable(chaioop_test main.cpp)
