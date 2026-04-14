// Test that validates exception propagation through the Emscripten eval wrapper.
// Without proper exception support flags (-fwasm-exceptions) in the WASM build,
// C++ exceptions would cause an abort instead of being catchable.

#ifndef CHAISCRIPT_NO_THREADS
#define CHAISCRIPT_NO_THREADS
#endif

#ifndef CHAISCRIPT_NO_DYNLOAD
#define CHAISCRIPT_NO_DYNLOAD
#endif

#include <chaiscript/chaiscript.hpp>
#include "../emscripten/chaiscript_eval.hpp"
#include <cassert>
#include <iostream>
#include <stdexcept>
#include <string>

int main() {
  // Verify that ChaiScript evaluation errors propagate as exceptions
  // through the eval wrapper functions. In WASM builds without exception
  // support, these would abort instead of throwing.

  bool caught = false;

  // Test 1: eval with undefined variable should throw
  caught = false;
  try {
    chaiscript_eval("this_variable_does_not_exist");
  } catch (const chaiscript::exception::eval_error &) {
    caught = true;
  }
  assert(caught && "eval of undefined variable must throw eval_error");

  // Test 2: evalString with a type mismatch should throw
  caught = false;
  try {
    chaiscript_eval_string("1 + 2");
  } catch (const chaiscript::exception::bad_boxed_cast &) {
    caught = true;
  }
  assert(caught && "evalString with non-string result must throw bad_boxed_cast");

  // Test 3: evalInt with invalid syntax should throw
  caught = false;
  try {
    chaiscript_eval_int("def {}");
  } catch (const chaiscript::exception::eval_error &) {
    caught = true;
  }
  assert(caught && "evalInt with syntax error must throw eval_error");

  // Test 4: eval with throw statement should propagate exception
  caught = false;
  try {
    chaiscript_eval("throw(\"user exception\")");
  } catch (const chaiscript::Boxed_Value &) {
    caught = true;
  } catch (...) {
    caught = true;
  }
  assert(caught && "ChaiScript throw must propagate as an exception");

  // Test 5: Verify normal operation still works after caught exceptions
  chaiscript_eval("var post_exception_test = 100");
  const int result = chaiscript_eval_int("post_exception_test");
  assert(result == 100 && "normal eval must work after caught exceptions");

  std::cout << "All emscripten exception tests passed.\n";
  return 0;
}
