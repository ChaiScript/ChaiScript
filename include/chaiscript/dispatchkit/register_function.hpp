// This file is distributed under the BSD License.
// See "license.txt" for details.
// Copyright 2009, Jonathan Turner (jturner@minnow-lang.org)
// and Jason Turner (lefticus@gmail.com)
// http://www.chaiscript.com

#include <boost/preprocessor.hpp>

#ifndef  BOOST_PP_IS_ITERATING
#ifndef __register_function_hpp__
#define __register_function_hpp__

#include "dispatchkit.hpp"
#include <boost/function.hpp>
#include <boost/bind.hpp>

namespace dispatchkit
{
  /**
   * Helper function for register_member function
   */
  template<typename T, typename Class>
    T &get_member(T Class::* m, Class *obj)
    {
      return obj->*m;
    }

  /**
   * Automatically create a get_member helper function for an object 
   * to allow for runtime dispatched access to public data members
   * for example, the case of std::pair<>::first and std::pair<>::second
   */
  template<typename T, typename Class>
    void register_member(Dispatch_Engine &s, T Class::* m, const std::string &name)
    {
      s.register_function(boost::function<T (Class *)>(boost::bind(&get_member<T, Class>, m, _1)), name);
    }
}

#define BOOST_PP_ITERATION_LIMITS ( 0, 10 )
#define BOOST_PP_FILENAME_1 <chaiscript/dispatchkit/register_function.hpp>
#include BOOST_PP_ITERATE()

# endif
#else
# define n BOOST_PP_ITERATION()

namespace dispatchkit
{
  /**
   * Register a global function of n parameters with name 
   */
  template<typename Ret BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM_PARAMS(n, typename Param)>
    void register_function(Dispatch_Engine &s, Ret (*f)(BOOST_PP_ENUM_PARAMS(n, Param)), const std::string &name)
    {
      s.register_function(boost::function<Ret (BOOST_PP_ENUM_PARAMS(n, Param))>(f), name);
    }

  /**
   * Register a class method of n parameters with name 
   */
  template<typename Ret, typename Class BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM_PARAMS(n, typename Param)>
    void register_function(Dispatch_Engine &s, Ret (Class::*f)(BOOST_PP_ENUM_PARAMS(n, Param)), const std::string &name)
    {
      s.register_function(boost::function<Ret (Class* BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM_PARAMS(n, Param))>(f), name);
    }

  /**
   * Register a const class method of n parameters with name 
   */
  template<typename Ret, typename Class BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM_PARAMS(n, typename Param)>
    void register_function(Dispatch_Engine &s, Ret (Class::*f)(BOOST_PP_ENUM_PARAMS(n, Param))const, const std::string &name)
    {
      s.register_function(boost::function<Ret (const Class* BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM_PARAMS(n, Param))>(f), name);
    }
}

#endif

