// This file is distributed under the BSD License.
// See "license.txt" for details.
// Copyright 2009-2012, Jonathan Turner (jonathan@emptycrate.com)
// Copyright 2009-2018, Jason Turner (jason@emptycrate.com)
// http://www.chaiscript.com

// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#ifndef CHAISCRIPT_REGISTER_FUNCTION_HPP_
#define CHAISCRIPT_REGISTER_FUNCTION_HPP_

#include <type_traits>

#include "bind_first.hpp"
#include "function_signature.hpp"
#include "proxy_functions.hpp"

namespace chaiscript {
  namespace dispatch::detail {
    template<typename Obj, typename Param1, typename... Rest>
    Param1 get_first_param(Function_Params<Param1, Rest...>, Obj &&obj) {
      return static_cast<Param1>(std::forward<Obj>(obj));
    }

    template<typename Func, bool Is_Noexcept, bool Is_Member, bool Is_MemberObject, bool Is_Object, typename Ret, typename... Param>
    auto make_callable_impl(Func &&func, Function_Signature<Ret, Function_Params<Param...>, Is_Noexcept, Is_Member, Is_MemberObject, Is_Object>) {
      if constexpr (Is_MemberObject) {
        // we now that the Param pack will have only one element, so we are safe expanding it here
        return Proxy_Function(chaiscript::make_shared<dispatch::Proxy_Function_Base, dispatch::Attribute_Access<Ret, std::decay_t<Param>...>>(
            std::forward<Func>(func)));
      } else if constexpr (Is_Member) {
        // TODO some kind of bug is preventing forwarding of this noexcept for the lambda
        auto call = [func = std::forward<Func>(func)](auto &&obj, auto &&...param) /* noexcept(Is_Noexcept) */ -> decltype(auto) {
          return ((get_first_param(Function_Params<Param...>{}, obj).*func)(std::forward<decltype(param)>(param)...));
        };
        return Proxy_Function(
            chaiscript::make_shared<dispatch::Proxy_Function_Base, dispatch::Proxy_Function_Callable_Impl<Ret(Param...), decltype(call)>>(
                std::move(call)));
      } else {
        return Proxy_Function(
            chaiscript::make_shared<dispatch::Proxy_Function_Base, dispatch::Proxy_Function_Callable_Impl<Ret(Param...), std::decay_t<Func>>>(
                std::forward<Func>(func)));
      }
    }

    // this version peels off the function object itself from the function signature, when used
    // on a callable object
    template<typename Func, typename Ret, typename Object, typename... Param, bool Is_Noexcept>
    auto make_callable(Func &&func, Function_Signature<Ret, Function_Params<Object, Param...>, Is_Noexcept, false, false, true>) {
      return make_callable_impl(std::forward<Func>(func), Function_Signature<Ret, Function_Params<Param...>, Is_Noexcept, false, false, true>{});
    }

    template<typename Func, typename Ret, typename... Param, bool Is_Noexcept, bool Is_Member, bool Is_MemberObject>
    auto make_callable(Func &&func, Function_Signature<Ret, Function_Params<Param...>, Is_Noexcept, Is_Member, Is_MemberObject, false> fs) {
      return make_callable_impl(std::forward<Func>(func), fs);
    }
  } // namespace dispatch::detail

  /// \brief Creates a new Proxy_Function object from a free function, member function or data member
  /// \param[in] t Function / member to expose
  ///
  /// \b Example:
  /// \code
  /// int myfunction(const std::string &);
  /// class MyClass
  /// {
  ///   public:
  ///     void memberfunction();
  ///     int memberdata;
  /// };
  ///
  /// chaiscript::ChaiScript chai;
  /// chai.add(fun(&myfunction), "myfunction");
  /// chai.add(fun(&MyClass::memberfunction), "memberfunction");
  /// chai.add(fun(&MyClass::memberdata), "memberdata");
  /// \endcode
  ///
  /// \sa \ref adding_functions
  template<typename T>
  Proxy_Function fun(T &&t) {
    return dispatch::detail::make_callable(std::forward<T>(t), dispatch::detail::function_signature(t));
  }

  /// \brief Creates a new Proxy_Function object from a free function, member function or data member and binds the first parameter of it
  /// \param[in] t Function / member to expose
  /// \param[in] q Value to bind to first parameter
  ///
  /// \b Example:
  /// \code
  /// struct MyClass
  /// {
  ///   void memberfunction(int);
  /// };
  ///
  /// MyClass obj;
  /// chaiscript::ChaiScript chai;
  /// // Add function taking only one argument, an int, and permanently bound to "obj"
  /// chai.add(fun(&MyClass::memberfunction, std::ref(obj)), "memberfunction");
  /// \endcode
  ///
  /// \sa \ref adding_functions
  template<typename T, typename Q>
  Proxy_Function fun(T &&t, const Q &q) {
    return fun(detail::bind_first(std::forward<T>(t), q));
  }

} // namespace chaiscript

#endif
