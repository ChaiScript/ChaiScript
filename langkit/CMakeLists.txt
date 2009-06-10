cmake_minimum_required(VERSION 2.6)
enable_testing()

project(langkit)

SET (CMAKE_BUILD_TYPE gdb)
SET (CMAKE_C_FLAGS_GDB " -Wall -ggdb")
SET (CMAKE_CXX_FLAGS_GDB " -Wall -ggdb")

find_package( Boost 1.36.0 COMPONENTS regex unit_test_framework)
if(Boost_FOUND)
   include_directories(${Boost_INCLUDE_DIRS})
   
   add_executable(langkit_test main.cpp langkit_parser.cpp)
   target_link_libraries(langkit_test ${Boost_LIBRARIES})

   add_library(langkit SHARED langkit_parser.cpp)
   target_link_libraries(langkit ${Boost_LIBRARIES})

   add_executable(langkit_unittest unittest.cpp)
   target_link_libraries(langkit_unittest ${Boost_LIBRARIES})
endif()

add_test(langkit_unittest ${EXECUTABLE_OUTPUT_PATH}/langkit_unittest)
