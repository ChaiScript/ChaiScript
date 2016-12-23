project(chaiscript_test_package)
cmake_minimum_required(VERSION 2.8)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
include(${CONAN_CMAKE-UTILS_ROOT}/conan.cmake)
conan_basic_setup()

add_conan_library(chaiscript)

add_executable(example example.cpp)
if(NOT MSVC)
    target_compile_options(example PRIVATE -std=c++14)
endif()
target_link_libraries(example PRIVATE chaiscript-conan)
