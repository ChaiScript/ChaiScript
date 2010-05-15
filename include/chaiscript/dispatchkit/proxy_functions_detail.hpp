// This file is distributed under the BSD License.
// See "license.txt" for details.
// Copyright 2009-2010, Jonathan Turner (jturner@minnow-lang.org)
// and Jason Turner (lefticus@gmail.com)
// http://www.chaiscript.com

#include <boost/preprocessor.hpp>

#define gettypeinfo(z,n,text)  ti.push_back(detail::Get_Type_Info<Param ## n>::get());
#define casthelper(z,n,text) BOOST_PP_COMMA_IF(n) chaiscript::boxed_cast< Param ## n >(params[n])
#define trycast(z,n,text) chaiscript::boxed_cast<Param ## n>(params[n]);

#ifndef  BOOST_PP_IS_ITERATING
#ifndef __proxy_functions_detail_hpp__
#define __proxy_functions_detail_hpp__

#include "boxed_value.hpp"
#include "type_info.hpp"
#include "handle_return.hpp"
#include <string>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>

#include <stdexcept>
#include <vector>

namespace chaiscript
{
  /**
   * Exception thrown when there is a mismatch in number of
   * parameters during Proxy_Function execution
   */
  struct arity_error : std::range_error
  {
    arity_error(int t_got, int t_expected)
      : std::range_error("Function dispatch arity mismatch"),
      got(t_got), expected(t_expected)
    {
    }

    virtual ~arity_error() throw() {}
    int got;
    int expected;
  };

  template<typename Ret>
    struct Do_Call
    {
      template<typename Fun>
      static Boxed_Value go(const boost::function<Fun> &fun, const std::vector<Boxed_Value> &params)
      {
        return Handle_Return<Ret>::handle(call_func(fun, params));
      }
    };

  template<>
    struct Do_Call<void>
    {
      template<typename Fun>
      static Boxed_Value go(const boost::function<Fun> &fun, const std::vector<Boxed_Value> &params)
      {
        call_func(fun, params);
        return Handle_Return<void>::handle();
      };
    };
}

#define BOOST_PP_ITERATION_LIMITS ( 0, 10 )
#define BOOST_PP_FILENAME_1 <chaiscript/dispatchkit/proxy_functions_detail.hpp>
#include BOOST_PP_ITERATE()


# endif
#else
# define n BOOST_PP_ITERATION()

namespace chaiscript
{
  /**
   * Used by Proxy_Function_Impl to return a list of all param types
   * it contains.
   */
  template<typename Ret BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM_PARAMS(n, typename Param) >
    std::vector<Type_Info> build_param_type_list(Ret (*)(BOOST_PP_ENUM_PARAMS(n, Param)))
    {
      std::vector<Type_Info> ti;
      ti.push_back(detail::Get_Type_Info<Ret>::get());

      BOOST_PP_REPEAT(n, gettypeinfo, ~) 

      return ti;
    }

  /**
   * Used by Proxy_Function_Impl to perform typesafe execution of a function.
   * The function attempts to unbox each paramter to the expected type.
   * if any unboxing fails the execution of the function fails and
   * the bad_boxed_cast is passed up to the caller.
   */
  template<typename Ret BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM_PARAMS(n, typename Param)>
    Ret call_func(const boost::function<Ret (BOOST_PP_ENUM_PARAMS(n, Param))> &f,
        const std::vector<Boxed_Value> &params)
    {
      if (params.size() != n)
      {
        throw arity_error(params.size(), n);
      } else {
        return f(BOOST_PP_REPEAT(n, casthelper, ~));
      }
    }

  /**
   * Used by Proxy_Function_Impl to determine if it is equivalent to another
   * Proxy_Function_Impl object. This function is primarly used to prevent
   * registration of two functions with the exact same signatures
   */
  template<typename Ret BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM_PARAMS(n, typename Param)>
    bool compare_types_cast(Ret (*)(BOOST_PP_ENUM_PARAMS(n, Param)),
        const std::vector<Boxed_Value> & BOOST_PP_IF(n, params, ))
    {
      try {
        BOOST_PP_REPEAT(n, trycast, ~);
      } catch (const bad_boxed_cast &) {
        return false;
      }

      return true;
    }

}

#undef n

#endif
