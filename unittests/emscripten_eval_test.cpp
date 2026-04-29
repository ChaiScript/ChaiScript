// Test that validates the eval patterns used by the Emscripten wrapper.
// Based on work by Rob Loach (https://github.com/RobLoach/ChaiScript.js)

#ifndef CHAISCRIPT_NO_THREADS
#define CHAISCRIPT_NO_THREADS
#endif

#ifndef CHAISCRIPT_NO_DYNLOAD
#define CHAISCRIPT_NO_DYNLOAD
#endif

#include "../emscripten/chaiscript_eval.hpp"
#include <cassert>
#include <chaiscript/chaiscript.hpp>
#include <cmath>
#include <string>

int main() {
  // Test eval (void return) - same as Emscripten eval()
  chaiscript_eval("var x = 42");

  // Test evalString - same as Emscripten evalString()
  [[maybe_unused]] std::string s = chaiscript_eval_string("to_string(x)");
  assert(s == "42");

  // Test evalInt - same as Emscripten evalInt()
  [[maybe_unused]] int i = chaiscript_eval_int("1 + 2");
  assert(i == 3);

  // Test evalBool - same as Emscripten evalBool()
  [[maybe_unused]] bool b = chaiscript_eval_bool("true");
  assert(b == true);

  b = chaiscript_eval_bool("false");
  assert(b == false);

  // Test evalFloat - same as Emscripten evalFloat()
  [[maybe_unused]] float f = chaiscript_eval_float("1.5f");
  assert(std::abs(f - 1.5f) < 0.001f);

  // Test evalDouble - same as Emscripten evalDouble()
  [[maybe_unused]] double d = chaiscript_eval_double("3.14");
  assert(std::abs(d - 3.14) < 0.001);

  // Test a more complex expression
  chaiscript_eval("def square(n) { return n * n; }");
  [[maybe_unused]] int sq = chaiscript_eval_int("square(7)");
  assert(sq == 49);

  return 0;
}
