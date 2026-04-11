// Regression test for issue #636:
// Heap-use-after-free when async threads access engine state during destruction.
//
// The root cause: Dispatch_Engine member destruction order destroys m_state (type maps)
// before m_stack_holder (which holds async futures). When async threads are still running
// and hit an error that triggers eval_error formatting, they access the already-freed
// type map via get_type_name().

#include <chaiscript/chaiscript.hpp>
#include <iostream>

int main() {
  // Run multiple iterations to increase chance of triggering the race
  for (int iter = 0; iter < 3; ++iter) {
    // Create engine in a nested scope so it's destroyed while async tasks may still be running
    {
      chaiscript::ChaiScript chai;

      try {
        // This script launches async tasks that run a long-running loop.
        // The futures are not retrieved - when the engine is destroyed, the
        // async threads may still be accessing engine state (e.g. type maps
        // during error formatting). Without the fix, m_state is destroyed
        // before m_stack_holder (which holds the futures), causing the async
        // threads to access freed memory.
        //
        // With ASan/TSan enabled, this reliably detects the heap-use-after-free.
        chai.eval(R"(
          var func = fun(){
            var ret = 0;
            for (var i = 0; i < 100000; ++i) {
              ret += i;
            }
            return ret;
          }

          var fut1 = async(func);
          var fut2 = async(func);
        )");
      } catch (const std::exception &) {
        // Exceptions from eval are expected and fine
      }
    }
    // If the fix is not applied, the engine destruction above may cause
    // heap-use-after-free (detectable with ASan/TSan) because m_state
    // is destroyed before async threads finish.
  }

  std::cout << "Async engine lifetime test passed\n";
  return EXIT_SUCCESS;
}
