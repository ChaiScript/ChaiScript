// Test that validates the save/restore/release state helpers used by the
// Emscripten wrapper. A handle-based registry is exposed so the JS playground
// can capture a pristine engine once and restore it between runs without
// reloading the WASM module.

#ifndef CHAISCRIPT_NO_THREADS
#define CHAISCRIPT_NO_THREADS
#endif

#ifndef CHAISCRIPT_NO_DYNLOAD
#define CHAISCRIPT_NO_DYNLOAD
#endif

#include "../emscripten/chaiscript_eval.hpp"
#include <cassert>
#include <chaiscript/chaiscript.hpp>
#include <string>

int main() {
  const int chai = chaiscript_create();
  assert(chai > 0 && "create must return a positive handle");

  // Capture a baseline before any user code has been evaluated.
  const int baseline = chaiscript_save_state(chai);
  assert(baseline > 0 && "saveState must return a positive handle");

  // Distinct calls produce distinct handles.
  const int second = chaiscript_save_state(chai);
  assert(second != baseline && "saveState must mint a fresh handle each call");
  chaiscript_release_state(second);

  // Define a global that did not exist in the baseline.
  chaiscript_eval(chai, "var post_baseline_marker = 123");
  assert(chaiscript_eval_int(chai, "post_baseline_marker") == 123);

  // Restoring the baseline must drop the post-baseline definition: evaluating
  // it should now fail with eval_error because the variable is undefined.
  chaiscript_restore_state(chai, baseline);

  [[maybe_unused]] bool caught = false;
  try {
    chaiscript_eval(chai, "post_baseline_marker");
  } catch (const chaiscript::exception::eval_error &) {
    caught = true;
  }
  assert(caught && "restoreState must drop globals defined after the snapshot");

  // The engine remains usable after a restore.
  chaiscript_eval(chai, "var after_restore = 7");
  assert(chaiscript_eval_int(chai, "after_restore * 6") == 42);

  // Releasing the handle is required so the registry does not grow unbounded.
  chaiscript_release_state(baseline);

  // Releasing a handle that was never minted (or has already been released) is
  // a no-op rather than an error.
  chaiscript_release_state(baseline);
  chaiscript_release_state(99999);

  // Restoring an unknown handle is a no-op: state must be unchanged.
  chaiscript_restore_state(chai, 99999);
  assert(chaiscript_eval_int(chai, "after_restore") == 7);

  // A snapshot taken on one engine can be restored onto a different engine,
  // confirming that state is decoupled from any particular instance.
  const int snapshot_with_after_restore = chaiscript_save_state(chai);
  const int chai2 = chaiscript_create();
  chaiscript_restore_state(chai2, snapshot_with_after_restore);
  assert(chaiscript_eval_int(chai2, "after_restore") == 7);

  chaiscript_release_state(snapshot_with_after_restore);
  chaiscript_destroy(chai2);
  chaiscript_destroy(chai);

  return 0;
}
