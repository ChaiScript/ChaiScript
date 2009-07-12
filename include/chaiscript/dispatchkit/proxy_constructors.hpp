// This file is distributed under the BSD License.
// See LICENSE.TXT for details.
// Copyright 2009, Jonathan Turner (jonathan.d.turner@gmail.com) 
// and Jason Turner (lefticus@gmail.com)
// http://www.chaiscript.com

#include <boost/preprocessor.hpp>

#ifndef  BOOST_PP_IS_ITERATING
#ifndef __proxy_constructors_hpp__
#define __proxy_constructors_hpp__

#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>


#define BOOST_PP_ITERATION_LIMITS ( 0, 10 )
#define BOOST_PP_FILENAME_1 <chaiscript/dispatchkit/proxy_constructors.hpp>
#include BOOST_PP_ITERATE()
# endif
#else
# define n BOOST_PP_ITERATION()

namespace dispatchkit
{
  /**
   * A constructor function, used for creating a new object
   * of a given type with a given set of params
   */
  template<typename Class BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM_PARAMS(n, typename Param) >
    boost::shared_ptr<Class> constructor( BOOST_PP_ENUM_BINARY_PARAMS(n, Param, p) )
    {
      return boost::shared_ptr<Class>(new Class( BOOST_PP_ENUM_PARAMS(n, p) ));
    }

  /**
   * Helper function for build a constructor function
   * example:
   * dispatchengine.register_function(build_constructor<MyClass, int, const std::string&>, "MyClass");
   * \todo See if it is possible to make this not be a variadic function
   */
  template<typename Class BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM_PARAMS(n, typename Param) >
    boost::function<boost::shared_ptr<Class> (BOOST_PP_ENUM_PARAMS(n, Param))> build_constructor()
    {
      typedef boost::shared_ptr<Class> (*func)(BOOST_PP_ENUM_PARAMS(n, Param));
      return boost::function<boost::shared_ptr<Class> (BOOST_PP_ENUM_PARAMS(n, Param))>(func(&(constructor<Class BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM_PARAMS(n, Param)>)));
    }
}

#endif

