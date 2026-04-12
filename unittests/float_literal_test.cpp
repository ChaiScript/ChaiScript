#include "../static_libs/chaiscript_parser.hpp"
#include "../static_libs/chaiscript_stdlib.hpp"
#include <chaiscript/chaiscript_basic.hpp>

#include <cmath>
#include <iomanip>
#include <iostream>
#include <string>

#define TEST_FLOAT_LITERAL(v) test_float_literal(v, #v)

template<typename T>
bool test_float_literal(const T val, const std::string &str) {
  chaiscript::ChaiScript_Basic chai(create_chaiscript_stdlib(), create_chaiscript_parser());
  const T val2 = chai.eval<T>(str);
  const bool pass = (val == val2);
  std::cout << (pass ? "PASS" : "FAIL") << " (" << str << ") C++="
            << std::setprecision(20) << val << " chai=" << val2 << "\n";
  return pass;
}

int main() {
  bool success = true;

  // Issue #378: scientific notation parsing inaccuracies
  success = TEST_FLOAT_LITERAL(1.1e-4) && success;
  success = TEST_FLOAT_LITERAL(1.5e+3) && success;
  success = TEST_FLOAT_LITERAL(3.14159) && success;
  success = TEST_FLOAT_LITERAL(2.718281828459045) && success;
  success = TEST_FLOAT_LITERAL(1.0e10) && success;
  success = TEST_FLOAT_LITERAL(1.0e-10) && success;
  success = TEST_FLOAT_LITERAL(0.1) && success;
  success = TEST_FLOAT_LITERAL(0.2) && success;
  success = TEST_FLOAT_LITERAL(1.7976931348623157e+308) && success;
  success = TEST_FLOAT_LITERAL(2.2250738585072014e-308) && success;

  // Float suffix
  success = TEST_FLOAT_LITERAL(1.1e-4f) && success;
  success = TEST_FLOAT_LITERAL(3.14f) && success;

  // Long double suffix
  success = TEST_FLOAT_LITERAL(1.1e-4l) && success;
  success = TEST_FLOAT_LITERAL(3.14l) && success;

  if (success) {
    std::cout << "All float literal tests passed.\n";
    return 0;
  } else {
    std::cout << "FLOAT LITERAL TEST FAILURE\n";
    return 1;
  }
}
