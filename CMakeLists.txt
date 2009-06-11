cmake_minimum_required(VERSION 2.6)

enable_testing()

project(jaytea)

SET (CMAKE_BUILD_TYPE gdb)
SET (CMAKE_C_FLAGS_GDB " -Wall -ggdb")
SET (CMAKE_CXX_FLAGS_GDB " -Wall -ggdb")

include_directories(dispatchkit langkit)

add_subdirectory(langkit bin)
add_subdirectory(dispatchkit bin)
add_subdirectory(wesley bin)
 
