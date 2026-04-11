// Regression test for #632 and #636:
// Heap-use-after-free when async threads outlive the ChaiScript engine.
// The engine must join all outstanding async threads before destroying shared state.

#include <chaiscript/chaiscript.hpp>

int main() {
  // Test 1: Async threads still running when engine is destroyed.
  // Without the fix, this triggers heap-use-after-free under ASAN/TSAN.
  for (int iter = 0; iter < 3; ++iter) {
    {
      chaiscript::ChaiScript chai;
      try {
        chai.eval(R"(
          var func = fun(){
            var ret = 0;
            for (var i = 0; i < 5000; ++i) {
              ret += i;
            }
            return ret;
          }

          var fut1 = async(func);
          var fut2 = async(func);
        )");
        // Engine destroyed here without waiting for futures.
      } catch (const std::exception &) {
        // Exceptions are fine
      }
    }
  }

  // Test 2: Verify async still works correctly (results are accessible).
  {
    chaiscript::ChaiScript chai;
    auto result = chai.eval<int>(R"(
      var f = async(fun() { return 42; });
      f.get();
    )");
    if (result != 42) {
      return EXIT_FAILURE;
    }
  }

  return EXIT_SUCCESS;
}
