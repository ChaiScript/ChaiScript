cmake_minimum_required(VERSION 2.8)

project(chaiscript)

list(APPEND CPACK_SOURCE_IGNORE_FILES "${CMAKE_CURRENT_BINARY_DIR}")
list(APPEND CPACK_SOURCE_IGNORE_FILES "\\\\.svn")
list(APPEND CPACK_SOURCE_IGNORE_FILES ".swp")
list(APPEND CPACK_SOURCE_IGNORE_FILES ".*~")

set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/license.txt")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/readme.txt")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/description.txt")

set(CPACK_PACKAGE_VERSION_MAJOR 2)
set(CPACK_PACKAGE_VERSION_MINOR 3)
set(CPACK_PACKAGE_VERSION_PATCH 3)
set(CPACK_PACKAGE_EXECUTABLES "chai;ChaiScript Eval")
set(CPACK_PACKAGE_VENDOR "ChaiScript.com")
set(CPACK_PACKAGE_CONTACT "contact@chaiscript.com")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "An embedded scripting language for C++")

set(CPACK_DEBIAN_PACKAGE_DEPENDS "libboost-dev (>=1.36.0)")
set(CPACK_DEBIAN_PACKAGE_SECTION "devel")
set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")

set(CPACK_RPM_PACKAGE_LICENSE "BSD")
set(CPACK_RPM_PACKAGE_GROUP "Programming")
set(CPACK_RPM_PACKAGE_REQUIRES "boost-devel >= 1.36.0, boost-thread >= 1.36.0")


include(CTest)
include(CPack)

FIND_LIBRARY(READLINE_LIBRARY NAMES readline PATH /usr/lib /usr/local/lib /opt/local/lib)

enable_testing()


MESSAGE(STATUS "Detecting readline support")
if (READLINE_LIBRARY)
    MESSAGE(STATUS "Found: ${READLINE_LIBRARY}")
    SET (READLINE_LIB readline)
    ADD_DEFINITIONS(/DREADLINE_AVAILABLE)
else(READLINE_LIBRARY)
    MESSAGE(STATUS "Not Found")
    SET (READLINE_LIB )
    SET (READLINE_FLAG )
endif(READLINE_LIBRARY)

#SET (CMAKE_C_FLAGS_REL " -Wall -O3 ${READLINE_FLAG}")
#SET (CMAKE_CXX_FLAGS_REL " -Wall -O3 ${READLINE_FLAG}")

#SET (CMAKE_C_FLAGS " -Wall -ggdb ${READLINE_FLAG}")
#SET (CMAKE_CXX_FLAGS " -Wall -ggdb ${READLINE_FLAG}")

include_directories(include)
 
SET(Boost_ADDITIONAL_VERSIONS "1.43" "1.43.0" "1.42" "1.42.0" "1.41")
SET(Boost_USE_MULTITHREADED ON)

find_package(Boost 1.36.0 COMPONENTS thread)

if (Boost_FOUND)
  link_directories( ${Boost_LIBRARY_DIRS} )
else()
  message(FATAL_ERROR "Can not find Boost")
endif(Boost_FOUND)

if (CMAKE_HOST_UNIX)
  SET(DYNAMIC_LOADER "dl")
endif(CMAKE_HOST_UNIX)


include_directories(${Boost_INCLUDE_DIRS})
add_executable(chai src/main.cpp)
#add_executable(dispatchkit_test contrib/test/dispatchkit_test.cpp)
target_link_libraries(chai ${DYNAMIC_LOADER} ${Boost_LIBRARIES} ${READLINE_LIB})

add_library(test_module MODULE src/test_module.cpp)
target_link_libraries(test_module ${Boost_LIBRARIES})

add_library(stl_extra MODULE src/stl_extra.cpp)
target_link_libraries(stl_extra ${Boost_LIBRARIES})

file(GLOB UNIT_TESTS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}/unittests/ ${CMAKE_CURRENT_SOURCE_DIR}/unittests/*.chai)

IF(BUILD_TESTING)

  foreach(filename ${UNIT_TESTS})
    message(STATUS "Adding test ${filename}")
    add_test(${filename} chai ${CMAKE_CURRENT_SOURCE_DIR}/unittests/unit_test.inc ${CMAKE_CURRENT_SOURCE_DIR}/unittests/${filename})
  endforeach(filename)

  SET_PROPERTY(TEST ${UNIT_TESTS}
   PROPERTY ENVIRONMENT
    "CHAI_USE_PATH=${CMAKE_CURRENT_SOURCE_DIR}/unittests/"
    "CHAI_MODULE_PATH=${CMAKE_CURRENT_BINARY_DIR}/"
  )
ENDIF(BUILD_TESTING)

install(TARGETS chai stl_extra test_module RUNTIME DESTINATION bin LIBRARY DESTINATION lib/chaiscript )
install(DIRECTORY include/chaiscript DESTINATION include 
   PATTERN "*.hpp"
   PATTERN "*/.svn*" EXCLUDE)
install(DIRECTORY unittests DESTINATION share/chaiscript 
   PATTERN "*.chai" 
   PATTERN "*.inc"
   PATTERN "*/.svn*" EXCLUDE)
install(DIRECTORY samples DESTINATION share/chaiscript 
   PATTERN "*.chai"
   PATTERN "*/.svn*" EXCLUDE)
        

