// This file is distributed under the BSD License.
// See "license.txt" for details.
// Copyright 2009-2012, Jonathan Turner (jonathan@emptycrate.com)
// Copyright 2009-2014, Jason Turner (jason@emptycrate.com)
// http://www.chaiscript.com

#include <boost/preprocessor.hpp>
#include <boost/preprocessor/arithmetic/inc.hpp>

#define param(z,n,text) BOOST_PP_CAT(text, BOOST_PP_INC(n)) 

#ifndef  BOOST_PP_IS_ITERATING
#ifndef CHAISCRIPT_BIND_FIRST_HPP_
#define CHAISCRIPT_BIND_FIRST_HPP_

#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/ref.hpp>

#define BOOST_PP_ITERATION_LIMITS ( 0, 8 )
#define BOOST_PP_FILENAME_1 <chaiscript/dispatchkit/bind_first.hpp>

#include BOOST_PP_ITERATE()


# endif
#else
# define n BOOST_PP_ITERATION()
# define m BOOST_PP_INC(n)

namespace chaiscript
{
  namespace detail
  { 
    /// \brief Helper function for binding the first parameter of a class method pointer. Used in chaiscript::fun overloads
    ///        that take 1 or 2 parameters to pre-bind to the function.
    ///
    /// \param[in] f method pointer to bind
    /// \param[in] o object to bind as first parameter 
    /// \returns a new boost::function object with one fewer parameters than the function passed in. 
    template<typename Ret, typename O, typename Class BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM_PARAMS(n, typename Param) >
      boost::function<Ret (BOOST_PP_ENUM_PARAMS(n, Param))> 
      bind_first(Ret (Class::*f)(BOOST_PP_ENUM_PARAMS(n, Param)), const O &o)
      {
        return boost::bind(boost::mem_fn(f), o BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM(n, param, _));
      }

    /// \brief Helper function for binding the first parameter of a const class method pointer. Used in chaiscript::fun overloads
    ///        that take 1 or 2 parameters to pre-bind to the function.
    ///
    /// \param[in] f method pointer to bind
    /// \param[in] o object to bind as first parameter 
    /// \returns a new boost::function object with one fewer parameters than the function passed in. 
    template<typename Ret, typename O, typename Class BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM_PARAMS(n, typename Param) >
      boost::function<Ret (BOOST_PP_ENUM_PARAMS(n, Param))> 
      bind_first(Ret (Class::*f)(BOOST_PP_ENUM_PARAMS(n, Param)) const, const O &o)
      {
        return boost::bind(boost::mem_fn(f), o BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM(n, param, _));
      }

    /// \brief Helper function for binding the first parameter of a function pointer. Used in chaiscript::fun overloads
    ///        that take 1 or 2 parameters to pre-bind to the function.
    ///
    /// \param[in] f method pointer to bind
    /// \param[in] o object to bind as first parameter 
    /// \returns a new boost::function object with one fewer parameters than the function passed in. 
    template<typename Ret,typename O BOOST_PP_COMMA_IF(m) BOOST_PP_ENUM_PARAMS(m, typename Param) >
      boost::function<Ret (BOOST_PP_ENUM(n, param, Param))> 
      bind_first(Ret (*f)(BOOST_PP_ENUM_PARAMS(m, Param)), const O &o)
      {
        return boost::bind(f, o BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM(n, param, _));
      }

    /// \brief Helper function for binding the first parameter of a boost::function object. Used in chaiscript::fun overloads
    ///        that take 1 or 2 parameters to pre-bind to the function.
    ///
    /// \param[in] f method pointer to bind
    /// \param[in] o object to bind as first parameter 
    /// \returns a new boost::function object with one fewer parameters than the function passed in. 
    template<typename Ret,typename O BOOST_PP_COMMA_IF(m) BOOST_PP_ENUM_PARAMS(m, typename Param) >
      boost::function<Ret (BOOST_PP_ENUM(n, param, Param))> 
      bind_first(const boost::function<Ret (BOOST_PP_ENUM_PARAMS(m, Param))> &f, const O &o)
      {
        return boost::bind(f, o BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM(n, param, _));
      }

  }
}

#undef n
#undef m
#undef param

#endif
