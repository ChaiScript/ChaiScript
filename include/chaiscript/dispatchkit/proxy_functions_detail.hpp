// This file is distributed under the BSD License.
// See "license.txt" for details.
// Copyright 2009-2012, Jonathan Turner (jonathan@emptycrate.com)
// Copyright 2009-2014, Jason Turner (jason@emptycrate.com)
// http://www.chaiscript.com

#include <boost/preprocessor.hpp>

#define gettypeinfo(z,n,text)  ti.push_back(chaiscript::detail::Get_Type_Info<Param ## n>::get());
#define casthelper(z,n,text) BOOST_PP_COMMA_IF(n) chaiscript::boxed_cast< Param ## n >(params[n], &t_conversions)
#define trycast(z,n,text) chaiscript::boxed_cast<Param ## n>(params[n], &t_conversions);

#ifndef  BOOST_PP_IS_ITERATING
#ifndef CHAISCRIPT_PROXY_FUNCTIONS_DETAIL_HPP_
#define CHAISCRIPT_PROXY_FUNCTIONS_DETAIL_HPP_

#include "boxed_value.hpp"
#include "boxed_cast.hpp"
#include "type_info.hpp"
#include "handle_return.hpp"
#include <string>
#include <boost/function.hpp>
#include <boost/bind.hpp>

#include <stdexcept>
#include <vector>

namespace chaiscript
{
  namespace exception
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
  }

}

#define BOOST_PP_ITERATION_LIMITS ( 0, 10 )
#define BOOST_PP_FILENAME_1 <chaiscript/dispatchkit/proxy_functions_detail.hpp>
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
       * Used by Proxy_Function_Impl to return a list of all param types
       * it contains.
       */
      template<typename Ret BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM_PARAMS(n, typename Param) >
        std::vector<Type_Info> build_param_type_list(Ret (*)(BOOST_PP_ENUM_PARAMS(n, Param)))
        {
          std::vector<Type_Info> ti;
          ti.push_back(chaiscript::detail::Get_Type_Info<Ret>::get());

          BOOST_PP_REPEAT(n, gettypeinfo, ~) 

          return ti;
        }

      /**
       * Used by Proxy_Function_Impl to perform typesafe execution of a function.
       * The function attempts to unbox each paramter to the expected type.
       * if any unboxing fails the execution of the function fails and
       * the bad_boxed_cast is passed up to the caller.
       */
#ifdef BOOST_MSVC
#pragma warning(push)
#pragma warning(disable : 4100)
#endif

#ifdef __llvm__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
#endif

      template<typename Ret BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM_PARAMS(n, typename Param)>
        Ret call_func(const boost::function<Ret (BOOST_PP_ENUM_PARAMS(n, Param))> &f,
            const std::vector<Boxed_Value> &params, const Dynamic_Cast_Conversions & BOOST_PP_IF(n, t_conversions, BOOST_PP_EMPTY))

#ifdef __llvm__
#pragma clang diagnostic pop
#endif
        {
          if (params.size() != n)
          {
            throw exception::arity_error(static_cast<int>(params.size()), n);
          } else {
            return f(BOOST_PP_REPEAT(n, casthelper, ~));
          }
        }

#ifdef BOOST_MSVC
#pragma warning(pop)
#endif

      /**
       * Used by Proxy_Function_Impl to determine if it is equivalent to another
       * Proxy_Function_Impl object. This function is primarly used to prevent
       * registration of two functions with the exact same signatures
       */

#ifdef BOOST_MSVC
#pragma warning(push)
#pragma warning(disable : 4100)
#endif

#ifdef __llvm__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
#endif

       template<typename Ret BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM_PARAMS(n, typename Param)>
        bool compare_types_cast(Ret (*)(BOOST_PP_ENUM_PARAMS(n, Param)),
            const std::vector<Boxed_Value> & BOOST_PP_IF(n, params, BOOST_PP_EMPTY), const Dynamic_Cast_Conversions &t_conversions)


#ifdef __llvm__
#pragma clang diagnostic pop
#endif
        {
          try {
            (void)t_conversions;
            BOOST_PP_REPEAT(n, trycast, ~);
          } catch (const exception::bad_boxed_cast &) {
            return false;
          }

          return true;
        }

#ifdef BOOST_MSVC
#pragma warning(pop)
#endif

    }
  }
}

#undef n
#undef gettypeinfo
#undef casthelper
#undef trycast


#endif


#ifndef BOOST_PP_IS_ITERATING

namespace chaiscript
{
  namespace dispatch
  {
    namespace detail
    {
    template<typename Ret>
      struct Do_Call
      {
        template<typename Fun>
          static Boxed_Value go(const boost::function<Fun> &fun, const std::vector<Boxed_Value> &params, const Dynamic_Cast_Conversions &t_conversions)
          {
            return Handle_Return<Ret>::handle(call_func(fun, params, t_conversions));
          }
      };

    template<>
      struct Do_Call<void>
      {
        template<typename Fun>
          static Boxed_Value go(const boost::function<Fun> &fun, const std::vector<Boxed_Value> &params, const Dynamic_Cast_Conversions &t_conversions)
          {
            call_func(fun, params, t_conversions);
            return Handle_Return<void>::handle();
          }
      };
    }
  }
}

#endif
