cmake_minimum_required(VERSION 2.6)

project(chaiscript)

FIND_LIBRARY(READLINE_LIBRARY NAMES readline PATH /usr/lib /usr/local/lib /opt/local/lib)

enable_testing()

MESSAGE(STATUS "Detecting readline support")
if (READLINE_LIBRARY)
    MESSAGE(STATUS "Found: ${READLINE_LIBRARY}")
    SET (READLINE_LIB readline)
    SET (READLINE_FLAG " -DREADLINE_AVAILABLE")
else(READLINE_LIBRARY)
    MESSAGE(STATUS "Not Found")
    SET (READLINE_LIB )
    SET (READLINE_FLAG )
endif(READLINE_LIBRARY)

SET (CMAKE_C_FLAGS_REL " -Wall -O3 ${READLINE_FLAG}")
SET (CMAKE_CXX_FLAGS_REL " -Wall -O3 ${READLINE_FLAG}")

SET (CMAKE_C_FLAGS " -Wall -ggdb ${READLINE_FLAG}")
SET (CMAKE_CXX_FLAGS " -Wall -ggdb ${READLINE_FLAG}")

include_directories(include)
 
find_package(Boost 1.36.0 COMPONENTS thread)

if (NOT Boost_FOUND)
  message(FATAL_ERROR "Can not find Boost")
endif(NOT Boost_FOUND)



include_directories(${Boost_INCLUDE_DIRS})
add_executable(chaiscript_eval src/main.cpp)
#add_executable(dispatchkit_test contrib/test/dispatchkit_test.cpp)
target_link_libraries(chaiscript_eval dl ${Boost_LIBRARIES} ${READLINE_LIB})

add_library(test MODULE src/test_module.cpp)
target_link_libraries(test ${Boost_LIBRARIES})

add_library(stl_extra MODULE src/stl_extra.cpp)
target_link_libraries(stl_extra ${Boost_LIBRARIES})

file(GLOB UNIT_TESTS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}/unittests/ ${CMAKE_CURRENT_SOURCE_DIR}/unittests/*.chai)



foreach(filename ${UNIT_TESTS})
  message(STATUS "Adding test ${filename}")
  add_test(${filename} chaiscript_eval ${CMAKE_CURRENT_SOURCE_DIR}/unittests/unit_test.inc ${CMAKE_CURRENT_SOURCE_DIR}/unittests/${filename})
endforeach(filename)


install(TARGETS chaiscript_eval DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/bin)
        

