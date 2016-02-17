// This file is distributed under the BSD License.
// See "license.txt" for details.
// Copyright 2009-2012, Jonathan Turner (jonathan@emptycrate.com)
// Copyright 2009-2016, Jason Turner (jason@emptycrate.com)
// http://www.chaiscript.com

#ifndef CHAISCRIPT_REGISTER_FUNCTION_HPP_
#define CHAISCRIPT_REGISTER_FUNCTION_HPP_

#include <type_traits>

#include "bind_first.hpp"
#include "proxy_functions.hpp"

namespace chaiscript
{

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
    Proxy_Function fun(const T &t)
    {
      typedef typename dispatch::detail::Callable_Traits<T>::Signature Signature;

      return Proxy_Function(
          chaiscript::make_shared<dispatch::Proxy_Function_Base, dispatch::Proxy_Function_Callable_Impl<Signature, T>>(t));
    }

  template<typename Ret, typename ... Param>
    Proxy_Function fun(Ret (*func)(Param...))
    {
      auto fun_call = dispatch::detail::Fun_Caller<Ret, Param...>(func);

      return Proxy_Function(
          chaiscript::make_shared<dispatch::Proxy_Function_Base, dispatch::Proxy_Function_Callable_Impl<Ret (Param...), decltype(fun_call)>>(fun_call));

    }

  template<typename Ret, typename Class, typename ... Param>
    Proxy_Function fun(Ret (Class::*t_func)(Param...) const)
    {
      auto call = dispatch::detail::Const_Caller<Ret, Class, Param...>(t_func);

      return Proxy_Function(
          chaiscript::make_shared<dispatch::Proxy_Function_Base, dispatch::Proxy_Function_Callable_Impl<Ret (const Class &, Param...), decltype(call)>>(call));
    }

  template<typename Ret, typename Class, typename ... Param>
    Proxy_Function fun(Ret (Class::*t_func)(Param...))
    {
      auto call = dispatch::detail::Caller<Ret, Class, Param...>(t_func);

      return Proxy_Function(
          chaiscript::make_shared<dispatch::Proxy_Function_Base, dispatch::Proxy_Function_Callable_Impl<Ret (Class &, Param...), decltype(call)>>(call));

    }


  template<typename T, typename Class /*, typename = typename std::enable_if<std::is_member_object_pointer<T>::value>::type*/>
    Proxy_Function fun(T Class::* m /*, typename std::enable_if<std::is_member_object_pointer<T>::value>::type* = 0*/ )
    {
      return Proxy_Function(chaiscript::make_shared<dispatch::Proxy_Function_Base, dispatch::Attribute_Access<T, Class>>(m));
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
    Proxy_Function fun(T &&t, const Q &q)
    {
      return fun(detail::bind_first(std::forward<T>(t), q));
    }

  /// \brief Creates a new Proxy_Function object from a free function or member function and binds the first and second parameters of it
  /// \param[in] t Function / member to expose
  /// \param[in] q Value to bind to first parameter
  /// \param[in] r Value to bind to second parameter
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
  /// // Add function taking only no arguments, and permanently bound to "obj" and "1"
  /// // memberfunction() will be equivalent to obj.memberfunction(1)
  /// chai.add(fun(&MyClass::memberfunction, std::ref(obj), 1), "memberfunction"); 
  /// \endcode
  /// 
  /// \sa \ref adding_functions
  template<typename T, typename Q, typename R>
    Proxy_Function fun(T &&t, Q &&q, R &&r)
    {
      return fun(detail::bind_first(detail::bind_first(std::forward<T>(t), std::forward<Q>(q)), std::forward<R>(r)));
    }

}


#endif

