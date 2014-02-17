// This file is distributed under the BSD License.
// See "license.txt" for details.
// Copyright 2009-2012, Jonathan Turner (jonathan@emptycrate.com)
// Copyright 2009-2014, Jason Turner (jason@emptycrate.com)
// http://www.chaiscript.com

#include <boost/preprocessor.hpp>

#define addparam(z,n,text)  params.push_back((boost::is_reference<Param ## n>::value&&!(boost::is_same<chaiscript::Boxed_Value, typename boost::remove_const<typename boost::remove_reference<Param ## n>::type>::type>::value))?Boxed_Value(boost::ref(BOOST_PP_CAT(p, n))):Boxed_Value(BOOST_PP_CAT(p, n) ));
#define curry(z,n,text)  BOOST_PP_CAT(_, BOOST_PP_INC(n))


#ifndef  BOOST_PP_IS_ITERATING
#ifndef CHAISCRIPT_FUNCTION_CALL_DETAIL_HPP_
#define CHAISCRIPT_FUNCTION_CALL_DETAIL_HPP_

#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <string>
#include <vector>
#include "proxy_functions.hpp"

namespace chaiscript
{
  namespace dispatch
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
          static Ret call(const std::vector<Const_Proxy_Function> &t_funcs, 
              const std::vector<Boxed_Value> &params, const Dynamic_Cast_Conversions &t_conversions)
          {
            return boxed_cast<Ret>(dispatch::dispatch(t_funcs, params, t_conversions));
          }
        };

      /**
       * Specialization for void return types
       */
      template<>
        struct Function_Caller_Ret<void>
        {
          static void call(const std::vector<Const_Proxy_Function> &t_funcs, 
              const std::vector<Boxed_Value> &params, const Dynamic_Cast_Conversions &t_conversions)
          {
            dispatch::dispatch(t_funcs, params, t_conversions);
          }
        };
    }
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
  namespace dispatch
  {
    namespace detail
    {
      /**
       * used internally for unwrapping a function call's types
       */
      template<typename Ret BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM_PARAMS(n, typename Param) >
        Ret function_caller(const std::vector<Const_Proxy_Function> &funcs, const Dynamic_Cast_Conversions &t_conversions 
            BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM_BINARY_PARAMS(n, Param, p) )
        {
          std::vector<Boxed_Value> params;

          BOOST_PP_REPEAT(n, addparam, ~);

          return Function_Caller_Ret<Ret>::call(funcs, params, t_conversions);
        }

      /**
       * used internally for unwrapping a function call's types
       */
      template<typename Ret BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM_PARAMS(n, typename Param) >
        boost::function<Ret (BOOST_PP_ENUM_PARAMS(n, Param)) > 
        build_function_caller_helper(Ret (BOOST_PP_ENUM_PARAMS(n, Param)), const std::vector<Const_Proxy_Function> &funcs, 
            const Dynamic_Cast_Conversions *t_conversions)
        {
          if (funcs.size() == 1)
          {
            boost::shared_ptr<const Proxy_Function_Impl<Ret (BOOST_PP_ENUM_PARAMS(n, Param))> > pfi = 
              boost::dynamic_pointer_cast<const Proxy_Function_Impl<Ret (BOOST_PP_ENUM_PARAMS(n, Param))> >
              (funcs[0]);

            if (pfi)
            {
              return pfi->internal_function();
            } 
            // looks like this either wasn't a Proxy_Function_Impl or the types didn't match
            // we cannot make any other guesses or assumptions really, so continuing
          }

          return boost::bind(&function_caller<Ret BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM_PARAMS(n, Param)>, funcs, (t_conversions?*t_conversions:Dynamic_Cast_Conversions())
              BOOST_PP_ENUM_TRAILING(n, curry, ~));
        }
    }
  }
}
#undef n
#undef addparam
#undef curry 

#endif

