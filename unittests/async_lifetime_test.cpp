// Regression test for #632: Heap-use-after-free in async threads during engine shutdown.
// The engine must wait for outstanding async threads before destroying shared state.

#include <chaiscript/chaiscript.hpp>
#include <chaiscript/chaiscript_basic.hpp>
#include <chaiscript/language/chaiscript_parser.hpp>

int main() {
  // Test 1: Async threads that are still running when the engine is destroyed.
  // Without the fix, this triggers a heap-use-after-free under ASAN/TSAN.
  {
    chaiscript::ChaiScript chai;
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
    // Engine is destroyed here without explicitly waiting for futures.
    // The fix ensures the engine joins all async threads before destruction.
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
