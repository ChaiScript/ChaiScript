// This file is distributed under the BSD License.
// See "license.txt" for details.
// Copyright 2009-2012, Jonathan Turner (jonathan@emptycrate.com)
// Copyright 2009-2016, Jason Turner (jason@emptycrate.com)
// http://www.chaiscript.com

#ifndef CHAISCRIPT_FUNCTION_CALL_DETAIL_HPP_
#define CHAISCRIPT_FUNCTION_CALL_DETAIL_HPP_

#include <functional>
#include <memory>
#include <string>
#include <type_traits>
#include <vector>

#include "boxed_cast.hpp"
#include "boxed_number.hpp"
#include "boxed_value.hpp"
#include "type_conversions.hpp"
#include "proxy_functions.hpp"

namespace chaiscript
{
  namespace dispatch
  {
    namespace detail
    {
      /// Internal helper class for handling the return
      /// value of a build_function_caller
      template<typename Ret, bool is_arithmetic>
        struct Function_Caller_Ret
        {
          static Ret call(const std::vector<Const_Proxy_Function> &t_funcs, 
              const std::vector<Boxed_Value> &params, const Type_Conversions_State *t_conversions)
          {
            if (t_conversions) {
              return boxed_cast<Ret>(dispatch::dispatch(t_funcs, params, *t_conversions), t_conversions);
            } else {
              Type_Conversions conv;
              Type_Conversions_State state(conv, conv.conversion_saves());
              return boxed_cast<Ret>(dispatch::dispatch(t_funcs, params, state), t_conversions);
            }
          }
        };

      /**
       * Specialization for arithmetic return types
       */
      template<typename Ret>
        struct Function_Caller_Ret<Ret, true>
        {
          static Ret call(const std::vector<Const_Proxy_Function> &t_funcs, 
              const std::vector<Boxed_Value> &params, const Type_Conversions_State *t_conversions)
          {
            if (t_conversions) {
              return Boxed_Number(dispatch::dispatch(t_funcs, params, *t_conversions)).get_as<Ret>();
            } else {
              Type_Conversions conv;
              Type_Conversions_State state(conv, conv.conversion_saves());
              return Boxed_Number(dispatch::dispatch(t_funcs, params, state)).get_as<Ret>();
            }
          }
        };


      /**
       * Specialization for void return types
       */
      template<>
        struct Function_Caller_Ret<void, false>
        {
          static void call(const std::vector<Const_Proxy_Function> &t_funcs, 
              const std::vector<Boxed_Value> &params, const Type_Conversions_State *t_conversions)
          {
            if (t_conversions) {
              dispatch::dispatch(t_funcs, params, *t_conversions);
            } else {
              Type_Conversions conv;
              Type_Conversions_State state(conv, conv.conversion_saves());
              dispatch::dispatch(t_funcs, params, state);
            }
          }
        };

      /**
       * used internally for unwrapping a function call's types
       */
      template<typename Ret, typename ... Param>
        struct Build_Function_Caller_Helper
        {
          Build_Function_Caller_Helper(std::vector<Const_Proxy_Function> t_funcs, const Type_Conversions *t_conversions)
            : m_funcs(std::move(t_funcs)),
              m_conversions(t_conversions)
          {
          }

          template<typename ... P>
          Ret operator()(P&&  ...  param)
          {
            if (m_conversions) {
              Type_Conversions_State state(*m_conversions, m_conversions->conversion_saves());
              return Function_Caller_Ret<Ret, std::is_arithmetic<Ret>::value && !std::is_same<Ret, bool>::value>::call(m_funcs, { 
                  box<P>(std::forward<P>(param))...
                  }, &state
                  );
            } else {
              return Function_Caller_Ret<Ret, std::is_arithmetic<Ret>::value && !std::is_same<Ret, bool>::value>::call(m_funcs, { 
                  box<P>(std::forward<P>(param))...
                  }, nullptr
                  );
            }

          }

          template<typename P, typename Q>
          static auto box(Q&& q) -> typename std::enable_if<std::is_reference<P>::value&&!std::is_same<chaiscript::Boxed_Value, typename std::remove_const<typename std::remove_reference<P>::type>::type>::value, Boxed_Value>::type
          {
            return Boxed_Value(std::ref(std::forward<Q>(q)));
          }

          template<typename P, typename Q>
          static auto box(Q&& q) -> typename std::enable_if<!std::is_reference<P>::value&&!std::is_same<chaiscript::Boxed_Value, typename std::remove_const<typename std::remove_reference<P>::type>::type>::value, Boxed_Value>::type
          {
            return Boxed_Value(std::forward<Q>(q));
          }

          template<typename P>
          static Boxed_Value box(Boxed_Value bv)
          {
            return bv;
          }


          std::vector<Const_Proxy_Function> m_funcs;
          const Type_Conversions *m_conversions;
        };



      /// \todo what happens if t_conversions is deleted out from under us?!
      template<typename Ret, typename ... Params>
        std::function<Ret (Params...)> build_function_caller_helper(Ret (Params...), const std::vector<Const_Proxy_Function> &funcs, const Type_Conversions_State *t_conversions)
        {
          return std::function<Ret (Params...)>(Build_Function_Caller_Helper<Ret, Params...>(funcs, t_conversions?t_conversions->get():nullptr));
        }
    }
  }
}

#endif

