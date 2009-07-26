cmake_minimum_required(VERSION 2.6)

project(chaiscript)

SET (CMAKE_BUILD_TYPE gdb)
SET (CMAKE_C_FLAGS_GDB " -Wall -ggdb")
SET (CMAKE_CXX_FLAGS_GDB " -Wall -ggdb")

#SET (CMAKE_BUILD_TYPE rel)
#SET (CMAKE_C_FLAGS_REL " -Wall -O3")
#SET (CMAKE_CXX_FLAGS_REL " -Wall -O3")

include_directories(include)
 
find_package(Boost 1.36.0)
if (Boost_FOUND)
	include_directories(${Boost_INCLUDE_DIRS})
	add_executable(chaiscript_eval src/main.cpp)
	#add_executable(dispatchkit_test contrib/test/dispatchkit_test.cpp)
	target_link_libraries(chaiscript_eval ${Boost_LIBRARIES})
	install(TARGETS chaiscript_eval DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/bin)
else(Boost_FOUND)
        message(FATAL_ERROR "Can not find Boost")
endif(Boost_FOUND)

