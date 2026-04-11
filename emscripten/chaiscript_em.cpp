// This file is distributed under the BSD License.
// See "license.txt" for details.
// Copyright 2019, Rob Loach (https://github.com/RobLoach/ChaiScript.js)
// Copyright 2009-2018, Jason Turner (jason@emptycrate.com)
// http://www.chaiscript.com
//
// Emscripten/WebAssembly wrapper for ChaiScript.
// Based on work by Rob Loach: https://github.com/RobLoach/ChaiScript.js

#include "chaiscript_eval.hpp"

#ifdef __EMSCRIPTEN__
#include <emscripten/bind.h>

EMSCRIPTEN_BINDINGS(chaiscript) {
  emscripten::function("eval", &chaiscript_eval);
  emscripten::function("evalString", &chaiscript_eval_string);
  emscripten::function("evalBool", &chaiscript_eval_bool);
  emscripten::function("evalInt", &chaiscript_eval_int);
  emscripten::function("evalFloat", &chaiscript_eval_float);
  emscripten::function("evalDouble", &chaiscript_eval_double);
}
#endif
