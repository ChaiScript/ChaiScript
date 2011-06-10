// This file is distributed under the BSD License.
// See "license.txt" for details.
// Copyright 2009-2011, Jonathan Turner (jonathan@emptycrate.com)
// and Jason Turner (jason@emptycrate.com)
// http://www.chaiscript.com

#ifndef CHAISCRIPT_BOXED_NUMERIC_HPP_
#define CHAISCRIPT_BOXED_NUMERIC_HPP_

#include "boxed_value.hpp"
#include "../language/chaiscript_algebraic.hpp"
#include <boost/cstdint.hpp>

namespace chaiscript 
{
 
  /// \brief Represents any numeric type, generically. Used internally for generic operations between POD values
  class Boxed_Number
  {
    private:
      struct boolean
      {
#pragma GCC diagnostic ignored "-Wsign-compare"
        template<typename T, typename U>
        static Boxed_Value go(Operators::Opers t_oper, const T &t, const U &u, const Boxed_Value &)
        {
          switch (t_oper)
          {
            case Operators::equals:
              return const_var(t == u);
            case Operators::less_than:
              return const_var(t < u);
            case Operators::greater_than:
              return const_var(t > u);
            case Operators::less_than_equal:
              return const_var(t <= u);
            case Operators::greater_than_equal:
              return const_var(t >= u);
            case Operators::not_equal:
              return const_var(t != u);
            default:
              throw boost::bad_any_cast();        
          }
          throw boost::bad_any_cast();        
        }
      };

      struct binary 
      {
        template<typename T, typename U>
        static Boxed_Value go(Operators::Opers t_oper, T &t, const U &u, const Boxed_Value &t_lhs) 
        {
          switch (t_oper)
          {
            case Operators::assign:
              t = u;
              break;
            case Operators::pre_increment:
              ++t;
              break;
            case Operators::pre_decrement:
              --t;
              break;
            case Operators::assign_product:
              t *= u;
              break;
            case Operators::assign_sum:
              t += u;
              break;
            case Operators::assign_quotient:
              t /= u;
              break;
            case Operators::assign_difference:
              t -= u;
              break;
            default:
              throw boost::bad_any_cast();        
          }

          return t_lhs;
        }
      };

      struct binary_int
      {
        template<typename T, typename U>
        static Boxed_Value go(Operators::Opers t_oper, T &t, const U &u, const Boxed_Value &t_lhs) 
        {
          switch (t_oper)
          {
            case Operators::assign_bitwise_and:
              t &= u;
              break;
            case Operators::assign_bitwise_or:
              t |= u;
              break;
            case Operators::assign_shift_left:
              t <<= u;
              break;
            case Operators::assign_shift_right:
              t >>= u;
              break;
            case Operators::assign_remainder:
              t %= u;
              break;
            case Operators::assign_bitwise_xor:
              t ^= u;
              break;
            default:
              throw boost::bad_any_cast();        
          }
          return t_lhs;
        }
      };

      struct const_binary_int
      {
        template<typename T, typename U>
        static Boxed_Value go(Operators::Opers t_oper, const T &t, const U &u, const Boxed_Value &) 
        {
          switch (t_oper)
          {
            case Operators::shift_left:
              return const_var(t << u);
            case Operators::shift_right:
              return const_var(t >> u);
            case Operators::remainder:
              return const_var(t % u);
            case Operators::bitwise_and:
              return const_var(t & u);
            case Operators::bitwise_or:
              return const_var(t | u);
            case Operators::bitwise_xor:
              return const_var(t ^ u);
            case Operators::bitwise_complement:
              return const_var(~t);
            default:
              throw boost::bad_any_cast();        
          }
          throw boost::bad_any_cast();        
        }
      };

      struct const_binary
      {
        template<typename T, typename U>
        static Boxed_Value go(Operators::Opers t_oper, const T &t, const U &u, const Boxed_Value &) 
        {
          switch (t_oper)
          {
            case Operators::sum:
              return const_var(t + u);
            case Operators::quotient:
              return const_var(t / u);
            case Operators::product:
              return const_var(t * u);
            case Operators::difference:
              return const_var(t - u);
            case Operators::unary_minus:
              return const_var(-t);
            case Operators::unary_plus:
              return const_var(+t);
            default:
              throw boost::bad_any_cast();        
          }
          throw boost::bad_any_cast();        
        }
      };

      template<typename LHS, typename RHS, bool Float>
      struct Go
      {
        static Boxed_Value go(Operators::Opers t_oper, const Boxed_Value &t_lhs, const Boxed_Value &t_rhs)
        {
          if (t_oper > Operators::boolean_flag && t_oper < Operators::non_const_flag)
          {
            return boolean::go<LHS, RHS>(t_oper, *static_cast<const LHS *>(t_lhs.get_const_ptr()), *static_cast<const RHS *>(t_rhs.get_const_ptr()), t_lhs);
          } else if (t_oper > Operators::non_const_flag && t_oper < Operators::non_const_int_flag && !t_lhs.is_const()) {
            return binary::go<LHS, RHS>(t_oper, *static_cast<LHS *>(t_lhs.get_ptr()), *static_cast<const RHS *>(t_rhs.get_const_ptr()), t_lhs);
          } else if (t_oper > Operators::non_const_int_flag && t_oper < Operators::const_int_flag && !t_lhs.is_const()) {
            return binary_int::go<LHS, RHS>(t_oper, *static_cast<LHS *>(t_lhs.get_ptr()), *static_cast<const RHS *>(t_rhs.get_const_ptr()), t_lhs);
          } else if (t_oper > Operators::const_int_flag && t_oper < Operators::const_flag) {
            return const_binary_int::go<LHS, RHS>(t_oper, *static_cast<const LHS *>(t_lhs.get_const_ptr()), *static_cast<const RHS *>(t_rhs.get_const_ptr()), t_lhs);
          } else if (t_oper > Operators::const_flag) {
            return const_binary::go<LHS, RHS>(t_oper, *static_cast<const LHS *>(t_lhs.get_const_ptr()), *static_cast<const RHS *>(t_rhs.get_const_ptr()), t_lhs);
          } else {
            throw boost::bad_any_cast();
          }
        }
      };

      template<typename LHS, typename RHS>
      struct Go<LHS, RHS, true>
      {
        static Boxed_Value go(Operators::Opers t_oper, const Boxed_Value &t_lhs, const Boxed_Value &t_rhs)
        {
          if (t_oper > Operators::boolean_flag && t_oper < Operators::non_const_flag)
          {
            return boolean::go<LHS, RHS>(t_oper, *static_cast<const LHS *>(t_lhs.get_const_ptr()), *static_cast<const RHS *>(t_rhs.get_const_ptr()), t_lhs);
          } else if (t_oper > Operators::non_const_flag && t_oper < Operators::non_const_int_flag && !t_lhs.is_const()) {
            return binary::go<LHS, RHS>(t_oper, *static_cast<LHS *>(t_lhs.get_ptr()), *static_cast<const RHS *>(t_rhs.get_const_ptr()), t_lhs);
          } else if (t_oper > Operators::non_const_int_flag && t_oper < Operators::const_int_flag) {
            throw boost::bad_any_cast();
          } else if (t_oper > Operators::const_int_flag && t_oper < Operators::const_flag) {
            throw boost::bad_any_cast();
          } else if (t_oper > Operators::const_flag) {
            return const_binary::go<LHS, RHS>(t_oper, *static_cast<const LHS *>(t_lhs.get_const_ptr()), *static_cast<const RHS *>(t_rhs.get_const_ptr()), t_lhs);
          } else {
            throw boost::bad_any_cast();
          }
        }
      };

      template<typename LHS, bool Float>
        static Boxed_Value oper_rhs(Operators::Opers t_oper, const Boxed_Value &t_lhs, const Boxed_Value &t_rhs)
        {
          const Type_Info &inp_ = t_rhs.get_type_info();

          if (inp_ == typeid(int)) {
            return Go<LHS, int, Float>::go(t_oper, t_lhs, t_rhs);
          } else if (inp_ == typeid(double)) {
            return Go<LHS, double, true>::go(t_oper, t_lhs, t_rhs);
          } else if (inp_ == typeid(float)) {
            return Go<LHS, float, true>::go(t_oper, t_lhs, t_rhs);
          } else if (inp_ == typeid(long double)) {
            return Go<LHS, long double, true>::go(t_oper, t_lhs, t_rhs);
          } else if (inp_ == typeid(char)) {
            return Go<LHS, char, Float>::go(t_oper, t_lhs, t_rhs);
          } else if (inp_ == typeid(unsigned int)) {
            return Go<LHS, unsigned int, Float>::go(t_oper, t_lhs, t_rhs);
          } else if (inp_ == typeid(long)) {
            return Go<LHS, long, Float>::go(t_oper, t_lhs, t_rhs);
          } else if (inp_ == typeid(unsigned long)) {
            return Go<LHS, unsigned long, Float>::go(t_oper, t_lhs, t_rhs);
          } else if (inp_ == typeid(boost::int8_t)) {
            return Go<LHS, boost::int8_t, Float>::go(t_oper, t_lhs, t_rhs);
          } else if (inp_ == typeid(boost::int16_t)) {
            return Go<LHS, boost::int16_t, Float>::go(t_oper, t_lhs, t_rhs);
          } else if (inp_ == typeid(boost::int32_t)) {
            return Go<LHS, boost::int32_t, Float>::go(t_oper, t_lhs, t_rhs);
          } else if (inp_ == typeid(boost::int64_t)) {
            return Go<LHS, boost::int64_t, Float>::go(t_oper, t_lhs, t_rhs);
          } else if (inp_ == typeid(boost::uint8_t)) {
            return Go<LHS, boost::uint8_t, Float>::go(t_oper, t_lhs, t_rhs);
          } else if (inp_ == typeid(boost::uint16_t)) {
            return Go<LHS, boost::uint16_t, Float>::go(t_oper, t_lhs, t_rhs);
          } else if (inp_ == typeid(boost::uint32_t)) {
            return Go<LHS, boost::uint32_t, Float>::go(t_oper, t_lhs, t_rhs);
          } else if (inp_ == typeid(boost::uint64_t)) {
            return Go<LHS, boost::uint64_t, Float>::go(t_oper, t_lhs, t_rhs);
          } else {
            throw boost::bad_any_cast();
          }
        } 

        static Boxed_Value oper(Operators::Opers t_oper, const Boxed_Value &t_lhs, const Boxed_Value &t_rhs)
        {
          const Type_Info &inp_ = t_lhs.get_type_info();

          if (inp_ == typeid(int)) {
            return oper_rhs<int, false>(t_oper, t_lhs, t_rhs);
          } else if (inp_ == typeid(double)) {
            return oper_rhs<double, true>(t_oper, t_lhs, t_rhs);
          } else if (inp_ == typeid(long double)) {
            return oper_rhs<long double, true>(t_oper, t_lhs, t_rhs);
          } else if (inp_ == typeid(float)) {
            return oper_rhs<float, true>(t_oper, t_lhs, t_rhs);
          } else if (inp_ == typeid(char)) {
            return oper_rhs<char, false>(t_oper, t_lhs, t_rhs);
          } else if (inp_ == typeid(unsigned int)) {
            return oper_rhs<unsigned int, false>(t_oper, t_lhs, t_rhs);
          } else if (inp_ == typeid(long)) {
            return oper_rhs<long, false>(t_oper, t_lhs, t_rhs);
          } else if (inp_ == typeid(unsigned long)) {
            return oper_rhs<unsigned long, false>(t_oper, t_lhs, t_rhs);
          } else if (inp_ == typeid(boost::int8_t)) {
            return oper_rhs<boost::int8_t, false>(t_oper, t_lhs, t_rhs);
          } else if (inp_ == typeid(boost::int16_t)) {
            return oper_rhs<boost::int32_t, false>(t_oper, t_lhs, t_rhs);
          } else if (inp_ == typeid(boost::int32_t)) {
            return oper_rhs<boost::int32_t, false>(t_oper, t_lhs, t_rhs);
          } else if (inp_ == typeid(boost::int64_t)) {
            return oper_rhs<boost::int64_t, false>(t_oper, t_lhs, t_rhs);
          } else if (inp_ == typeid(boost::uint8_t)) {
            return oper_rhs<boost::uint8_t, false>(t_oper, t_lhs, t_rhs);
          } else if (inp_ == typeid(boost::uint16_t)) {
            return oper_rhs<boost::uint16_t, false>(t_oper, t_lhs, t_rhs);
          } else if (inp_ == typeid(boost::uint32_t)) {
            return oper_rhs<boost::uint32_t, false>(t_oper, t_lhs, t_rhs);
          } else if (inp_ == typeid(boost::uint64_t)) {
            return oper_rhs<boost::uint64_t, false>(t_oper, t_lhs, t_rhs);
          } else  {
            throw boost::bad_any_cast();
          }
        }
      

      
    public:
      Boxed_Number(const Boxed_Value &v)
        : bv(v)
      {
        const Type_Info &inp_ = v.get_type_info();
        if (inp_ == typeid(bool))
        {
          throw boost::bad_any_cast();
        }

        if (!inp_.is_arithmetic())
        {
          throw boost::bad_any_cast();
        }
      }


      bool operator==(const Boxed_Number &t_rhs) const
      {
        return boxed_cast<bool>(oper(Operators::equals, this->bv, t_rhs.bv));
      }

      bool operator<(const Boxed_Number &t_rhs) const
      {
        return boxed_cast<bool>(oper(Operators::less_than, this->bv, t_rhs.bv));
      }

      bool operator>(const Boxed_Number &t_rhs) const
      {
        return boxed_cast<bool>(oper(Operators::greater_than, this->bv, t_rhs.bv));
      }

      bool operator>=(const Boxed_Number &t_rhs) const
      {
        return boxed_cast<bool>(oper(Operators::greater_than_equal, this->bv, t_rhs.bv));
      }

      bool operator<=(const Boxed_Number &t_rhs) const
      {
        return boxed_cast<bool>(oper(Operators::less_than_equal, this->bv, t_rhs.bv));
      }

      bool operator!=(const Boxed_Number &t_rhs) const
      {
        return boxed_cast<bool>(oper(Operators::not_equal, this->bv, t_rhs.bv));
      }

      Boxed_Number operator--()
      {
        return oper(Operators::pre_decrement, this->bv, var(0));
      }

      Boxed_Number operator++() 
      {
        return oper(Operators::pre_increment, this->bv, var(0));
      }

      Boxed_Number operator+(const Boxed_Number &t_rhs) const
      {
        return oper(Operators::sum, this->bv, t_rhs.bv);
      }

      Boxed_Number operator+() const
      {
        return oper(Operators::unary_plus, this->bv, Boxed_Value(0));
      }

      Boxed_Number operator-() const
      {
        return oper(Operators::unary_minus, this->bv, Boxed_Value(0));
      }

      Boxed_Number operator-(const Boxed_Number &t_rhs) const
      {
        return oper(Operators::difference, this->bv, t_rhs.bv);
      }

      Boxed_Number operator&=(const Boxed_Number &t_rhs)
      {
        return oper(Operators::assign_bitwise_and, this->bv, t_rhs.bv);
      }

      Boxed_Number operator=(const Boxed_Number &t_rhs) const
      {
        return oper(Operators::assign, this->bv, t_rhs.bv);
      }

      Boxed_Number operator|=(const Boxed_Number &t_rhs)
      {
        return oper(Operators::assign_bitwise_or, this->bv, t_rhs.bv);
      }

      Boxed_Number operator^=(const Boxed_Number &t_rhs)
      {
        return oper(Operators::assign_bitwise_xor, this->bv, t_rhs.bv);
      }

      Boxed_Number operator%=(const Boxed_Number &t_rhs)
      {
        return oper(Operators::assign_remainder, this->bv, t_rhs.bv);
      }

      Boxed_Number operator<<=(const Boxed_Number &t_rhs)
      {
        return oper(Operators::assign_shift_left, this->bv, t_rhs.bv);
      }

      Boxed_Number operator>>=(const Boxed_Number &t_rhs)
      {
        return oper(Operators::assign_shift_right, this->bv, t_rhs.bv);
      }

      Boxed_Number operator&(const Boxed_Number &t_rhs) const
      {
        return oper(Operators::bitwise_and, this->bv, t_rhs.bv);
      }

      Boxed_Number operator~() const
      {
        return oper(Operators::bitwise_complement, this->bv, Boxed_Value(0));
      }

      Boxed_Number operator^(const Boxed_Number &t_rhs) const
      {
        return oper(Operators::bitwise_xor, this->bv, t_rhs.bv);
      }

      Boxed_Number operator|(const Boxed_Number &t_rhs) const
      {
        return oper(Operators::bitwise_or, this->bv, t_rhs.bv);
      }

      Boxed_Number operator*=(const Boxed_Number &t_rhs) 
      {
        return oper(Operators::assign_product, this->bv, t_rhs.bv);
      }
      Boxed_Number operator/=(const Boxed_Number &t_rhs)
      {
        return oper(Operators::assign_quotient, this->bv, t_rhs.bv);
      }
      Boxed_Number operator+=(const Boxed_Number &t_rhs)
      {
        return oper(Operators::assign_sum, this->bv, t_rhs.bv);
      }
      Boxed_Number operator-=(const Boxed_Number &t_rhs)
      {
        return oper(Operators::assign_difference, this->bv, t_rhs.bv);
      }

      Boxed_Number operator/(const Boxed_Number &t_rhs) const
      {
        return oper(Operators::quotient, this->bv, t_rhs.bv);
      }

      Boxed_Number operator<<(const Boxed_Number &t_rhs) const
      {
        return oper(Operators::shift_left, this->bv, t_rhs.bv);
      }

      Boxed_Number operator*(const Boxed_Number &t_rhs) const
      {
        return oper(Operators::product, this->bv, t_rhs.bv);
      }

      Boxed_Number operator%(const Boxed_Number &t_rhs) const
      {
        return oper(Operators::remainder, this->bv, t_rhs.bv);
      }

      Boxed_Number operator>>(const Boxed_Number &t_rhs) const
      {
        return oper(Operators::shift_right, this->bv, t_rhs.bv);
      }



      static bool equals(const Boxed_Number &t_lhs, const Boxed_Number &t_rhs)
      {
        return boxed_cast<bool>(oper(Operators::equals, t_lhs.bv, t_rhs.bv));
      }

      static bool less_than(const Boxed_Number &t_lhs, const Boxed_Number &t_rhs)
      {
        return boxed_cast<bool>(oper(Operators::less_than, t_lhs.bv, t_rhs.bv));
      }

      static bool greater_than(const Boxed_Number &t_lhs, const Boxed_Number &t_rhs) 
      {
        return boxed_cast<bool>(oper(Operators::greater_than, t_lhs.bv, t_rhs.bv));
      }

      static bool greater_than_equal(const Boxed_Number &t_lhs, const Boxed_Number &t_rhs)
      {
        return boxed_cast<bool>(oper(Operators::greater_than_equal, t_lhs.bv, t_rhs.bv));
      }

      static bool less_than_equal(const Boxed_Number &t_lhs, const Boxed_Number &t_rhs)
      {
        return boxed_cast<bool>(oper(Operators::less_than_equal, t_lhs.bv, t_rhs.bv));
      }

      static bool not_equal(const Boxed_Number &t_lhs, const Boxed_Number &t_rhs) 
      {
        return boxed_cast<bool>(oper(Operators::not_equal, t_lhs.bv, t_rhs.bv));
      }

      static Boxed_Number pre_decrement(Boxed_Number t_lhs)
      {
        return oper(Operators::pre_decrement, t_lhs.bv, var(0));
      }

      static Boxed_Number pre_increment(Boxed_Number t_lhs) 
      {
        return oper(Operators::pre_increment, t_lhs.bv, var(0));
      }

      static const Boxed_Number sum(const Boxed_Number &t_lhs, const Boxed_Number &t_rhs) 
      {
        return oper(Operators::sum, t_lhs.bv, t_rhs.bv);
      }

      static const Boxed_Number unary_plus(const Boxed_Number &t_lhs) 
      {
        return oper(Operators::unary_plus, t_lhs.bv, Boxed_Value(0));
      }

      static const Boxed_Number unary_minus(const Boxed_Number &t_lhs)
      {
        return oper(Operators::unary_minus, t_lhs.bv, Boxed_Value(0));
      }

      static const Boxed_Number difference(const Boxed_Number &t_lhs, const Boxed_Number &t_rhs) 
      {
        return oper(Operators::difference, t_lhs.bv, t_rhs.bv);
      }

      static Boxed_Number assign_bitwise_and(Boxed_Number t_lhs, const Boxed_Number &t_rhs)
      {
        return oper(Operators::assign_bitwise_and, t_lhs.bv, t_rhs.bv);
      }

      static Boxed_Number assign(Boxed_Number t_lhs, const Boxed_Number &t_rhs) 
      {
        return oper(Operators::assign, t_lhs.bv, t_rhs.bv);
      }

      static Boxed_Number assign_bitwise_or(Boxed_Number t_lhs, const Boxed_Number &t_rhs)
      {
        return oper(Operators::assign_bitwise_or, t_lhs.bv, t_rhs.bv);
      }

      static Boxed_Number assign_bitwise_xor(Boxed_Number t_lhs, const Boxed_Number &t_rhs)
      {
        return oper(Operators::assign_bitwise_xor, t_lhs.bv, t_rhs.bv);
      }

      static Boxed_Number assign_remainder(Boxed_Number t_lhs, const Boxed_Number &t_rhs)
      {
        return oper(Operators::assign_remainder, t_lhs.bv, t_rhs.bv);
      }

      static Boxed_Number assign_shift_left(Boxed_Number t_lhs, const Boxed_Number &t_rhs)
      {
        return oper(Operators::assign_shift_left, t_lhs.bv, t_rhs.bv);
      }

      static Boxed_Number assign_shift_right(Boxed_Number t_lhs, const Boxed_Number &t_rhs)
      {
        return oper(Operators::assign_shift_right, t_lhs.bv, t_rhs.bv);
      }

      static const Boxed_Number bitwise_and(const Boxed_Number &t_lhs, const Boxed_Number &t_rhs)
      {
        return oper(Operators::bitwise_and, t_lhs.bv, t_rhs.bv);
      }

      static const Boxed_Number bitwise_complement(const Boxed_Number &t_lhs)
      {
        return oper(Operators::bitwise_complement, t_lhs.bv, Boxed_Value(0));
      }

      static const Boxed_Number bitwise_xor(const Boxed_Number &t_lhs, const Boxed_Number &t_rhs)
      {
        return oper(Operators::bitwise_xor, t_lhs.bv, t_rhs.bv);
      }

      static const Boxed_Number bitwise_or(const Boxed_Number &t_lhs, const Boxed_Number &t_rhs)
      {
        return oper(Operators::bitwise_or, t_lhs.bv, t_rhs.bv);
      }

      static Boxed_Number assign_product(Boxed_Number t_lhs, const Boxed_Number &t_rhs) 
      {
        return oper(Operators::assign_product, t_lhs.bv, t_rhs.bv);
      }

      static Boxed_Number assign_quotient(Boxed_Number t_lhs, const Boxed_Number &t_rhs)
      {
        return oper(Operators::assign_quotient, t_lhs.bv, t_rhs.bv);
      }

      static Boxed_Number assign_sum(Boxed_Number t_lhs, const Boxed_Number &t_rhs)
      {
        return oper(Operators::assign_sum, t_lhs.bv, t_rhs.bv);
      }
      static Boxed_Number assign_difference(Boxed_Number t_lhs, const Boxed_Number &t_rhs)
      {
        return oper(Operators::assign_difference, t_lhs.bv, t_rhs.bv);
      }

      static const Boxed_Number quotient(const Boxed_Number &t_lhs, const Boxed_Number &t_rhs) 
      {
        return oper(Operators::quotient, t_lhs.bv, t_rhs.bv);
      }

      static const Boxed_Number shift_left(const Boxed_Number &t_lhs, const Boxed_Number &t_rhs)
      {
        return oper(Operators::shift_left, t_lhs.bv, t_rhs.bv);
      }

      static const Boxed_Number product(const Boxed_Number &t_lhs, const Boxed_Number &t_rhs)
      {
        return oper(Operators::product, t_lhs.bv, t_rhs.bv);
      }

      static const Boxed_Number remainder(const Boxed_Number &t_lhs, const Boxed_Number &t_rhs)
      {
        return oper(Operators::remainder, t_lhs.bv, t_rhs.bv);
      }

      static const Boxed_Number shift_right(const Boxed_Number &t_lhs, const Boxed_Number &t_rhs)
      {
        return oper(Operators::shift_right, t_lhs.bv, t_rhs.bv);
      }



      static Boxed_Value do_oper(Operators::Opers t_oper, const Boxed_Value &t_lhs, const Boxed_Value &t_rhs)
      {
        return oper(t_oper, t_lhs, t_rhs);
      }

      static Boxed_Value do_oper(Operators::Opers t_oper, const Boxed_Value &t_lhs)
      {
        return oper(t_oper, t_lhs, const_var(0));
      }



      Boxed_Value bv;
  };

  namespace detail
  {
    /**
     * Cast_Helper for converting from Boxed_Value to Boxed_Number
     */
    template<>
      struct Cast_Helper<Boxed_Number>
      {
        typedef Boxed_Number Result_Type;

        static Result_Type cast(const Boxed_Value &ob)
        {
          return Boxed_Number(ob);
        }
      };

    /**
     * Cast_Helper for converting from Boxed_Value to Boxed_Number
     */
    template<>
      struct Cast_Helper<const Boxed_Number &> : Cast_Helper<Boxed_Number>
      {
      };
      
    /**
     * Cast_Helper for converting from Boxed_Value to Boxed_Number
     */
    template<>
      struct Cast_Helper<const Boxed_Number> : Cast_Helper<Boxed_Number>
      {
      };  
  }
  
}



#endif

