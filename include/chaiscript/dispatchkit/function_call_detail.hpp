// This file is distributed under the BSD License.
// See "license.txt" for details.
// Copyright 2009-2010, Jonathan Turner (jturner@minnow-lang.org)
// and Jason Turner (lefticus@gmail.com)
// http://www.chaiscript.com

#include <boost/preprocessor.hpp>

#define addparam(z,n,text)  params.push_back(boost::is_reference<Param ## n>::value?Boxed_Value(boost::ref(BOOST_PP_CAT(p, n))):Boxed_Value(BOOST_PP_CAT(p, n) ));
#define curry(z,n,text)  BOOST_PP_CAT(_, BOOST_PP_INC(n))


#ifndef  BOOST_PP_IS_ITERATING
#ifndef __function_call_detail_hpp__
#define __function_call_detail_hpp__

#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <string>
#include <vector>
#include "proxy_functions.hpp"

namespace chaiscript
{
  namespace detail
  {
    /**
     * Internal helper class for handling the return
     * value of a build_function_caller
     */
    template<typename Ret>
      struct Function_Caller_Ret
      {
        static Ret call(const std::vector<std::pair<std::string, Proxy_Function > > &t_funcs, 
            const std::vector<Boxed_Value> &params)
        {
          return boxed_cast<Ret>(dispatch(t_funcs, params));
        }
      };

    /**
     * Specialization for void return types
     */
    template<>
      struct Function_Caller_Ret<void>
      {
        static void call(const std::vector<std::pair<std::string, Proxy_Function > > &t_funcs, 
            const std::vector<Boxed_Value> &params)
        {
          dispatch(t_funcs, params);
        }
      };
  }
}

#define BOOST_PP_ITERATION_LIMITS ( 0, 9 )
#define BOOST_PP_FILENAME_1 <chaiscript/dispatchkit/function_call_detail.hpp>
#include BOOST_PP_ITERATE()

# endif
#else
# define n BOOST_PP_ITERATION()

namespace chaiscript
{
  namespace detail
  {
    /**
     * used internally for unwrapping a function call's types
     */
    template<typename Ret BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM_PARAMS(n, typename Param) >
      Ret function_caller(const std::vector<std::pair<std::string, Proxy_Function > > &funcs 
          BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM_BINARY_PARAMS(n, Param, p) )
      {
        std::vector<Boxed_Value> params;

        BOOST_PP_REPEAT(n, addparam, ~)

          return Function_Caller_Ret<Ret>::call(funcs, params);
      }

    /**
     * used internally for unwrapping a function call's types
     */
    template<typename Ret BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM_PARAMS(n, typename Param) >
      boost::function<Ret (BOOST_PP_ENUM_PARAMS(n, Param)) > 
      build_function_caller_helper(Ret (BOOST_PP_ENUM_PARAMS(n, Param)), const std::vector<std::pair<std::string, Proxy_Function> > &funcs)
      {
        return boost::bind(&function_caller<Ret BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM_PARAMS(n, Param)>, funcs
            BOOST_PP_ENUM_TRAILING(n, curry, ~));
      }

  }
}
#undef n

#endif

