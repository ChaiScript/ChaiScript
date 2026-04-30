// This file is distributed under the BSD License.
// See "license.txt" for details.
// Copyright 2019, Rob Loach (https://github.com/RobLoach/ChaiScript.js)
// Copyright 2009-2018, Jason Turner (jason@emptycrate.com)
// http://www.chaiscript.com

// Shared eval helper functions for the ChaiScript Emscripten wrapper.
// These functions provide typed evaluation of ChaiScript expressions,
// used by both the Emscripten/WebAssembly build and native tests.

#ifndef CHAISCRIPT_EMSCRIPTEN_EVAL_HPP_
#define CHAISCRIPT_EMSCRIPTEN_EVAL_HPP_

#include <chaiscript/chaiscript.hpp>
#include <string>
#include <unordered_map>

namespace detail {
  inline chaiscript::ChaiScript &get_chai_instance() {
    static chaiscript::ChaiScript chai;
    return chai;
  }

  inline std::unordered_map<int, chaiscript::ChaiScript::State> &state_registry() {
    static std::unordered_map<int, chaiscript::ChaiScript::State> registry;
    return registry;
  }

  inline int next_state_handle() {
    static int handle = 0;
    return ++handle;
  }
} // namespace detail

inline void chaiscript_eval(const std::string &input) {
  detail::get_chai_instance().eval(input);
}

inline std::string chaiscript_eval_string(const std::string &input) {
  return detail::get_chai_instance().eval<std::string>(input);
}

inline bool chaiscript_eval_bool(const std::string &input) {
  return detail::get_chai_instance().eval<bool>(input);
}

inline int chaiscript_eval_int(const std::string &input) {
  return detail::get_chai_instance().eval<int>(input);
}

inline float chaiscript_eval_float(const std::string &input) {
  return detail::get_chai_instance().eval<float>(input);
}

inline double chaiscript_eval_double(const std::string &input) {
  return detail::get_chai_instance().eval<double>(input);
}

// Snapshot the current engine state and return an opaque handle. The handle
// is owned by the caller; release it with chaiscript_release_state when it is
// no longer needed.
inline int chaiscript_save_state() {
  const int handle = detail::next_state_handle();
  detail::state_registry().emplace(handle, detail::get_chai_instance().get_state());
  return handle;
}

// Restore a previously snapshotted state. Unknown handles are silently
// ignored so JS callers do not need to track validity defensively.
inline void chaiscript_restore_state(const int handle) {
  const auto it = detail::state_registry().find(handle);
  if (it != detail::state_registry().end()) {
    detail::get_chai_instance().set_state(it->second);
  }
}

// Release a handle returned by chaiscript_save_state. Releasing an unknown
// handle is a no-op.
inline void chaiscript_release_state(const int handle) {
  detail::state_registry().erase(handle);
}

#endif /* CHAISCRIPT_EMSCRIPTEN_EVAL_HPP_ */
