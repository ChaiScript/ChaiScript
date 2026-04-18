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

namespace detail {
  inline chaiscript::ChaiScript &get_chai_instance() {
    static chaiscript::ChaiScript chai;
    return chai;
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

#endif /* CHAISCRIPT_EMSCRIPTEN_EVAL_HPP_ */
