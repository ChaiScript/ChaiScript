// This file is distributed under the BSD License.
// See "license.txt" for details.
// Copyright 2009, Jonathan Turner (jturner@minnow-lang.org)
// and Jason Turner (lefticus@gmail.com)
// http://www.chaiscript.com

#include <boost/preprocessor.hpp>
#include <boost/preprocessor/arithmetic/inc.hpp>

#define param(z,n,text) BOOST_PP_CAT(_, BOOST_PP_INC(n)) 

#ifndef  BOOST_PP_IS_ITERATING
#ifndef __bind_first_hpp__
#define __bind_first_hpp__

#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/ref.hpp>

#define BOOST_PP_ITERATION_LIMITS ( 0, 8 )
#define BOOST_PP_FILENAME_1 <chaiscript/dispatchkit/bind_first.hpp>

#include BOOST_PP_ITERATE()


# endif
#else
# define n BOOST_PP_ITERATION()

namespace chaiscript
{
  
  template<typename Ret, typename Class BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM_PARAMS(n, typename Param) >
    boost::function<Ret (BOOST_PP_ENUM_PARAMS(n, Param))> 
      bind_first(Ret (Class::*f)(BOOST_PP_ENUM_PARAMS(n, Param)), boost::reference_wrapper<Class> &o)
      {
        return boost::bind(f, o BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM(n, param, ~));
      }

  template<typename Ret, typename Class BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM_PARAMS(n, typename Param) >
    boost::function<Ret (BOOST_PP_ENUM_PARAMS(n, Param))> 
      bind_first(Ret (Class::*f)(BOOST_PP_ENUM_PARAMS(n, Param)) const, boost::reference_wrapper<Class> &o)
      {
        return boost::bind(f, o BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM(n, param, ~));
      }

  template<typename Ret, typename Class BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM_PARAMS(n, typename Param) >
    boost::function<Ret (BOOST_PP_ENUM_PARAMS(n, Param))> 
      bind_first(Ret (Class::*f)(BOOST_PP_ENUM_PARAMS(n, Param)), Class *o)
      {
        return boost::bind(f, o BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM(n, param, ~));
      }

  template<typename Ret, typename Class BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM_PARAMS(n, typename Param) >
    boost::function<Ret (BOOST_PP_ENUM_PARAMS(n, Param))> 
      bind_first(Ret (Class::*f)(BOOST_PP_ENUM_PARAMS(n, Param))const, Class *o)
      {
        return boost::bind(f, o BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM(n, param, ~));
      }


  template<typename Ret, typename Class BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM_PARAMS(n, typename Param) >
    boost::function<Ret (BOOST_PP_ENUM_PARAMS(n, Param))> 
      bind_first(Ret (Class::*f)(BOOST_PP_ENUM_PARAMS(n, Param)), boost::shared_ptr<Class> o)
      {
        return boost::bind(f, o BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM(n, param, ~));
      }

  template<typename Ret, typename Class BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM_PARAMS(n, typename Param) >
    boost::function<Ret (BOOST_PP_ENUM_PARAMS(n, Param))> 
      bind_first(Ret (Class::*f)(BOOST_PP_ENUM_PARAMS(n, Param))const, boost::shared_ptr<Class> o)
      {
        return boost::bind(f, o BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM(n, param, ~));
      }


}

#endif
