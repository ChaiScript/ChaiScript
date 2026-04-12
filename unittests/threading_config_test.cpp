#include <chaiscript/chaiscript.hpp>

int main() {
#ifndef CHAISCRIPT_NO_THREADS
  // When threading is enabled, verify that async() is registered and functional
  chaiscript::ChaiScript chai;
  const auto result = chai.eval<int>(R"(
    var f = async(fun() { return 42; });
    f.get();
  )");
  if (result != 42) {
    return EXIT_FAILURE;
  }
#else
  // When threading is disabled, verify that async() is NOT registered
  chaiscript::ChaiScript chai;
  try {
    chai.eval("async(fun() { return 0; })");
    return EXIT_FAILURE;
  } catch (const std::exception &) {
    // Expected: async should not exist
  }
#endif

  return EXIT_SUCCESS;
}
