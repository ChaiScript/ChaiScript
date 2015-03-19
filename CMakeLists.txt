cmake_minimum_required(VERSION 2.8)

IF(BIICODE)
  INIT_BIICODE_BLOCK()
  ADD_BIICODE_TARGETS()
ELSE()
  # Your regular CMakeLists configuration here

project(chaiscript)

# MINGW does not yet support C++11's concurrency features
if(MINGW)
  option(MULTITHREAD_SUPPORT_ENABLED "Multithreaded Support Enabled" FALSE)
else()
  option(MULTITHREAD_SUPPORT_ENABLED "Multithreaded Support Enabled" TRUE)
endif()


option(BUILD_MODULES "Build Extra Modules (stl)" TRUE)
option(BUILD_SAMPLES "Build Samples Folder" FALSE)


if(CMAKE_COMPILER_IS_GNUCC)
  option(ENABLE_COVERAGE "Enable Coverage Reporting in GCC" FALSE)

  if(ENABLE_COVERAGE)
    add_definitions(--coverage -O0)
    set(LINKER_FLAGS "${LINKER_FLAGS} --coverage")
  endif()
endif()

if(CMAKE_COMPILER_IS_GNUCC OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  option(ENABLE_THREAD_SANITIZER "Enable thread sanitizer testing in gcc/clang" FALSE)
  if(ENABLE_THREAD_SANITIZER)
    add_definitions(-fsanitize=thread -g)
    set(LINKER_FLAGS "${LINKER_FLAGS} -fsanitize=thread")
  endif()

  option(ENABLE_ADDRESS_SANITIZER "Enable address sanitizer testing in gcc/clang" FALSE)
  if(ENABLE_ADDRESS_SANITIZER)
    add_definitions(-fsanitize=address -g)
    set(LINKER_FLAGS "${LINKER_FLAGS} -fsanitize=address")
  endif()

  option(ENABLE_MEMORY_SANITIZER "Enable memory sanitizer testing in gcc/clang" FALSE)
  if(ENABLE_MEMORY_SANITIZER)
    add_definitions(-fsanitize=memory -g)
    set(LINKER_FLAGS "${LINKER_FLAGS} -fsanitize=memory")
  endif()

  option(ENABLE_UNDEFINED_SANITIZER "Enable undefined behavior sanitizer testing in gcc/clang" FALSE)
  if(ENABLE_UNDEFINED_SANITIZER)
    add_definitions(-fsanitize=undefined -g)
    set(LINKER_FLAGS "${LINKER_FLAGS} -fsanitize=undefined")
  endif()

  option(ENABLE_LTO "Enable Link Time Optimization" FALSE)

  if (ENABLE_LTO)
    add_definitions(-flto)
    set(LINKER_FLAGS "${LINKER_FLAGS} -flto")
  endif()

  option(PROFILE_GENERATE "Generate profile data" FALSE)
  if (PROFILE_GENERATE)
    add_definitions(-fprofile-generate)
    set(LINKER_FLAGS "${LINKER_FLAGS} -fprofile-generate")
  endif()

  option(PROFILE_USE "Use profile data" FALSE)
  if (PROFILE_USE)
    add_definitions(-fprofile-use)
    set(LINKER_FLAGS "${LINKER_FLAGS} -fprofile-use")
  endif()


endif()

list(APPEND CPACK_SOURCE_IGNORE_FILES "${CMAKE_CURRENT_BINARY_DIR}")
list(APPEND CPACK_SOURCE_IGNORE_FILES "\\\\.svn")
list(APPEND CPACK_SOURCE_IGNORE_FILES "\\\\.git")
list(APPEND CPACK_SOURCE_IGNORE_FILES ".swp")
list(APPEND CPACK_SOURCE_IGNORE_FILES ".*~")

set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/license.txt")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/readme.md")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/description.txt")

set(CPACK_PACKAGE_VERSION_MAJOR 5)
set(CPACK_PACKAGE_VERSION_MINOR 5)
set(CPACK_PACKAGE_VERSION_PATCH 1)

set(CPACK_PACKAGE_EXECUTABLES "chai;ChaiScript Eval")
set(CPACK_PACKAGE_VENDOR "ChaiScript.com")
set(CPACK_PACKAGE_CONTACT "contact@chaiscript.com")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "An embedded scripting language for C++")

set(CPACK_DEBIAN_PACKAGE_SECTION "devel")
set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")

set(CPACK_RPM_PACKAGE_LICENSE "BSD")
set(CPACK_RPM_PACKAGE_GROUP "Programming")

set(CHAI_VERSION ${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH})

configure_file(Doxyfile.in ${CMAKE_BINARY_DIR}/Doxyfile)

include(CTest)
include(CPack)

include(cmake/CheckCXX11Features.cmake)

if(NOT MINGW)
  find_library(READLINE_LIBRARY NAMES readline PATH /usr/lib /usr/local/lib /opt/local/lib)
endif()

if(HAS_CXX11_VARIADIC_TEMPLATES)
  message(STATUS "Variadic Template support detected")
else()
  message(SEND_ERROR "The selected compiler does not support the C++11 feature Variadic Templates.")
endif()

enable_testing()


message(STATUS "Detecting readline support")
if(READLINE_LIBRARY)
  message(STATUS "Found: ${READLINE_LIBRARY}")
  set(READLINE_LIB readline)
  add_definitions(/DREADLINE_AVAILABLE)
else()
  message(STATUS "Not Found")
  set(READLINE_LIB)
  set(READLINE_FLAG)
endif()

if(CMAKE_COMPILER_IS_GNUCC)
  execute_process(COMMAND ${CMAKE_C_COMPILER} -dumpversion OUTPUT_VARIABLE GCC_VERSION)

  if(GCC_VERSION VERSION_LESS 4.8)
    set(CPP11_FLAG "-std=c++0x")
  else()
    set(CPP11_FLAG "-std=c++11")
  endif()
else()
  set(CPP11_FLAG "-std=c++11")
endif()

if(MSVC)
  add_definitions(/W4 /w44640)
  add_definitions(/bigobj)
  # Note on MSVC compiler flags.
  # The code base selective disables warnings as necessary when the compiler is complaining too much
  # about something that is perfectly valid, or there is simply no technical way around it
  # This particular warning, C4503 is in regards to the decorated names that MSVC generates internally.
  # The error did not come up until the move to C++11, but the compiler doesn't give enough information
  # to determine where the error is coming from, and the internet provides no real information for
  # how to workaround or fix the error. So I'm disabling it globally.
  add_definitions(/wd4503)
else()
  add_definitions(-Wall -Wextra -Wshadow -Wnon-virtual-dtor -pedantic ${CPP11_FLAG})

  if(APPLE)
    add_definitions(-Wno-sign-compare)
  endif()
endif()

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  option(USE_LIBCXX "Use clang's libcxx" TRUE)

  if(USE_LIBCXX)
    add_definitions(-stdlib=libc++)
    set(LINKER_FLAGS "${LINKER_FLAGS} ${CPP11_FLAG} -stdlib=libc++")
  else()
    set(LINKER_FLAGS "${LINKER_FLAGS} ${CPP11_FLAG}")
  endif()
elseif(CMAKE_COMPILER_IS_GNUCC)
  set(LINKER_FLAGS "${LINKER_FLAGS} ${CPP11_FLAG}")
endif()

# limitations in MinGW require us to make an optimized build
# for the sake of object sizes or something
if(MINGW OR CYGWIN)
  add_definitions(-O3)
endif()

include_directories(include)


set(Chai_INCLUDES include/chaiscript/chaiscript.hpp include/chaiscript/chaiscript_threading.hpp include/chaiscript/dispatchkit/bad_boxed_cast.hpp include/chaiscript/dispatchkit/bind_first.hpp include/chaiscript/dispatchkit/bootstrap.hpp include/chaiscript/dispatchkit/bootstrap_stl.hpp include/chaiscript/dispatchkit/boxed_cast.hpp include/chaiscript/dispatchkit/boxed_cast_helper.hpp include/chaiscript/dispatchkit/boxed_number.hpp include/chaiscript/dispatchkit/boxed_value.hpp include/chaiscript/dispatchkit/dispatchkit.hpp include/chaiscript/dispatchkit/type_conversions.hpp include/chaiscript/dispatchkit/dynamic_object.hpp include/chaiscript/dispatchkit/exception_specification.hpp include/chaiscript/dispatchkit/function_call.hpp include/chaiscript/dispatchkit/function_call_detail.hpp include/chaiscript/dispatchkit/handle_return.hpp include/chaiscript/dispatchkit/operators.hpp include/chaiscript/dispatchkit/proxy_constructors.hpp include/chaiscript/dispatchkit/proxy_functions.hpp include/chaiscript/dispatchkit/proxy_functions_detail.hpp include/chaiscript/dispatchkit/register_function.hpp include/chaiscript/dispatchkit/type_info.hpp include/chaiscript/language/chaiscript_algebraic.hpp include/chaiscript/language/chaiscript_common.hpp include/chaiscript/language/chaiscript_engine.hpp include/chaiscript/language/chaiscript_eval.hpp include/chaiscript/language/chaiscript_parser.hpp include/chaiscript/language/chaiscript_prelude.chai include/chaiscript/language/chaiscript_prelude_docs.hpp include/chaiscript/utility/utility.hpp)

set_source_files_properties(${Chai_INCLUDES} PROPERTIES HEADER_FILE_ONLY TRUE)

if(NOT MULTITHREAD_SUPPORT_ENABLED)
  add_definitions(-DCHAISCRIPT_NO_THREADS)
endif()

if(CMAKE_HOST_UNIX)
  if(NOT ${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD" AND NOT ${CMAKE_SYSTEM_NAME} MATCHES "Haiku")
    list(APPEND LIBS "dl")
  endif()

  if(MULTITHREAD_SUPPORT_ENABLED)
    if(CMAKE_COMPILER_IS_GNUCC)
      execute_process(COMMAND ${CMAKE_C_COMPILER} --version OUTPUT_VARIABLE GCC_FULL_VERSION)
      if(GCC_FULL_VERSION MATCHES "4.8.1.*ubuntu")
        set(LINKER_FLAGS "${LINKER_FLAGS} -Wl,--no-as-needed -pthread")
      else()
        set(LINKER_FLAGS "${LINKER_FLAGS} -pthread")
      endif()
    else()
      set(LINKER_FLAGS "${LINKER_FLAGS} -pthread")
    endif()

    add_definitions(-pthread)
  endif()

endif()

list(APPEND LIBS ${READLINE_LIB})

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${LINKER_FLAGS}")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${LINKER_FLAGS}")
set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} ${LINKER_FLAGS}")


add_library(chaiscript_stdlib-${CHAI_VERSION} MODULE src/chaiscript_stdlib.cpp)
target_link_libraries(chaiscript_stdlib-${CHAI_VERSION} ${LIBS} ${CMAKE_THREAD_LIBS_INIT})

add_executable(chai src/main.cpp ${Chai_INCLUDES})
target_link_libraries(chai ${LIBS})
add_dependencies(chai chaiscript_stdlib-${CHAI_VERSION})

if(BUILD_SAMPLES)
  add_executable(example samples/example.cpp)
  target_link_libraries(example ${LIBS})
  add_executable(memory_leak_test samples/memory_leak_test.cpp)
  target_link_libraries(memory_leak_test ${LIBS})
  add_executable(inheritance samples/inheritance.cpp)
  target_link_libraries(inheritance ${LIBS})
  add_executable(fun_call_performance samples/fun_call_performance.cpp)
  target_link_libraries(fun_call_performance ${LIBS})
endif()


if(BUILD_MODULES)
  add_library(stl_extra MODULE src/stl_extra.cpp)
  target_link_libraries(stl_extra ${LIBS})

  set(MODULES stl_extra)
endif()

file(GLOB UNIT_TESTS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}/unittests/ ${CMAKE_CURRENT_SOURCE_DIR}/unittests/*.chai ${CMAKE_CURRENT_SOURCE_DIR}/unittests/3.x/*.chai)

list(SORT UNIT_TESTS)

if(BUILD_TESTING)
  option(UNIT_TEST_LIGHT "Unit tests light (expect module loading failures)" FALSE)

  add_test(version_check chai -c  "if(\"\\\${ version() };\\\${version_major()};\\\${version_minor()};\\\${version_patch()}\" != \"${CHAI_VERSION};${CPACK_PACKAGE_VERSION_MAJOR};${CPACK_PACKAGE_VERSION_MINOR};${CPACK_PACKAGE_VERSION_PATCH}\") { exit(-1) }")
  set_property(TEST version_check
    PROPERTY ENVIRONMENT
    "CHAI_USE_PATH=${CMAKE_CURRENT_SOURCE_DIR}/unittests/"
    "CHAI_MODULE_PATH=${CMAKE_CURRENT_BINARY_DIR}/"
  )


  foreach(filename ${UNIT_TESTS})
    message(STATUS "Adding test ${filename}")
    add_test(${filename} chai ${CMAKE_CURRENT_SOURCE_DIR}/unittests/unit_test.inc ${CMAKE_CURRENT_SOURCE_DIR}/unittests/${filename})
  endforeach()

  set_property(TEST ${UNIT_TESTS}
    PROPERTY ENVIRONMENT
    "CHAI_USE_PATH=${CMAKE_CURRENT_SOURCE_DIR}/unittests/"
    "CHAI_MODULE_PATH=${CMAKE_CURRENT_BINARY_DIR}/"
  )

  if(NOT UNIT_TEST_LIGHT)
    # commented out because uniform initializer syntax is not working properly in MSVC 2013
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

    add_executable(object_lifetime_test unittests/object_lifetime_test.cpp)
    target_link_libraries(object_lifetime_test ${LIBS})
    add_test(NAME Object_Lifetime_Test COMMAND object_lifetime_test)

    add_executable(object_lifetime_test2 unittests/object_lifetime_test2.cpp)
    target_link_libraries(object_lifetime_test2 ${LIBS})
    add_test(NAME Object_Lifetime_Test2 COMMAND object_lifetime_test2)

    add_executable(function_ordering_test unittests/function_ordering_test.cpp)
    target_link_libraries(function_ordering_test ${LIBS})
    add_test(NAME Function_Ordering_Test COMMAND function_ordering_test)

    add_executable(type_info_test unittests/type_info_test.cpp)
    target_link_libraries(type_info_test ${LIBS})
    add_test(NAME Type_Info_Test COMMAND type_info_test)

    add_executable(type_name_test unittests/type_name_test.cpp)
    target_link_libraries(type_name_test ${LIBS})
    add_test(NAME Type_Name_Test COMMAND type_name_test)

    add_executable(eval_catch_exception_test unittests/eval_catch_exception_test.cpp)
    target_link_libraries(eval_catch_exception_test ${LIBS})
    add_test(NAME Eval_Catch_Exception_Test COMMAND eval_catch_exception_test)

    add_executable(short_comparison_test unittests/short_comparison_test.cpp)
    target_link_libraries(short_comparison_test ${LIBS})
    add_test(NAME Short_Comparison_Test COMMAND short_comparison_test)

    add_executable(cpp_lambda_test unittests/cpp_lambda_test.cpp)
    target_link_libraries(cpp_lambda_test ${LIBS})
    add_test(NAME cpp_lambda_test COMMAND cpp_lambda_test)

    add_executable(expected_eval_errors_test unittests/expected_eval_errors_test.cpp)
    target_link_libraries(expected_eval_errors_test ${LIBS})
    add_test(NAME Expected_Eval_Errors_Test COMMAND expected_eval_errors_test)

    add_executable(set_state_test unittests/set_state_test.cpp)
    target_link_libraries(set_state_test ${LIBS})
    add_test(NAME Set_State_Test COMMAND set_state_test)

    add_executable(simultaneous_chaiscript_test unittests/simultaneous_chaiscript_test.cpp)
    target_link_libraries(simultaneous_chaiscript_test ${LIBS})
    add_test(NAME Simultaneous_ChaiScript_Test COMMAND simultaneous_chaiscript_test)

    add_executable(heap_allocated_chaiscript_test unittests/heap_allocated_chaiscript_test.cpp)
    target_link_libraries(heap_allocated_chaiscript_test ${LIBS})
    add_test(NAME Heap_Allocated_ChaiScript_Test COMMAND heap_allocated_chaiscript_test)

    add_executable(c_linkage_test unittests/c_linkage_test.cpp)
    target_link_libraries(c_linkage_test ${LIBS})
    add_test(NAME C_Linkage_Test COMMAND c_linkage_test)

    add_executable(integer_literal_test unittests/integer_literal_test.cpp)
    target_link_libraries(integer_literal_test ${LIBS})
    add_test(NAME Integer_Literal_Test COMMAND integer_literal_test)

    add_executable(arithmetic_conversions_test unittests/arithmetic_conversions_test.cpp)
    target_link_libraries(arithmetic_conversions_test ${LIBS})
    add_test(NAME Arithmetic_Conversions_Test COMMAND arithmetic_conversions_test)

    if(MULTITHREAD_SUPPORT_ENABLED)
      add_executable(multithreaded_test unittests/multithreaded_test.cpp)
      target_link_libraries(multithreaded_test ${LIBS})
      add_test(NAME Multithreaded_Test COMMAND multithreaded_test)
      set_property(TEST Multithreaded_Test
        PROPERTY ENVIRONMENT
        "CHAI_USE_PATH=${CMAKE_CURRENT_SOURCE_DIR}/unittests/"
        "CHAI_MODULE_PATH=${CMAKE_CURRENT_BINARY_DIR}/"
      )

    endif()


    add_executable(multifile_test
      unittests/multifile_test_main.cpp
      unittests/multifile_test_chai.cpp
      unittests/multifile_test_module.cpp
    )
    target_link_libraries(multifile_test ${LIBS})
    add_test(NAME MultiFile_Test COMMAND multifile_test)

    add_library(test_module MODULE src/test_module.cpp)
    target_link_libraries(test_module ${LIBS})

    install(TARGETS test_module RUNTIME DESTINATION bin LIBRARY DESTINATION lib/chaiscript)
  endif()
endif()

install(TARGETS chai chaiscript_stdlib-${CHAI_VERSION} ${MODULES} RUNTIME DESTINATION bin LIBRARY DESTINATION lib/chaiscript)

install(DIRECTORY include/chaiscript DESTINATION include
  PATTERN "*.hpp"
  PATTERN "*/.svn*" EXCLUDE
  PATTERN "*/.git*" EXCLUDE
  PATTERN "*~" EXCLUDE)
install(DIRECTORY unittests DESTINATION share/chaiscript
  PATTERN "*.chai"
  PATTERN "*.inc"
  PATTERN "*/.svn*" EXCLUDE
  PATTERN "*/.git*" EXCLUDE
  PATTERN "*~" EXCLUDE)
install(DIRECTORY samples DESTINATION share/chaiscript
  PATTERN "*.chai"
  PATTERN "*/.svn*" EXCLUDE
  PATTERN "*/.git*" EXCLUDE
  PATTERN "*~" EXCLUDE)

configure_file(contrib/pkgconfig/chaiscript.pc.in lib/pkgconfig/chaiscript.pc @ONLY)
install(FILES "${chaiscript_BINARY_DIR}/lib/pkgconfig/chaiscript.pc"
        DESTINATION lib/pkgconfig)

    
ENDIF()

