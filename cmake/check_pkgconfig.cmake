# Verify the generated chaiscript.pc has correct prefix and libdir values.
# This test catches issues where CMAKE_INSTALL_PREFIX or CMAKE_INSTALL_LIBDIR
# are not properly respected in the pkg-config file.

file(READ "${PC_FILE}" PC_CONTENT)

# Check that prefix matches CMAKE_INSTALL_PREFIX
string(REGEX MATCH "prefix=([^\n]*)" PREFIX_MATCH "${PC_CONTENT}")
set(ACTUAL_PREFIX "${CMAKE_MATCH_1}")

if(NOT ACTUAL_PREFIX STREQUAL EXPECTED_PREFIX)
  message(FATAL_ERROR
    "pkgconfig prefix mismatch:\n"
    "  expected: '${EXPECTED_PREFIX}'\n"
    "  actual:   '${ACTUAL_PREFIX}'\n"
    "CMAKE_INSTALL_PREFIX is not being respected in chaiscript.pc")
endif()

# Check that libdir uses the correct lib directory name
string(REGEX MATCH "libdir=([^\n]*)" LIBDIR_MATCH "${PC_CONTENT}")
set(ACTUAL_LIBDIR "${CMAKE_MATCH_1}")

set(EXPECTED_LIBDIR_VALUE "\${exec_prefix}/${EXPECTED_LIBDIR}")
if(NOT ACTUAL_LIBDIR STREQUAL EXPECTED_LIBDIR_VALUE)
  message(FATAL_ERROR
    "pkgconfig libdir mismatch:\n"
    "  expected: '${EXPECTED_LIBDIR_VALUE}'\n"
    "  actual:   '${ACTUAL_LIBDIR}'\n"
    "CMAKE_INSTALL_LIBDIR is not being respected in chaiscript.pc")
endif()

message(STATUS "pkgconfig check passed: prefix='${ACTUAL_PREFIX}', libdir='${ACTUAL_LIBDIR}'")
