cmake_minimum_required(VERSION 2.6)

project(chaiscript)

FIND_LIBRARY(READLINE_LIBRARY NAMES readline PATH /usr/lib /usr/local/lib /opt/local/lib)

MESSAGE(STATUS "Detecting readline support")
if (READLINE_LIBRARY)
    MESSAGE(STATUS "Found: ${READLINE_LIBRARY}")

    SET (READLINE_LIB readline)
    SET (CMAKE_BUILD_TYPE gdb)
    SET (CMAKE_C_FLAGS_GDB " -Wall -ggdb -DREADLINE_AVAILABLE")
    SET (CMAKE_CXX_FLAGS_GDB " -Wall -ggdb -DREADLINE_AVAILABLE")

    #SET (CMAKE_BUILD_TYPE rel)
    #SET (CMAKE_C_FLAGS_REL " -Wall -O3 -DREADLINE_AVAILABLE")
    #SET (CMAKE_CXX_FLAGS_REL " -Wall -O3 -DREADLINE_AVAILABLE")
else(READLINE_LIBRARY)
    MESSAGE(STATUS "Not Found")

    SET (READLINE_LIB )
    SET (CMAKE_BUILD_TYPE gdb)
    SET (CMAKE_C_FLAGS_GDB " -Wall -ggdb")
    SET (CMAKE_CXX_FLAGS_GDB " -Wall -ggdb")

    #SET (CMAKE_BUILD_TYPE rel)
    #SET (CMAKE_C_FLAGS_REL " -Wall -O3")
    #SET (CMAKE_CXX_FLAGS_REL " -Wall -O3")
endif(READLINE_LIBRARY)

include_directories(include)
 
find_package(Boost 1.36.0 COMPONENTS thread)

if (Boost_FOUND)
	include_directories(${Boost_INCLUDE_DIRS})
	add_executable(chaiscript_eval src/main.cpp)
	#add_executable(dispatchkit_test contrib/test/dispatchkit_test.cpp)
	target_link_libraries(chaiscript_eval dl ${Boost_LIBRARIES} ${READLINE_LIB})

  add_library(test MODULE src/test_module.cpp)
	target_link_libraries(test ${Boost_LIBRARIES})

	install(TARGETS chaiscript_eval DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/bin)
        
else(Boost_FOUND)
        message(FATAL_ERROR "Can not find Boost")
endif(Boost_FOUND)

