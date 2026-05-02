// This file is distributed under the BSD License.
// See "license.txt" for details.
// Copyright 2019, Rob Loach (https://github.com/RobLoach/ChaiScript.js)
// Copyright 2009-2018, Jason Turner (jason@emptycrate.com)
// http://www.chaiscript.com

// Shared eval helper functions for the ChaiScript Emscripten wrapper.
// These functions provide typed evaluation of ChaiScript expressions,
// used by both the Emscripten/WebAssembly build and native tests.
//
// The interface is opaque and handle-based: JS callers create one or more
// engines via chaiscript_create(), pass the resulting handle to the eval and
// state helpers, and release the engine with chaiscript_destroy() when done.
// State snapshots are themselves opaque handles produced by
// chaiscript_save_state() and consumed by chaiscript_restore_state() /
// chaiscript_release_state(). Hiding ChaiScript and ChaiScript::State behind
// integer handles keeps embind from having to manage their lifetimes and
// avoids forcing a static singleton on the C++ side.

#ifndef CHAISCRIPT_EMSCRIPTEN_EVAL_HPP_
#define CHAISCRIPT_EMSCRIPTEN_EVAL_HPP_

#include <chaiscript/chaiscript.hpp>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>

namespace detail {
  // ChaiScript::State captures globals/functions/types but not the top-level
  // scripting locals (variables created by `var x = ...` at the script's
  // outermost scope). The playground's reset-between-runs use case needs both,
  // so the snapshot pairs the engine state with the locals map.
  struct Snapshot {
    chaiscript::ChaiScript::State engine_state;
    std::map<std::string, chaiscript::Boxed_Value> locals;
  };

  inline std::unordered_map<int, std::unique_ptr<chaiscript::ChaiScript>> &chai_registry() {
    static std::unordered_map<int, std::unique_ptr<chaiscript::ChaiScript>> registry;
    return registry;
  }

  inline std::unordered_map<int, Snapshot> &state_registry() {
    static std::unordered_map<int, Snapshot> registry;
    return registry;
  }

  inline int next_handle() {
    static int handle = 0;
    return ++handle;
  }

  inline chaiscript::ChaiScript &get_chai(const int handle) {
    return *chai_registry().at(handle);
  }
} // namespace detail

// Construct a fresh ChaiScript engine and return an opaque handle. The handle
// is owned by the caller and must be released with chaiscript_destroy().
inline int chaiscript_create() {
  const int handle = detail::next_handle();
  detail::chai_registry().emplace(handle, std::make_unique<chaiscript::ChaiScript>());
  return handle;
}

// Destroy an engine handle. Unknown handles are silently ignored so JS callers
// do not need to track validity defensively.
inline void chaiscript_destroy(const int handle) {
  detail::chai_registry().erase(handle);
}

inline void chaiscript_eval(const int handle, const std::string &input) {
  detail::get_chai(handle).eval(input);
}

inline std::string chaiscript_eval_string(const int handle, const std::string &input) {
  return detail::get_chai(handle).eval<std::string>(input);
}

inline bool chaiscript_eval_bool(const int handle, const std::string &input) {
  return detail::get_chai(handle).eval<bool>(input);
}

inline int chaiscript_eval_int(const int handle, const std::string &input) {
  return detail::get_chai(handle).eval<int>(input);
}

inline float chaiscript_eval_float(const int handle, const std::string &input) {
  return detail::get_chai(handle).eval<float>(input);
}

inline double chaiscript_eval_double(const int handle, const std::string &input) {
  return detail::get_chai(handle).eval<double>(input);
}

// Snapshot the engine identified by chai_handle and return a fresh opaque
// state handle. Release it with chaiscript_release_state() when no longer
// needed.
inline int chaiscript_save_state(const int chai_handle) {
  const int state_handle = detail::next_handle();
  auto &chai = detail::get_chai(chai_handle);
  detail::state_registry().emplace(state_handle, detail::Snapshot{chai.get_state(), chai.get_locals()});
  return state_handle;
}

// Restore a previously snapshotted state onto the engine identified by
// chai_handle. Unknown state handles are silently ignored.
inline void chaiscript_restore_state(const int chai_handle, const int state_handle) {
  const auto it = detail::state_registry().find(state_handle);
  if (it != detail::state_registry().end()) {
    auto &chai = detail::get_chai(chai_handle);
    chai.set_state(it->second.engine_state);
    chai.set_locals(it->second.locals);
  }
}

// Release a state handle returned by chaiscript_save_state. Releasing an
// unknown handle is a no-op.
inline void chaiscript_release_state(const int state_handle) {
  detail::state_registry().erase(state_handle);
}

#endif /* CHAISCRIPT_EMSCRIPTEN_EVAL_HPP_ */
