cmake_minimum_required(VERSION 2.6)
enable_testing()

project(wesley)

SET (CMAKE_BUILD_TYPE gdb)
SET (CMAKE_C_FLAGS_GDB " -Wall -ggdb")
SET (CMAKE_CXX_FLAGS_GDB " -Wall -ggdb")

find_package(Boost 1.36.0 COMPONENTS regex unit_test_framework)
if(Boost_FOUND)
   include_directories(${Boost_INCLUDE_DIRS})
   include_directories(../langkit ../dispatchkit)
   
   add_executable(chaiscript_test main.cpp)
   target_link_libraries(chaiscript_test ${Boost_LIBRARIES})

   add_executable(chaiscript_callbacktest callbacktest.cpp)
   target_link_libraries(chaiscript_callbacktest ${Boost_LIBRARIES})

endif()


