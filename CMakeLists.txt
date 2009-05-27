cmake_minimum_required(VERSION 2.6)

project(langkit)

SET (CMAKE_BUILD_TYPE gdb)
SET (CMAKE_C_FLAGS_GDB " -Wall -ggdb")
SET (CMAKE_CXX_FLAGS_GDB " -Wall -ggdb")

find_package( Boost 1.36.0 COMPONENTS regex)
if(Boost_FOUND)
   include_directories(${Boost_INCLUDE_DIRS})
   include_directories(langkit boxedcpp)
   
   add_executable(langkit_test langkit/main.cpp langkit/langkit_lexer.cpp langkit/langkit_parser.cpp)
   target_link_libraries(langkit_test ${Boost_LIBRARIES})

   add_executable(boxedcpp_test boxedcpp/test.cpp)
endif()
