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
  const int chai = chaiscript_create();
  assert(chai > 0 && "create must return a positive handle");

  // Test eval (void return) - same as Emscripten eval()
  chaiscript_eval(chai, "var x = 42");

  // Test evalString - same as Emscripten evalString()
  [[maybe_unused]] const std::string s = chaiscript_eval_string(chai, "to_string(x)");
  assert(s == "42");

  // Test evalInt - same as Emscripten evalInt()
  [[maybe_unused]] const int i = chaiscript_eval_int(chai, "1 + 2");
  assert(i == 3);

  // Test evalBool - same as Emscripten evalBool()
  [[maybe_unused]] bool b = chaiscript_eval_bool(chai, "true");
  assert(b == true);

  b = chaiscript_eval_bool(chai, "false");
  assert(b == false);

  // Test evalFloat - same as Emscripten evalFloat()
  [[maybe_unused]] const float f = chaiscript_eval_float(chai, "1.5f");
  assert(std::abs(f - 1.5f) < 0.001f);

  // Test evalDouble - same as Emscripten evalDouble()
  [[maybe_unused]] const double d = chaiscript_eval_double(chai, "3.14");
  assert(std::abs(d - 3.14) < 0.001);

  // Test a more complex expression
  chaiscript_eval(chai, "def square(n) { return n * n; }");
  [[maybe_unused]] const int sq = chaiscript_eval_int(chai, "square(7)");
  assert(sq == 49);

  // A second engine is fully independent of the first.
  const int chai2 = chaiscript_create();
  assert(chai2 != chai && "create must mint a fresh handle each call");
  [[maybe_unused]] bool caught = false;
  try {
    chaiscript_eval(chai2, "x");
  } catch (const chaiscript::exception::eval_error &) {
    caught = true;
  }
  assert(caught && "engines must not share globals");

  chaiscript_destroy(chai2);
  chaiscript_destroy(chai);

  // Destroying an unknown handle is a no-op.
  chaiscript_destroy(chai);
  chaiscript_destroy(99999);

  return 0;
}
