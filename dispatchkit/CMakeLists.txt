cmake_minimum_required(VERSION 2.6)
enable_testing()

project(boxedcpp)

SET (CMAKE_BUILD_TYPE gdb)
SET (CMAKE_C_FLAGS_GDB " -Wall -ggdb")
SET (CMAKE_CXX_FLAGS_GDB " -Wall -ggdb")

find_package( Boost 1.36.0 COMPONENTS unit_test_framework)
if(Boost_FOUND)
   include_directories(${Boost_INCLUDE_DIRS})
   include_directories(.)
   
	 add_executable(dispatchkit_test test.cpp)

	 add_executable(dispatchkit_unittest unittest.cpp)

	 target_link_libraries(dispatchkit_unittest ${Boost_LIBRARIES})

endif()

add_test(dispatchkit_unittest dispatchkit_unittest)
