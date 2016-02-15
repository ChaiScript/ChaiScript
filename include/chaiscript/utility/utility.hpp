// This file is distributed under the BSD License.
// See "license.txt" for details.
// Copyright 2009-2012, Jonathan Turner (jonathan@emptycrate.com)
// Copyright 2009-2016, Jason Turner (jason@emptycrate.com)
// http://www.chaiscript.com

#ifndef CHAISCRIPT_UTILITY_UTILITY_HPP_
#define CHAISCRIPT_UTILITY_UTILITY_HPP_

#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "../chaiscript.hpp"
#include "../dispatchkit/proxy_functions.hpp"
#include "../dispatchkit/type_info.hpp"
#include "../dispatchkit/operators.hpp"


namespace chaiscript 
{
  namespace utility
  {

    /// Single step command for registering a class with ChaiScript
    /// 
    /// \param[in,out] t_module Model to add class to
    /// \param[in] t_class_name Name of the class being registered
    /// \param[in] t_constructors Vector of constructors to add
    /// \param[in] t_funcs Vector of methods to add
    ///
    /// \example Adding a basic class to ChaiScript in one step
    /// 
    /// \code
    /// chaiscript::utility::add_class<test>(*m,
    ///      "test",
    ///      { constructor<test ()>(),
    ///        constructor<test (const test &)>() },
    ///      { {fun(&test::function), "function"},
    ///        {fun(&test::function2), "function2"},
    ///        {fun(&test::function3), "function3"},
    ///        {fun(static_cast<std::string(test::*)(double)>(&test::function_overload)), "function_overload" },
    ///        {fun(static_cast<std::string(test::*)(int)>(&test::function_overload)), "function_overload" },
    ///        {fun(static_cast<test & (test::*)(const test &)>(&test::operator=)), "=" }
    ///        }
    ///      );
    /// 
    template<typename Class, typename ModuleType>
      void add_class(ModuleType &t_module,
          const std::string &t_class_name,
          const std::vector<chaiscript::Proxy_Function> &t_constructors,
          const std::vector<std::pair<chaiscript::Proxy_Function, std::string>> &t_funcs)
      {
        t_module.add(chaiscript::user_type<Class>(), t_class_name); 

        for(const chaiscript::Proxy_Function &ctor: t_constructors)
        {
          t_module.add(ctor, t_class_name);
        }

        for(const auto &fun: t_funcs)
        {
          t_module.add(fun.first, fun.second);
        }
      }

    template<typename Enum, typename ModuleType>
      typename std::enable_if<std::is_enum<Enum>::value, void>::type
      add_class(ModuleType &t_module,
        const std::string &t_class_name,
#ifdef CHAISCRIPT_GCC_4_6
        const std::vector<std::pair<int, std::string>> &t_constants
#else
        const std::vector<std::pair<typename std::underlying_type<Enum>::type, std::string>> &t_constants
#endif
        )
      {
        t_module.add(chaiscript::user_type<Enum>(), t_class_name);

        t_module.add(chaiscript::constructor<Enum ()>(), t_class_name);
        t_module.add(chaiscript::constructor<Enum (const Enum &)>(), t_class_name);

        using namespace chaiscript::bootstrap::operators;
        t_module.add([](){
              // add some comparison and assignment operators
              return assign<Enum>(not_equal<Enum>(equal<Enum>()));
            }());

#ifdef CHAISCRIPT_GCC_4_6
        t_module.add(chaiscript::fun([](const Enum &e, const int &i) { return e == i; }), "==");
        t_module.add(chaiscript::fun([](const int &i, const Enum &e) { return i == e; }), "==");
#else
        t_module.add(chaiscript::fun([](const Enum &e, const typename std::underlying_type<Enum>::type &i) { return e == i; }), "==");
        t_module.add(chaiscript::fun([](const typename std::underlying_type<Enum>::type &i, const Enum &e) { return i == e; }), "==");
#endif

        for (const auto &constant : t_constants)
        {
          t_module.add_global_const(chaiscript::const_var(Enum(constant.first)), constant.second);
        }
      }
  }
}

#endif

