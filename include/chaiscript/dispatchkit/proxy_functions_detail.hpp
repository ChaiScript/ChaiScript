// This file is distributed under the BSD License.
// See "license.txt" for details.
// Copyright 2009-2012, Jonathan Turner (jonathan@emptycrate.com)
// and Jason Turner (jason@emptycrate.com)
// http://www.chaiscript.com

#ifndef CHAISCRIPT_PROXY_FUNCTIONS_DETAIL_HPP_
#define CHAISCRIPT_PROXY_FUNCTIONS_DETAIL_HPP_

#include "boxed_value.hpp"
#include "boxed_cast.hpp"
#include "type_info.hpp"
#include "handle_return.hpp"
#include <string>

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

      virtual ~arity_error() noexcept {}

      int got;
      int expected;
    };
  }

  namespace dispatch
  {
    namespace detail
    {
      /**
       * Used by Proxy_Function_Impl to return a list of all param types
       * it contains.
       */
      template<typename Ret, typename ... Params>
        std::vector<Type_Info> build_param_type_list(Ret (*)(Params...))
        {
          return std::vector<Type_Info> { chaiscript::detail::Get_Type_Info<Ret>::get(),
              chaiscript::detail::Get_Type_Info<Params>::get()... };
        }


      // Forward declaration
      template<typename ... Rest> 
        struct Try_Cast; 

      template<typename Param, typename ... Rest>
        struct Try_Cast<Param, Rest...>
        {
          static void do_try(const std::vector<Boxed_Value> &params, int generation, const Dynamic_Cast_Conversions &t_conversions)
          {
            boxed_cast<Param>(params[generation], &t_conversions);
            Try_Cast<Rest...>::do_try(params, generation+1, t_conversions);
          }
        };

      // 0th case
      template<>
        struct Try_Cast<>
        {
          static void do_try(const std::vector<Boxed_Value> &, int, const Dynamic_Cast_Conversions &)
          {
          }
        };
      


      /**
       * Used by Proxy_Function_Impl to determine if it is equivalent to another
       * Proxy_Function_Impl object. This function is primarly used to prevent
       * registration of two functions with the exact same signatures
       */
      template<typename Ret, typename ... Params>
        bool compare_types_cast(Ret (*)(Params...),
             const std::vector<Boxed_Value> &params, const Dynamic_Cast_Conversions &t_conversions)
       {
          try {
            Try_Cast<Params...>::do_try(params, 0, t_conversions);
          } catch (const exception::bad_boxed_cast &) {
            return false;
          }

          return true;
        }

      template<typename Ret, int count, typename ... Params>
        struct Call_Func
        {

          template<typename ... InnerParams>
          static Ret do_call(const std::function<Ret (Params...)> &f,
              const std::vector<Boxed_Value> &params, const Dynamic_Cast_Conversions &t_conversions, InnerParams &&... innerparams)
          {
            return Call_Func<Ret, count - 1, Params...>::do_call(f, params, t_conversions, std::forward<InnerParams>(innerparams)..., params[sizeof...(Params) - count]);
          } 
        };

      template<typename Ret, typename ... Params>
        struct Call_Func<Ret, 0, Params...>
        {
          template<typename ... InnerParams>
            static Ret do_call(const std::function<Ret (Params...)> &f,
                const std::vector<Boxed_Value> &, const Dynamic_Cast_Conversions &t_conversions, 	InnerParams &&... innerparams)
            {
              return f(boxed_cast<Params>(std::forward<InnerParams>(innerparams), &t_conversions)...);
            }
        };

      /**
       * Used by Proxy_Function_Impl to perform typesafe execution of a function.
       * The function attempts to unbox each paramter to the expected type.
       * if any unboxing fails the execution of the function fails and
       * the bad_boxed_cast is passed up to the caller.
       */
      template<typename Ret, typename ... Params>
        Ret call_func(const std::function<Ret (Params...)> &f,
            const std::vector<Boxed_Value> &params, const Dynamic_Cast_Conversions &t_conversions)
        {
          if (params.size() == sizeof...(Params))
          {
            return Call_Func<Ret, sizeof...(Params), Params...>::do_call(f, params, t_conversions);
          }

          throw exception::arity_error(static_cast<int>(params.size()), sizeof...(Params));
        }
          
    }
  }

}


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
          static Boxed_Value go(const std::function<Fun> &fun, const std::vector<Boxed_Value> &params, const Dynamic_Cast_Conversions &t_conversions)
          {
            return Handle_Return<Ret>::handle(call_func(fun, params, t_conversions));
          }
      };

    template<>
      struct Do_Call<void>
      {
        template<typename Fun>
          static Boxed_Value go(const std::function<Fun> &fun, const std::vector<Boxed_Value> &params, const Dynamic_Cast_Conversions &t_conversions)
          {
            call_func(fun, params, t_conversions);
            return Handle_Return<void>::handle();
          }
      };
    }
  }
}

#endif
