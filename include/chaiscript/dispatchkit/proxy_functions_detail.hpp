// This file is distributed under the BSD License.
// See "license.txt" for details.
// Copyright 2009-2012, Jonathan Turner (jonathan@emptycrate.com)
// Copyright 2009-2018, Jason Turner (jason@emptycrate.com)
// http://www.chaiscript.com

// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#ifndef CHAISCRIPT_PROXY_FUNCTIONS_DETAIL_HPP_
#define CHAISCRIPT_PROXY_FUNCTIONS_DETAIL_HPP_

#include <array>
#include <functional>
#include <stdexcept>
#include <vector>

#include "../chaiscript_defines.hpp"
#include "boxed_cast.hpp"
#include "boxed_value.hpp"
#include "function_params.hpp"
#include "handle_return.hpp"
#include "type_info.hpp"

namespace chaiscript {
  class Type_Conversions_State;
  namespace exception {
    class bad_boxed_cast;
  } // namespace exception
} // namespace chaiscript

namespace chaiscript {
  namespace exception {
    /**
 * Exception thrown when there is a mismatch in number of
 * parameters during Proxy_Function execution
 */
    struct arity_error : std::range_error {
      arity_error(int t_got, int t_expected)
          : std::range_error("Function dispatch arity mismatch")
          , got(t_got)
          , expected(t_expected) {
      }

      arity_error(const arity_error &) = default;

      ~arity_error() noexcept override = default;

      int got;
      int expected;
    };
  } // namespace exception

  namespace dispatch {
    namespace detail {
      /**
 * Used by Proxy_Function_Impl to return a list of all param types
 * it contains.
 */
      template<typename Ret, typename... Params>
      std::vector<Type_Info> build_param_type_list(Ret (*)(Params...)) {
        /// \note somehow this is responsible for a large part of the code generation
        return {user_type<Ret>(), user_type<Params>()...};
      }

      /**
 * Used by Proxy_Function_Impl to determine if it is equivalent to another
 * Proxy_Function_Impl object. This function is primarily used to prevent
 * registration of two functions with the exact same signatures
 */
      template<typename Ret, typename... Params>
      bool compare_types_cast(Ret (*)(Params...), const chaiscript::Function_Params &params, const Type_Conversions_State &t_conversions) noexcept {
        try {
          std::vector<Boxed_Value>::size_type i = 0;
          (boxed_cast<Params>(params[i++], &t_conversions), ...);
          return true;
        } catch (const exception::bad_boxed_cast &) {
          return false;
        }
      }

      template<typename Callable, typename Ret, typename... Params, size_t... I>
      Ret call_func(Ret (*)(Params...),
                    std::index_sequence<I...>,
                    const Callable &f,
                    [[maybe_unused]] const chaiscript::Function_Params &params,
                    [[maybe_unused]] const Type_Conversions_State &t_conversions) {
        return f(boxed_cast<Params>(params[I], &t_conversions)...);
      }

      /// Used by Proxy_Function_Impl to perform typesafe execution of a function.
      /// The function attempts to unbox each parameter to the expected type.
      /// if any unboxing fails the execution of the function fails and
      /// the bad_boxed_cast is passed up to the caller.
      template<typename Callable, typename Ret, typename... Params>
      Boxed_Value
      call_func(Ret (*sig)(Params...), const Callable &f, const chaiscript::Function_Params &params, const Type_Conversions_State &t_conversions) {
        if constexpr (std::is_same_v<Ret, void>) {
          call_func(sig, std::index_sequence_for<Params...>{}, f, params, t_conversions);
          return Handle_Return<void>::handle();
        } else {
          return Handle_Return<Ret>::handle(call_func(sig, std::index_sequence_for<Params...>{}, f, params, t_conversions));
        }
      }

    } // namespace detail
  } // namespace dispatch

} // namespace chaiscript

#endif
