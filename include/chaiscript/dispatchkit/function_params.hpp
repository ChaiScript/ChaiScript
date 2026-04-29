// This file is distributed under the BSD License.
// See "license.txt" for details.
// Copyright 2009-2012, Jonathan Turner (jonathan@emptycrate.com)
// Copyright 2009-2017, Jason Turner (jason@emptycrate.com)
// http://www.chaiscript.com

// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#ifndef CHAISCRIPT_FUNCTION_PARAMS_HPP
#define CHAISCRIPT_FUNCTION_PARAMS_HPP

#include "boxed_value.hpp"

#include <span>

namespace chaiscript {

  using Function_Params = std::span<const Boxed_Value>;

} // namespace chaiscript

#endif
