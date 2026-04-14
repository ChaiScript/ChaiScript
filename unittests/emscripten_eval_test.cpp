// Test that validates the eval patterns used by the Emscripten wrapper.
// Based on work by Rob Loach (https://github.com/RobLoach/ChaiScript.js)

#ifndef CHAISCRIPT_NO_THREADS
#define CHAISCRIPT_NO_THREADS
#endif

#ifndef CHAISCRIPT_NO_DYNLOAD
#define CHAISCRIPT_NO_DYNLOAD
#endif

#include <chaiscript/chaiscript.hpp>
#include "../emscripten/chaiscript_eval.hpp"
#include <cassert>
#include <cmath>
#include <string>

int main() {
  chaiscript_eval("var x = 42");

  const std::string s = chaiscript_eval_string("to_string(x)");
  assert(s == "42");
  static_cast<void>(s);

  const int i = chaiscript_eval_int("1 + 2");
  assert(i == 3);
  static_cast<void>(i);

  bool b = chaiscript_eval_bool("true");
  assert(b == true);
  b = chaiscript_eval_bool("false");
  assert(b == false);
  static_cast<void>(b);

  const float f = chaiscript_eval_float("1.5f");
  assert(std::abs(f - 1.5f) < 0.001f);
  static_cast<void>(f);

  const double d = chaiscript_eval_double("3.14");
  assert(std::abs(d - 3.14) < 0.001);
  static_cast<void>(d);

  chaiscript_eval("def square(n) { return n * n; }");
  const int sq = chaiscript_eval_int("square(7)");
  assert(sq == 49);
  static_cast<void>(sq);

  return 0;
}
