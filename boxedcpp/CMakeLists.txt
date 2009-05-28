cmake_minimum_required(VERSION 2.6)

project(boxedcpp)

SET (CMAKE_BUILD_TYPE gdb)
SET (CMAKE_C_FLAGS_GDB " -Wall -ggdb")
SET (CMAKE_CXX_FLAGS_GDB " -Wall -ggdb")

find_package( Boost 1.36.0 COMPONENTS regex)
if(Boost_FOUND)
   include_directories(${Boost_INCLUDE_DIRS})
	 include_directories(.)
   
	 add_executable(boxedcpp_test test.cpp)
endif()
