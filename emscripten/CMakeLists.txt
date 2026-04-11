# Emscripten/WebAssembly build for ChaiScript
# Based on work by Rob Loach: https://github.com/RobLoach/ChaiScript.js
#
# Usage:
#   emcmake cmake -B build-em -S emscripten
#   cmake --build build-em

cmake_minimum_required(VERSION 3.12)
project(chaiscript_em)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Emscripten-specific compiler/linker flags
add_definitions(-DCHAISCRIPT_NO_THREADS -DCHAISCRIPT_NO_DYNLOAD)

add_executable(chaiscript chaiscript_em.cpp)
target_include_directories(chaiscript PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../include)

# Emscripten link flags: enable embind, allow memory growth, export as ES module-compatible
target_link_options(chaiscript PRIVATE
  --bind
  -sALLOW_MEMORY_GROWTH=1
  -sEXPORT_ES6=0
  -sMODULARIZE=0
  -sINVOKE_RUN=0
)

# Copy the HTML shell to the build output directory
add_custom_command(TARGET chaiscript POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy
    ${CMAKE_CURRENT_SOURCE_DIR}/chaiscript.html
    ${CMAKE_CURRENT_BINARY_DIR}/chaiscript.html
  COMMENT "Copying HTML frontend to build directory"
)
