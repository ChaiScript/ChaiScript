// This file is distributed under the BSD License.
// See "license.txt" for details.
// Copyright 2009-2012, Jonathan Turner (jonathan@emptycrate.com)
// Copyright 2009-2014, Jason Turner (jason@emptycrate.com)
// http://www.chaiscript.com

#ifndef CHAISCRIPT_OPERATORS_HPP_
#define CHAISCRIPT_OPERATORS_HPP_

#include "../chaiscript_defines.hpp"

namespace chaiscript 
{
  namespace bootstrap
  {
    namespace operators
    {
      template<typename Ret, typename L, typename R>
        Ret assign(L l, R r)
        {
          return (l = r);
        }

      template<typename Ret, typename L, typename R>
        Ret assign_bitwise_and(L l, R r)
        {
          return (l &= r);
        }

      template<typename Ret, typename L, typename R>
        Ret assign_xor(L l, R r)
        {
          return (l ^= r);
        }

      template<typename Ret, typename L, typename R>
        Ret assign_bitwise_or(L l, R r)
        {
          return (l |= r);
        }

      template<typename Ret, typename L, typename R>
        Ret assign_difference(L l, R r)
        {
          return (l -= r);
        }

      template<typename Ret, typename L, typename R>
        Ret assign_left_shift(L l, R r)
        {
          return (l <<= r);
        }

      template<typename Ret, typename L, typename R>
        Ret assign_product(L l, R r)
        {
          return (l *= r);
        }

      template<typename Ret, typename L, typename R>
        Ret assign_quotient(L l, R r)
        {
          return (l /= r);
        }

      template<typename Ret, typename L, typename R>
        Ret assign_remainder(L l, R r)
        {
          return (l %= r);
        }

      template<typename Ret, typename L, typename R>
        Ret assign_right_shift(L l, R r)
        {
          return (l >>= r);
        }

      template<typename Ret, typename L, typename R>
        Ret assign_sum(L l, R r)
        {
          return (l += r);
        }

      template<typename Ret, typename L>
        Ret prefix_decrement(L l)
        {
          return (--l);
        }

      template<typename Ret, typename L>
        Ret prefix_increment(L l)
        {
          return (++l);
        }

      template<typename Ret, typename L, typename R>
        Ret equal(L l, R r)
        {
          return (l == r);
        }

      template<typename Ret, typename L, typename R>
        Ret greater_than(L l, R r)
        {
          return (l > r);
        }

      template<typename Ret, typename L, typename R>
        Ret greater_than_equal(L l, R r)
        {
          return (l >= r);
        }

      template<typename Ret, typename L, typename R>
        Ret less_than(L l, R r)
        {
          return (l < r);
        }

      template<typename Ret, typename L, typename R>
        Ret less_than_equal(L l, R r)
        {
          return (l <= r);
        }

      template<typename Ret, typename L>
        Ret logical_compliment(L l)
        {
          return (!l);
        }

      template<typename Ret, typename L, typename R>
        Ret not_equal(L l, R r)
        {
          return (l != r);
        }

      template<typename Ret, typename L, typename R>
        Ret addition(L l, R r)
        {
          return (l + r);
        }

      template<typename Ret, typename L>
        Ret unary_plus(L l)
        {
          return (+l);
        }

      template<typename Ret, typename L, typename R>
        Ret subtraction(L l, R r)
        {
          return (l - r);
        }

      template<typename Ret, typename L>
        Ret unary_minus(L l)
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

      template<typename Ret, typename L, typename R>
        Ret bitwise_and(L l, R r)
        {
          return (l & r);
        }

      template<typename Ret, typename L>
        Ret bitwise_compliment(L l)
        {
          return (~l);
        }

      template<typename Ret, typename L, typename R>
        Ret bitwise_xor(L l, R r)
        {
          return (l ^ r);
        }

      template<typename Ret, typename L, typename R>
        Ret bitwise_or(L l, R r)
        {
          return (l | r);
        }

      template<typename Ret, typename L, typename R>
        Ret division(L l, R r)
        {
          return (l / r);
        }

      template<typename Ret, typename L, typename R>
        Ret left_shift(L l, R r)
        {
          return l << r;
        }

      template<typename Ret, typename L, typename R>
        Ret multiplication(L l, R r)
        {
          return l * r;
        }

      template<typename Ret, typename L, typename R>
        Ret remainder(L l, R r)
        {
          return (l % r);
        }

      template<typename Ret, typename L, typename R>
        Ret right_shift(L l, R r)
        {
          return (l >> r);
        }




      template<typename T>
        ModulePtr assign(ModulePtr m = ModulePtr(new Module()))
        {
          m->add(fun(&assign<T &, T &, const T&>), "=");
          return m;
        }

      template<typename T>
        ModulePtr assign_bitwise_and(ModulePtr m = ModulePtr(new Module()))
        {
          m->add(fun(&assign_bitwise_and<T &, T &, const T&>), "&=");
          return m;
        }

      template<typename T>
        ModulePtr assign_xor(ModulePtr m = ModulePtr(new Module()))
        {
          m->add(fun(&assign_xor<T &, T &, const T&>), "^=");
          return m;
        }

      template<typename T>
        ModulePtr assign_bitwise_or(ModulePtr m = ModulePtr(new Module()))
        {
          m->add(fun(&assign_bitwise_or<T &, T &, const T&>), "|=");
          return m;
        }

      template<typename T>
        ModulePtr assign_difference(ModulePtr m = ModulePtr(new Module()))
        {
          m->add(fun(&assign_difference<T &, T &, const T&>), "-=");
          return m;
        }

      template<typename T>
        ModulePtr assign_left_shift(ModulePtr m = ModulePtr(new Module()))
        {
          m->add(fun(&assign_left_shift<T &, T &, const T&>), "<<=");
          return m;
        }

      template<typename T>
        ModulePtr assign_product(ModulePtr m = ModulePtr(new Module()))
        {
          m->add(fun(&assign_product<T &, T &, const T&>), "*=");
          return m;
        }

      template<typename T>
        ModulePtr assign_quotient(ModulePtr m = ModulePtr(new Module()))
        {
          m->add(fun(&assign_quotient<T &, T &, const T&>), "/=");
          return m;
        }

      template<typename T>
        ModulePtr assign_remainder(ModulePtr m = ModulePtr(new Module()))
        {
          m->add(fun(&assign_remainder<T &, T &, const T&>), "%=");
          return m;
        }

      template<typename T>
        ModulePtr assign_right_shift(ModulePtr m = ModulePtr(new Module()))
        {
          m->add(fun(&assign_right_shift<T &, T &, const T&>), ">>=");
          return m;
        }

      template<typename T>
        ModulePtr assign_sum(ModulePtr m = ModulePtr(new Module()))
        {
          m->add(fun(&assign_sum<T &, T &, const T&>), "+=");
          return m;
        }

      template<typename T>
        ModulePtr prefix_decrement(ModulePtr m = ModulePtr(new Module()))
        {
          m->add(fun(&prefix_decrement<T &, T &>), "--");
          return m;
        }

      template<typename T>
        ModulePtr prefix_increment(ModulePtr m = ModulePtr(new Module()))
        {
          m->add(fun(&prefix_increment<T &, T &>), "++");
          return m;
        }

      template<typename T>
        ModulePtr equal(ModulePtr m = ModulePtr(new Module()))
        {
          m->add(fun(&equal<bool, const T&, const T&>), "==");
          return m;
        }

      template<typename T>
        ModulePtr greater_than(ModulePtr m = ModulePtr(new Module()))
        {
          m->add(fun(&greater_than<bool, const T&, const T&>), ">");
          return m;
        }

      template<typename T>
        ModulePtr greater_than_equal(ModulePtr m = ModulePtr(new Module()))
        {
          m->add(fun(&greater_than_equal<bool, const T&, const T&>), ">=");
          return m;
        }

      template<typename T>
        ModulePtr less_than(ModulePtr m = ModulePtr(new Module()))
        {
          m->add(fun(&less_than<bool, const T&, const T&>), "<");
          return m;
        }

      template<typename T>
        ModulePtr less_than_equal(ModulePtr m = ModulePtr(new Module()))
        {
          m->add(fun(&less_than_equal<bool, const T&, const T&>), "<=");
          return m;
        }

      template<typename T>
        ModulePtr logical_compliment(ModulePtr m = ModulePtr(new Module()))
        {
          m->add(fun(&logical_compliment<bool, const T &>), "!");
          return m;
        }

      template<typename T>
        ModulePtr not_equal(ModulePtr m = ModulePtr(new Module()))
        {
          m->add(fun(&not_equal<bool, const T &, const T &>), "!=");
          return m;
        }

      template<typename T>
        ModulePtr addition(ModulePtr m = ModulePtr(new Module()))
        {
          m->add(fun(&addition<T, const T &, const T &>), "+");
          return m;
        }

      template<typename T>
        ModulePtr unary_plus(ModulePtr m = ModulePtr(new Module()))
        {
          m->add(fun(&unary_plus<T, const T &>), "+");
          return m;
        }

      template<typename T>
        ModulePtr subtraction(ModulePtr m = ModulePtr(new Module()))
        {
          m->add(fun(&subtraction<T, const T &, const T &>), "-");
          return m;
        }

      template<typename T>
        ModulePtr unary_minus(ModulePtr m = ModulePtr(new Module()))
        {
          m->add(fun(&unary_minus<T, const T &>), "-");
          return m;
        }

      template<typename T>
        ModulePtr bitwise_and(ModulePtr m = ModulePtr(new Module()))
        {
          m->add(fun(&bitwise_and<T, const T &, const T &>), "&");
          return m;
        }

      template<typename T>
        ModulePtr bitwise_compliment(ModulePtr m = ModulePtr(new Module()))
        {
          m->add(fun(&bitwise_compliment<T, const T &>), "~");
          return m;
        }

      template<typename T>
        ModulePtr bitwise_xor(ModulePtr m = ModulePtr(new Module()))
        {
          m->add(fun(&bitwise_xor<T, const T &, const T &>), "^");
          return m;
        }

      template<typename T>
        ModulePtr bitwise_or(ModulePtr m = ModulePtr(new Module()))
        {
          m->add(fun(&bitwise_or<T, const T &, const T &>), "|");
          return m;
        }

      template<typename T>
        ModulePtr division(ModulePtr m = ModulePtr(new Module()))
        {
          m->add(fun(&division<T, const T &, const T &>), "/");
          return m;
        }

      template<typename T>
        ModulePtr left_shift(ModulePtr m = ModulePtr(new Module()))
        {
          m->add(fun(&left_shift<T, const T &, const T &>), "<<");
          return m;
        }

      template<typename T>
        ModulePtr multiplication(ModulePtr m = ModulePtr(new Module()))
        {
          m->add(fun(&multiplication<T, const T &, const T &>), "*");
          return m;
        }

      template<typename T>
        ModulePtr remainder(ModulePtr m = ModulePtr(new Module()))
        {
          m->add(fun(&remainder<T, const T &, const T &>), "%");
          return m;
        }

      template<typename T>
        ModulePtr right_shift(ModulePtr m = ModulePtr(new Module()))
        {
          m->add(fun(&right_shift<T, const T &, const T &>), ">>");
          return m;
        }
    }
  }
}

#endif
