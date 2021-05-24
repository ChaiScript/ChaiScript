// This file is distributed under the BSD License.
// See "license.txt" for details.
// Copyright 2009-2012, Jonathan Turner (jonathan@emptycrate.com)
// Copyright 2009-2018, Jason Turner (jason@emptycrate.com)
// http://www.chaiscript.com

// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

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
#include "proxy_functions.hpp"
#include "type_conversions.hpp"

namespace chaiscript::dispatch::detail {
  /// used internally for unwrapping a function call's types
  template<typename Ret, typename... Param>
  struct Build_Function_Caller_Helper {
    Build_Function_Caller_Helper(std::vector<Const_Proxy_Function> t_funcs, const Type_Conversions *t_conversions)
        : m_funcs(std::move(t_funcs))
        , m_conversions(t_conversions) {
    }

    Ret call(const chaiscript::Function_Params &params, const Type_Conversions_State &t_state) {
      if constexpr (std::is_arithmetic_v<Ret> && !std::is_same_v<std::remove_cv_t<std::remove_reference_t<Ret>>, bool>) {
        return Boxed_Number(dispatch::dispatch(m_funcs, params, t_state)).get_as<Ret>();
      } else if constexpr (std::is_same_v<void, Ret>) {
        dispatch::dispatch(m_funcs, params, t_state);
      } else {
        return boxed_cast<Ret>(dispatch::dispatch(m_funcs, params, t_state), &t_state);
      }
    }

    template<typename... P>
    Ret operator()(P &&...param) {
      std::array<Boxed_Value, sizeof...(P)> params{box<P>(std::forward<P>(param))...};

      if (m_conversions) {
        Type_Conversions_State state(*m_conversions, m_conversions->conversion_saves());
        return call(chaiscript::Function_Params{params}, state);
      } else {
        Type_Conversions conv;
        Type_Conversions_State state(conv, conv.conversion_saves());
        return call(chaiscript::Function_Params{params}, state);
      }
    }

    template<typename P, typename Q>
    static Boxed_Value box(Q &&q) {
      if constexpr (std::is_same_v<chaiscript::Boxed_Value, std::decay_t<Q>>) {
        return std::forward<Q>(q);
      } else if constexpr (std::is_reference_v<P>) {
        return Boxed_Value(std::ref(std::forward<Q>(q)));
      } else {
        return Boxed_Value(std::forward<Q>(q));
      }
    }

    std::vector<Const_Proxy_Function> m_funcs;
    const Type_Conversions *m_conversions;
  };

  /// \todo what happens if t_conversions is deleted out from under us?!
  template<typename Ret, typename... Params>
  std::function<Ret(Params...)>
  build_function_caller_helper(Ret(Params...), const std::vector<Const_Proxy_Function> &funcs, const Type_Conversions_State *t_conversions) {
    /*
    if (funcs.size() == 1)
    {
      std::shared_ptr<const Proxy_Function_Impl<Ret (Params...)>> pfi =
        std::dynamic_pointer_cast<const Proxy_Function_Impl<Ret (Params...)> >
          (funcs[0]);

      if (pfi)
      {
        return pfi->internal_function();
      }
      // looks like this either wasn't a Proxy_Function_Impl or the types didn't match
      // we cannot make any other guesses or assumptions really, so continuing
    }
  */

    return std::function<Ret(Params...)>(Build_Function_Caller_Helper<Ret, Params...>(funcs, t_conversions ? t_conversions->get() : nullptr));
  }
} // namespace chaiscript::dispatch::detail

#endif
