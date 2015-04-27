// This file is distributed under the BSD License.
// See "license.txt" for details.
// Copyright 2009-2012, Jonathan Turner (jonathan@emptycrate.com)
// Copyright 2009-2015, Jason Turner (jason@emptycrate.com)
// http://www.chaiscript.com

#ifndef CHAISCRIPT_REGISTER_FUNCTION_HPP_
#define CHAISCRIPT_REGISTER_FUNCTION_HPP_

#include <functional>
#include <type_traits>

#include "bind_first.hpp"
#include "proxy_functions.hpp"

namespace chaiscript
{
  namespace dispatch
  {
    namespace detail
    {
      template<typename T>
        struct FunctionSignature
        {
        };

      template<typename Sig>
        struct FunctionSignature<std::function<Sig> >
        {
          typedef Sig Signature;
        };

      template<typename Ret, typename ... Args> 
        std::function<Ret (Args...) > to_function(Ret (*func)(Args...))
        {
          return std::function<Ret (Args...)>(func);
        }

      template<typename Ret, typename Class, typename ... Args> 
        std::function<Ret (Class &, Args...) > to_function(Ret (Class::*func)(Args...))
        {
#ifdef CHAISCRIPT_MSVC
          /// \todo this std::mem_fn wrap shouldn't be necessary but type conversions for 
          ///       std::function for member function pointers seems to be broken in MSVC
          return std::function<Ret(Class &, Args...)>(std::mem_fn(func));
#else
          return std::function<Ret(Class &, Args...)>(func);
#endif
        }

      template<typename Ret, typename Class, typename ... Args> 
        std::function<Ret (const Class &, Args...) > to_function(Ret (Class::*func)(Args...) const)
        {
#if defined(CHAISCRIPT_MSVC) || defined(CHAISCRIPT_LIBCPP)
          /// \todo this std::mem_fn wrap shouldn't be necessary but type conversions for 
          ///       std::function for member function pointers seems to be broken in MSVC
          return std::function<Ret (const Class &, Args...)>([func](const Class &o, Args... args)->Ret {
                return (o.*func)(std::forward<Args>(args)...);
              });
#else
          return std::function<Ret(const Class &, Args...)>(func);
#endif
        }

    }
  }

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
      return Proxy_Function(
          chaiscript::make_shared<dispatch::Proxy_Function_Base, dispatch::Proxy_Function_Impl<typename dispatch::detail::FunctionSignature<decltype(dispatch::detail::to_function(t)) >::Signature>>(dispatch::detail::to_function(t)));
    }

  template<typename Ret, typename Class, typename ... Param>
    Proxy_Function fun(Ret (Class::*func)(Param...) const)
    {
      return Proxy_Function(
          chaiscript::make_shared<dispatch::Proxy_Function_Base, dispatch::Proxy_Function_Impl<typename dispatch::detail::FunctionSignature<decltype(dispatch::detail::to_function(func)) >::Signature>>(dispatch::detail::to_function(func)));
    }

  template<typename Ret, typename Class, typename ... Param>
    Proxy_Function fun(Ret (Class::*func)(Param...))
    {
      return Proxy_Function(
          chaiscript::make_shared<dispatch::Proxy_Function_Base, dispatch::Proxy_Function_Impl<typename dispatch::detail::FunctionSignature<decltype(dispatch::detail::to_function(func)) >::Signature>>(dispatch::detail::to_function(func)));
    }


  template<typename T, typename Class /*, typename = typename std::enable_if<std::is_member_object_pointer<T>::value>::type*/>
    Proxy_Function fun(T Class::* m /*, typename std::enable_if<std::is_member_object_pointer<T>::value>::type* = 0*/ )
    {
      return Proxy_Function(chaiscript::make_shared<dispatch::Proxy_Function_Base, dispatch::Attribute_Access<T, Class>>(m));
    }


  /// \brief Creates a new Proxy_Function object from a std::function object
  /// \param[in] f std::function to expose to ChaiScript
  ///
  /// \b Example:
  /// \code
  /// std::function<int (char, float, std::string)> f = get_some_function();
  /// chaiscript::ChaiScript chai;
  /// chai.add(fun(f), "some_function");
  /// \endcode
  /// 
  /// \sa \ref adding_functions
  template<typename T>
    Proxy_Function fun(const std::function<T> &f)
    {
      return Proxy_Function(chaiscript::make_shared<dispatch::Proxy_Function_Base, dispatch::Proxy_Function_Impl<T>>(f));
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

