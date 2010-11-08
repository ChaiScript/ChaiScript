cmake_minimum_required(VERSION 2.8)

project(chaiscript)

option(MULTITHREAD_SUPPORT_ENABLED "Multithreaded Support Enabled" TRUE)
option(BUILD_SAMPLES "Build Samples Folder" FALSE)

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

IF(MSVC)
  ADD_DEFINITIONS(/W4)
  IF(CMAKE_CL_64)
    ADD_DEFINITIONS(/bigobj)
   ENDIF()
ELSE()
  # -Wno-missing-field-initializers is for boost on macos
  ADD_DEFINITIONS(-Wall -Wextra -Wno-missing-field-initializers)
ENDIF()

include_directories(include)
 
SET(Boost_ADDITIONAL_VERSIONS "1.44" "1.44.0" "1.43" "1.43.0" "1.42" "1.42.0" "1.41")
SET(Boost_USE_MULTITHREADED ON)

if (MULTITHREAD_SUPPORT_ENABLED)
  find_package(Boost 1.36.0 COMPONENTS thread)

  if (Boost_FOUND)
    link_directories( ${Boost_LIBRARY_DIRS} )
  else()
    message(FATAL_ERROR "Can not find Boost")
  endif(Boost_FOUND)
else()
  ADD_DEFINITIONS(-DCHAISCRIPT_NO_THREADS)
endif()

if (CMAKE_HOST_UNIX)
  SET(DYNAMIC_LOADER "dl")
endif(CMAKE_HOST_UNIX)

if (MSVC)
  # Boost on MSVC does automatic linking
  SET(LIBS ${DYNAMIC_LOADER} ${READLINE_LIB})
else()
  SET(LIBS ${DYNAMIC_LOADER} ${Boost_LIBRARIES} ${READLINE_LIB})
endif()

if (CMAKE_COMPILER_2005)
  # vs2005 is a bit too loud about possible loss of data warnings
#  ADD_DEFINITIONS(/wd4244)
endif()

include_directories(${Boost_INCLUDE_DIRS})
include_directories(${Boost_INCLUDE_DIR})
add_executable(chai src/main.cpp)
target_link_libraries(chai ${LIBS})

if (BUILD_SAMPLES)
  add_executable(example samples/example.cpp)
  target_link_libraries(example ${LIBS})
  add_executable(memory_leak_test samples/memory_leak_test.cpp)
  target_link_libraries(memory_leak_test ${LIBS})
endif()


add_library(stl_extra MODULE src/stl_extra.cpp)
target_link_libraries(stl_extra ${LIBS})

add_library(reflection MODULE src/reflection.cpp)
target_link_libraries(reflection ${LIBS})

file(GLOB UNIT_TESTS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}/unittests/ ${CMAKE_CURRENT_SOURCE_DIR}/unittests/*.chai)

list(SORT UNIT_TESTS)

IF(BUILD_TESTING)
  option(UNIT_TEST_LIGHT "Unit tests light (expect module loading failures)" FALSE)

  foreach(filename ${UNIT_TESTS})
    message(STATUS "Adding test ${filename}")
    add_test(${filename} chai ${CMAKE_CURRENT_SOURCE_DIR}/unittests/unit_test.inc ${CMAKE_CURRENT_SOURCE_DIR}/unittests/${filename})
  endforeach(filename)

  SET_PROPERTY(TEST ${UNIT_TESTS}
   PROPERTY ENVIRONMENT
    "CHAI_USE_PATH=${CMAKE_CURRENT_SOURCE_DIR}/unittests/"
    "CHAI_MODULE_PATH=${CMAKE_CURRENT_BINARY_DIR}/"
  )

  IF (NOT UNIT_TEST_LIGHT)
    add_executable(utility_test unittests/utility_test.cpp)
    target_link_libraries(utility_test ${LIBS})
    add_test(NAME Utility_Test COMMAND utility_test)

    add_executable(dynamic_object_test unittests/dynamic_object_test.cpp)
    target_link_libraries(dynamic_object_test ${LIBS})
    add_test(NAME Dynamic_Object_Test COMMAND dynamic_object_test)

    add_executable(functor_creation_test unittests/functor_creation_test.cpp)
    target_link_libraries(functor_creation_test ${LIBS})
    add_test(NAME Functor_Creation_Test COMMAND functor_creation_test)

    add_executable(functor_cast_test unittests/functor_cast_test.cpp)
    target_link_libraries(functor_cast_test ${LIBS})
    add_test(NAME Functor_Cast_Test COMMAND functor_cast_test)
    
    add_executable(boxed_cast_test unittests/boxed_cast_test.cpp)
    target_link_libraries(boxed_cast_test ${LIBS})
    add_test(NAME Boxed_Cast_Test COMMAND boxed_cast_test)

    add_executable(multifile_test unittests/multifile_test_main.cpp unittests/multifile_test_chai.cpp
      unittests/multifile_test_module.cpp)
    target_link_libraries(multifile_test ${LIBS})
    add_test(NAME MultiFile_Test COMMAND multifile_test)

    add_library(test_module MODULE src/test_module.cpp)
    target_link_libraries(test_module ${LIBS})

    install(TARGETS test_module RUNTIME DESTINATION bin LIBRARY DESTINATION lib/chaiscript)
  ENDIF()
ENDIF(BUILD_TESTING)

install(TARGETS chai stl_extra RUNTIME DESTINATION bin LIBRARY DESTINATION lib/chaiscript )
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
        

