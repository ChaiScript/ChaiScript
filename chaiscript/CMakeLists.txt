cmake_minimum_required(VERSION 2.6)
enable_testing()

project(chaiscript)

SET (CMAKE_BUILD_TYPE gdb)
SET (CMAKE_C_FLAGS_GDB " -Wall -ggdb")
SET (CMAKE_CXX_FLAGS_GDB " -Wall -ggdb")

find_package(Boost 1.36.0 COMPONENTS)
if(Boost_FOUND)
   include_directories(${Boost_INCLUDE_DIRS})
   include_directories(../dispatchkit)
   
   add_executable(chaiscript_eval main.cpp)
   target_link_libraries(chaiscript_eval ${Boost_LIBRARIES})

	 #add_executable(chaiscript_callbacktest callbacktest.cpp)
	 #target_link_libraries(chaiscript_callbacktest ${Boost_LIBRARIES})

	 #add_executable(sensors sensors.cpp)
	 #target_link_libraries(sensors ${Boost_LIBRARIES})


endif()


 