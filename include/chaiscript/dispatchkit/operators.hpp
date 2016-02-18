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
        ModulePtr assign(ModulePtr m = std::make_shared<Module>())
        {
          m->add(chaiscript::fun(&detail::assign<T &, const T&>), "=");
          return m;
        }

      template<typename T>
        ModulePtr assign_bitwise_and(ModulePtr m = std::make_shared<Module>())
        {
          m->add(chaiscript::fun(&detail::assign_bitwise_and<T &, const T&>), "&=");
          return m;
        }

      template<typename T>
        ModulePtr assign_xor(ModulePtr m = std::make_shared<Module>())
        {
          m->add(chaiscript::fun(&detail::assign_xor<T &, const T&>), "^=");
          return m;
        }

      template<typename T>
        ModulePtr assign_bitwise_or(ModulePtr m = std::make_shared<Module>())
        {
          m->add(chaiscript::fun(&detail::assign_bitwise_or<T &, const T&>), "|=");
          return m;
        }

      template<typename T>
        ModulePtr assign_difference(ModulePtr m = std::make_shared<Module>())
        {
          m->add(chaiscript::fun(&detail::assign_difference<T &, const T&>), "-=");
          return m;
        }

      template<typename T>
        ModulePtr assign_left_shift(ModulePtr m = std::make_shared<Module>())
        {
          m->add(chaiscript::fun(&detail::assign_left_shift<T &, const T&>), "<<=");
          return m;
        }

      template<typename T>
        ModulePtr assign_product(ModulePtr m = std::make_shared<Module>())
        {
          m->add(chaiscript::fun(&detail::assign_product<T &, const T&>), "*=");
          return m;
        }

      template<typename T>
        ModulePtr assign_quotient(ModulePtr m = std::make_shared<Module>())
        {
          m->add(chaiscript::fun(&detail::assign_quotient<T &, const T&>), "/=");
          return m;
        }

      template<typename T>
        ModulePtr assign_remainder(ModulePtr m = std::make_shared<Module>())
        {
          m->add(chaiscript::fun(&detail::assign_remainder<T &, const T&>), "%=");
          return m;
        }

      template<typename T>
        ModulePtr assign_right_shift(ModulePtr m = std::make_shared<Module>())
        {
          m->add(chaiscript::fun(&detail::assign_right_shift<T &, const T&>), ">>=");
          return m;
        }

      template<typename T>
        ModulePtr assign_sum(ModulePtr m = std::make_shared<Module>())
        {
          m->add(chaiscript::fun(&detail::assign_sum<T &, const T&>), "+=");
          return m;
        }

      template<typename T>
        ModulePtr prefix_decrement(ModulePtr m = std::make_shared<Module>())
        {
          m->add(chaiscript::fun(&detail::prefix_decrement<T &>), "--");
          return m;
        }

      template<typename T>
        ModulePtr prefix_increment(ModulePtr m = std::make_shared<Module>())
        {
          m->add(chaiscript::fun(&detail::prefix_increment<T &>), "++");
          return m;
        }

      template<typename T>
        ModulePtr equal(ModulePtr m = std::make_shared<Module>())
        {
          m->add(chaiscript::fun(&detail::equal<const T&, const T&>), "==");
          return m;
        }

      template<typename T>
        ModulePtr greater_than(ModulePtr m = std::make_shared<Module>())
        {
          m->add(chaiscript::fun(&detail::greater_than<const T&, const T&>), ">");
          return m;
        }

      template<typename T>
        ModulePtr greater_than_equal(ModulePtr m = std::make_shared<Module>())
        {
          m->add(chaiscript::fun(&detail::greater_than_equal<const T&, const T&>), ">=");
          return m;
        }

      template<typename T>
        ModulePtr less_than(ModulePtr m = std::make_shared<Module>())
        {
          m->add(chaiscript::fun(&detail::less_than<const T&, const T&>), "<");
          return m;
        }

      template<typename T>
        ModulePtr less_than_equal(ModulePtr m = std::make_shared<Module>())
        {
          m->add(chaiscript::fun(&detail::less_than_equal<const T&, const T&>), "<=");
          return m;
        }

      template<typename T>
        ModulePtr logical_compliment(ModulePtr m = std::make_shared<Module>())
        {
          m->add(chaiscript::fun(&detail::logical_compliment<const T &>), "!");
          return m;
        }

      template<typename T>
        ModulePtr not_equal(ModulePtr m = std::make_shared<Module>())
        {
          m->add(chaiscript::fun(&detail::not_equal<const T &, const T &>), "!=");
          return m;
        }

      template<typename T>
        ModulePtr addition(ModulePtr m = std::make_shared<Module>())
        {
          m->add(chaiscript::fun(&detail::addition<const T &, const T &>), "+");
          return m;
        }

      template<typename T>
        ModulePtr unary_plus(ModulePtr m = std::make_shared<Module>())
        {
          m->add(chaiscript::fun(&detail::unary_plus<const T &>), "+");
          return m;
        }

      template<typename T>
        ModulePtr subtraction(ModulePtr m = std::make_shared<Module>())
        {
          m->add(chaiscript::fun(&detail::subtraction<const T &, const T &>), "-");
          return m;
        }

      template<typename T>
        ModulePtr unary_minus(ModulePtr m = std::make_shared<Module>())
        {
          m->add(chaiscript::fun(&detail::unary_minus<const T &>), "-");
          return m;
        }

      template<typename T>
        ModulePtr bitwise_and(ModulePtr m = std::make_shared<Module>())
        {
          m->add(chaiscript::fun(&detail::bitwise_and<const T &, const T &>), "&");
          return m;
        }

      template<typename T>
        ModulePtr bitwise_compliment(ModulePtr m = std::make_shared<Module>())
        {
          m->add(chaiscript::fun(&detail::bitwise_compliment<const T &>), "~");
          return m;
        }

      template<typename T>
        ModulePtr bitwise_xor(ModulePtr m = std::make_shared<Module>())
        {
          m->add(chaiscript::fun(&detail::bitwise_xor<const T &, const T &>), "^");
          return m;
        }

      template<typename T>
        ModulePtr bitwise_or(ModulePtr m = std::make_shared<Module>())
        {
          m->add(chaiscript::fun(&detail::bitwise_or<const T &, const T &>), "|");
          return m;
        }

      template<typename T>
        ModulePtr division(ModulePtr m = std::make_shared<Module>())
        {
          m->add(chaiscript::fun(&detail::division<const T &, const T &>), "/");
          return m;
        }

      template<typename T>
        ModulePtr left_shift(ModulePtr m = std::make_shared<Module>())
        {
          m->add(chaiscript::fun(&detail::left_shift<const T &, const T &>), "<<");
          return m;
        }

      template<typename T>
        ModulePtr multiplication(ModulePtr m = std::make_shared<Module>())
        {
          m->add(chaiscript::fun(&detail::multiplication<const T &, const T &>), "*");
          return m;
        }

      template<typename T>
        ModulePtr remainder(ModulePtr m = std::make_shared<Module>())
        {
          m->add(chaiscript::fun(&detail::remainder<const T &, const T &>), "%");
          return m;
        }

      template<typename T>
        ModulePtr right_shift(ModulePtr m = std::make_shared<Module>())
        {
          m->add(chaiscript::fun(&detail::right_shift<const T &, const T &>), ">>");
          return m;
        }
    }
  }
}

#endif
