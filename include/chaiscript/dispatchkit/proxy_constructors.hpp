// This file is distributed under the BSD License.
// See "license.txt" for details.
// Copyright 2009-2012, Jonathan Turner (jonathan@emptycrate.com)
// Copyright 2009-2018, Jason Turner (jason@emptycrate.com)
// http://www.chaiscript.com

// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#ifndef CHAISCRIPT_PROXY_CONSTRUCTORS_HPP_
#define CHAISCRIPT_PROXY_CONSTRUCTORS_HPP_

#include "proxy_functions.hpp"

namespace chaiscript::dispatch::detail {
  template<typename Class, typename... Params>
  Proxy_Function build_constructor_(Class (*)(Params...)) {
    // is_copy_constructible is required to avoid compilation errors if classes cannot be copied
    // is_trivially_destructible is required to avoid bugs if classes don't correctly implement
    // copy or move. To avoid unexpected runtime behavior we allocate them as shared_ptrs.
    if constexpr (!std::is_copy_constructible_v<Class> || !std::is_trivially_destructible_v<Class>) {
      auto call = [](auto &&...param) { return std::make_shared<Class>(std::forward<decltype(param)>(param)...); };

      return Proxy_Function(
          chaiscript::make_shared<dispatch::Proxy_Function_Base,
                                  dispatch::Proxy_Function_Callable_Impl<std::shared_ptr<Class>(Params...), decltype(call)>>(call));
    } else if constexpr (true) {
      auto call = [](auto &&...param) { return Class{std::forward<decltype(param)>(param)...}; };

      return Proxy_Function(
          chaiscript::make_shared<dispatch::Proxy_Function_Base, dispatch::Proxy_Function_Callable_Impl<Class(Params...), decltype(call)>>(
              call));
    }
  }
} // namespace chaiscript::dispatch::detail

namespace chaiscript {
  /// \brief Generates a constructor function for use with ChaiScript
  ///
  /// \tparam T The signature of the constructor to generate. In the form of: ClassType (ParamType1, ParamType2, ...)
  ///
  /// Example:
  /// \code
  ///    chaiscript::ChaiScript chai;
  ///    // Create a new function that creates a MyClass object using the (int, float) constructor
  ///    // and call that function "MyClass" so that it appears as a normal constructor to the user.
  ///    chai.add(constructor<MyClass (int, float)>(), "MyClass");
  /// \endcode
  template<typename T>
  Proxy_Function constructor() {
    T *f = nullptr;
    return (dispatch::detail::build_constructor_(f));
  }
} // namespace chaiscript

#endif
