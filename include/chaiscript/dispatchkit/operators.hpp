// This file is distributed under the BSD License.
// See "license.txt" for details.
// Copyright 2009-2012, Jonathan Turner (jonathan@emptycrate.com)
// Copyright 2009-2016, Jason Turner (jason@emptycrate.com)
// http://www.chaiscript.com

#ifndef CHAISCRIPT_OPERATORS_HPP_
#define CHAISCRIPT_OPERATORS_HPP_

#include "../chaiscript_defines.hpp"
#include "register_function.hpp"

namespace chaiscript 
{
  namespace bootstrap
  {
    namespace operators
    {
      namespace detail
      {
        /// \todo make this return a decltype once we drop gcc 4.6
        template<typename L, typename R>
          auto assign(L l, R r) -> L&
          {
            return (l = r);
          }

        template<typename L, typename R>
          auto assign_bitwise_and(L l, R r) -> decltype((l &= r))
          {
            return (l &= r);
          }

        template<typename L, typename R>
          auto assign_xor(L l, R r) -> decltype((l^=r))
          {
            return (l ^= r);
          }

        template<typename L, typename R>
          auto assign_bitwise_or(L l, R r) -> decltype((l |= r))
          {
            return (l |= r);
          }

        template<typename L, typename R>
          auto assign_difference(L l, R r) -> decltype(( l -= r))
          {
            return (l -= r);
          }

        template<typename L, typename R>
          auto assign_left_shift(L l, R r) -> decltype(( l <<= r))
          {
            return (l <<= r);
          }

        template<typename L, typename R>
          auto assign_product(L l, R r) -> decltype(( l *= r ))
          {
            return (l *= r);
          }

        template<typename L, typename R>
          auto assign_quotient(L l, R r) -> decltype(( l /= r ))
          {
            return (l /= r);
          }

        template<typename L, typename R>
          auto assign_remainder(L l, R r) -> decltype(( l %= r ))
          {
            return (l %= r);
          }

        template<typename L, typename R>
          auto assign_right_shift(L l, R r) -> decltype(( l >>= r))
          {
            return (l >>= r);
          }

        /// \todo make this return a decltype once we drop gcc 4.6
        template<typename L, typename R>
          auto assign_sum(L l, R r) -> L&
          {
            return (l += r);
          }

        template<typename L>
          auto prefix_decrement(L l) -> decltype(( --l ))
          {
            return (--l);
          }

        template<typename L>
          auto prefix_increment(L l) -> decltype(( ++l ))
          {
            return (++l);
          }

        template<typename L, typename R>
          auto equal(L l, R r) -> decltype(( l == r ))
          {
            return (l == r);
          }

        template<typename L, typename R>
          auto greater_than(L l, R r) -> decltype(( l > r ))
          {
            return (l > r);
          }

        template<typename L, typename R>
          auto greater_than_equal(L l, R r) -> decltype(( l >= r ))
          {
            return (l >= r);
          }

        template<typename L, typename R>
          auto less_than(L l, R r) -> decltype(( l < r ))
          {
            return (l < r);
          }

        template<typename L, typename R>
          auto less_than_equal(L l, R r) -> decltype(( l <= r ))
          {
            return (l <= r);
          }

        template<typename L>
          auto logical_compliment(L l) -> decltype(( !l ))
          {
            return (!l);
          }

        template<typename L, typename R>
          auto not_equal(L l, R r) -> decltype(( l != r ))
          {
            return (l != r);
          }

        template<typename L, typename R>
          auto addition(L l, R r) -> decltype(( l + r ))
          {
            return (l + r);
          }

        template<typename L>
          auto unary_plus(L l) -> decltype(( +l ))
          {
            return (+l);
          }

        template<typename L, typename R>
          auto subtraction(L l, R r) -> decltype(( l - r ))
          {
            return (l - r);
          }

        template<typename L>
          auto unary_minus(L l) -> decltype(( -l ))
          {
#ifdef CHAISCRIPT_MSVC
#pragma warning(push)
#pragma warning(disable : 4146)
            return (-l);
#pragma warning(pop)
#else
            return (-l);
#endif
          }

        template<typename L, typename R>
          auto bitwise_and(L l, R r) -> decltype(( l & r ))
          {
            return (l & r);
          }

        template<typename L>
          auto bitwise_compliment(L l) -> decltype(( ~l ))
          {
            return (~l);
          }

        template<typename L, typename R>
          auto bitwise_xor(L l, R r) -> decltype(( l ^ r ))
          {
            return (l ^ r);
          }

        template<typename L, typename R>
          auto bitwise_or(L l, R r) -> decltype(( l | r ))
          {
            return (l | r);
          }

        template<typename L, typename R>
          auto division(L l, R r) -> decltype(( l / r ))
          {
            return (l / r);
          }

        template<typename L, typename R>
          auto left_shift(L l, R r) -> decltype(( l << r ))
          {
            return l << r;
          }

        template<typename L, typename R>
          auto multiplication(L l, R r) -> decltype(( l * r ))
          {
            return l * r;
          }

        template<typename L, typename R>
          auto remainder(L l, R r) -> decltype(( l % r ))
          {
            return (l % r);
          }

        template<typename L, typename R>
          auto right_shift(L l, R r) -> decltype(( l >> r ))
          {
            return (l >> r);
          }
      }



      template<typename T>
        void assign(Module& m)
        {
          m.add(chaiscript::fun(&detail::assign<T &, const T&>), "=");
        }

      template<typename T>
        void assign_bitwise_and(Module& m)
        {
          m.add(chaiscript::fun(&detail::assign_bitwise_and<T &, const T&>), "&=");
        }

      template<typename T>
        void assign_xor(Module& m)
        {
          m.add(chaiscript::fun(&detail::assign_xor<T &, const T&>), "^=");
        }

      template<typename T>
        void assign_bitwise_or(Module& m)
        {
          m.add(chaiscript::fun(&detail::assign_bitwise_or<T &, const T&>), "|=");
        }

      template<typename T>
        void assign_difference(Module& m)
        {
          m.add(chaiscript::fun(&detail::assign_difference<T &, const T&>), "-=");
        }

      template<typename T>
        void assign_left_shift(Module& m)
        {
          m.add(chaiscript::fun(&detail::assign_left_shift<T &, const T&>), "<<=");
        }

      template<typename T>
        void assign_product(Module& m)
        {
          m.add(chaiscript::fun(&detail::assign_product<T &, const T&>), "*=");
        }

      template<typename T>
        void assign_quotient(Module& m)
        {
          m.add(chaiscript::fun(&detail::assign_quotient<T &, const T&>), "/=");
        }

      template<typename T>
        void assign_remainder(Module& m)
        {
          m.add(chaiscript::fun(&detail::assign_remainder<T &, const T&>), "%=");
        }

      template<typename T>
        void assign_right_shift(Module& m)
        {
          m.add(chaiscript::fun(&detail::assign_right_shift<T &, const T&>), ">>=");
        }

      template<typename T>
        void assign_sum(Module& m)
        {
          m.add(chaiscript::fun(&detail::assign_sum<T &, const T&>), "+=");
        }

      template<typename T>
        void prefix_decrement(Module& m)
        {
          m.add(chaiscript::fun(&detail::prefix_decrement<T &>), "--");
        }

      template<typename T>
        void prefix_increment(Module& m)
        {
          m.add(chaiscript::fun(&detail::prefix_increment<T &>), "++");
        }

      template<typename T>
        void equal(Module& m)
        {
          m.add(chaiscript::fun(&detail::equal<const T&, const T&>), "==");
        }

      template<typename T>
        void greater_than(Module& m)
        {
          m.add(chaiscript::fun(&detail::greater_than<const T&, const T&>), ">");
        }

      template<typename T>
        void greater_than_equal(Module& m)
        {
          m.add(chaiscript::fun(&detail::greater_than_equal<const T&, const T&>), ">=");
        }

      template<typename T>
        void less_than(Module& m)
        {
          m.add(chaiscript::fun(&detail::less_than<const T&, const T&>), "<");
        }

      template<typename T>
        void less_than_equal(Module& m)
        {
          m.add(chaiscript::fun(&detail::less_than_equal<const T&, const T&>), "<=");
        }

      template<typename T>
        void logical_compliment(Module& m)
        {
          m.add(chaiscript::fun(&detail::logical_compliment<const T &>), "!");
        }

      template<typename T>
        void not_equal(Module& m)
        {
          m.add(chaiscript::fun(&detail::not_equal<const T &, const T &>), "!=");
        }

      template<typename T>
        void addition(Module& m)
        {
          m.add(chaiscript::fun(&detail::addition<const T &, const T &>), "+");
        }

      template<typename T>
        void unary_plus(Module& m)
        {
          m.add(chaiscript::fun(&detail::unary_plus<const T &>), "+");
        }

      template<typename T>
        void subtraction(Module& m)
        {
          m.add(chaiscript::fun(&detail::subtraction<const T &, const T &>), "-");
        }

      template<typename T>
        void unary_minus(Module& m)
        {
          m.add(chaiscript::fun(&detail::unary_minus<const T &>), "-");
        }

      template<typename T>
        void bitwise_and(Module& m)
        {
          m.add(chaiscript::fun(&detail::bitwise_and<const T &, const T &>), "&");
        }

      template<typename T>
        void bitwise_compliment(Module& m)
        {
          m.add(chaiscript::fun(&detail::bitwise_compliment<const T &>), "~");
        }

      template<typename T>
        void bitwise_xor(Module& m)
        {
          m.add(chaiscript::fun(&detail::bitwise_xor<const T &, const T &>), "^");
        }

      template<typename T>
        void bitwise_or(Module& m)
        {
          m.add(chaiscript::fun(&detail::bitwise_or<const T &, const T &>), "|");
        }

      template<typename T>
        void division(Module& m)
        {
          m.add(chaiscript::fun(&detail::division<const T &, const T &>), "/");
        }

      template<typename T>
        void left_shift(Module& m)
        {
          m.add(chaiscript::fun(&detail::left_shift<const T &, const T &>), "<<");
        }

      template<typename T>
        void multiplication(Module& m)
        {
          m.add(chaiscript::fun(&detail::multiplication<const T &, const T &>), "*");
        }

      template<typename T>
        void remainder(Module& m)
        {
          m.add(chaiscript::fun(&detail::remainder<const T &, const T &>), "%");
        }

      template<typename T>
        void right_shift(Module& m)
        {
          m.add(chaiscript::fun(&detail::right_shift<const T &, const T &>), ">>");
        }
    }
  }
}

#endif
