// This file is distributed under the BSD License.
// See "license.txt" for details.
// Copyright 2009-2011, Jonathan Turner (jonathan@emptycrate.com)
// and Jason Turner (jason@emptycrate.com)
// http://www.chaiscript.com

#ifndef CHAISCRIPT_BOXED_NUMERIC_HPP_
#define CHAISCRIPT_BOXED_NUMERIC_HPP_

#include "type_info.hpp"
#include "boxed_value.hpp"
#include "boxed_cast_helper.hpp"
#include <boost/any.hpp>
#include <boost/cstdint.hpp>
#include <boost/integer_traits.hpp>

namespace chaiscript 
{
 
  /// \brief Represents any numeric type, generically. Used internally for generic operations between POD values
  class Boxed_Numeric
  {
    private:

      enum opers
      {
        boolean_flag,
        equals,
        less_than,
        greater_than,
        less_than_equal,
        greater_than_equal,
        not_equal,
        non_const_flag,
        assign,
        pre_increment,
        pre_decrement,
        assign_product,
        assign_sum,
        assign_quotient,
        assign_difference,
        non_const_int_flag,
        assign_bitwise_and,
        assign_bitwise_or,
        assign_shift_left,
        assign_shift_right,
        assign_remainder,
        assign_bitwise_xor,
        const_int_flag,
        shift_left,
        shift_right,
        remainder,
        bitwise_and,
        bitwise_or,
        bitwise_xor,
        bitwise_complement,
        const_flag,
        sum,
        quotient,
        product,
        difference,
        unary_plus,
        unary_minus
      };


      struct boolean
      {
#pragma GCC diagnostic ignored "-Wsign-compare"
        template<typename T, typename U>
        static Boxed_Value go(opers t_oper, const T &t, const U &u)
        {
          switch (t_oper)
          {
            case equals:
              return const_var(t == u);
            case less_than:
              return const_var(t < u);
            case greater_than:
              return const_var(t > u);
            case less_than_equal:
              return const_var(t <= u);
            case greater_than_equal:
              return const_var(t >= u);
            case not_equal:
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
        static Boxed_Value go(opers t_oper, T &t, const U &u) 
        {
          switch (t_oper)
          {
            case assign:
              return var(&(t = u));
            case pre_increment:
              return var(&(++t));
            case pre_decrement:
              return var(&(--t));
            case assign_product:
              return var(&(t *= u));
            case assign_sum:
              return var(&(t += u));
            case assign_quotient:
              return var(&(t /= u));
            case assign_difference:
              return var(&(t -= u));
            default:
              throw boost::bad_any_cast();        
          }
          throw boost::bad_any_cast();        
        }
      };

      struct binary_int
      {
        template<typename T, typename U>
        static Boxed_Value go(opers t_oper, T &t, const U &u) 
        {
          switch (t_oper)
          {
            case assign_bitwise_and:
              return var(&(t &= u));
            case assign_bitwise_or:
              return var(&(t |= u));
            case assign_shift_left:
              return var(&(t <<= u));
            case assign_shift_right:
              return var(&(t >>= u));
            case assign_remainder:
              return var(&(t %= u));
            case assign_bitwise_xor:
              return var(&(t ^= u));
            default:
              throw boost::bad_any_cast();        
          }
          throw boost::bad_any_cast();        
        }
      };

      struct const_binary_int
      {
        template<typename T, typename U>
        static Boxed_Value go(opers t_oper, const T &t, const U &u) 
        {
          switch (t_oper)
          {
            case shift_left:
              return const_var(t << u);
            case shift_right:
              return const_var(t >> u);
            case remainder:
              return const_var(t % u);
            case bitwise_and:
              return const_var(t & u);
            case bitwise_or:
              return const_var(t | u);
            case bitwise_xor:
              return const_var(t ^ u);
            case bitwise_complement:
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
        static Boxed_Value go(opers t_oper, const T &t, const U &u) 
        {
          switch (t_oper)
          {
            case sum:
              return const_var(t + u);
            case quotient:
              return const_var(t / u);
            case product:
              return const_var(t * u);
            case difference:
              return const_var(t - u);
            case unary_minus:
              return const_var(-t);
            case unary_plus:
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
        static Boxed_Value go(opers t_oper, const Boxed_Numeric &t_lhs, const Boxed_Numeric &t_rhs)
        {
          if (t_oper > boolean_flag && t_oper < non_const_flag)
          {
            return boolean::go<LHS, RHS>(t_oper, boxed_cast<const LHS &>(t_lhs.bv), boxed_cast<const RHS &>(t_rhs.bv));
          } else if (t_oper > non_const_flag && t_oper < non_const_int_flag) {
            return binary::go<LHS, RHS>(t_oper, boxed_cast<LHS &>(t_lhs.bv), boxed_cast<const RHS &>(t_rhs.bv));
          } else if (t_oper > non_const_int_flag && t_oper < const_int_flag) {
            return binary_int::go<LHS, RHS>(t_oper, boxed_cast<LHS &>(t_lhs.bv), boxed_cast<const RHS &>(t_rhs.bv));
          } else if (t_oper > const_int_flag && t_oper < const_flag) {
            return const_binary_int::go<LHS, RHS>(t_oper, boxed_cast<const LHS &>(t_lhs.bv), boxed_cast<const RHS &>(t_rhs.bv));
          } else if (t_oper > const_flag) {
            return const_binary::go<LHS, RHS>(t_oper, boxed_cast<const LHS &>(t_lhs.bv), boxed_cast<const RHS &>(t_rhs.bv));
          } else {
            throw boost::bad_any_cast();
          }
        }
      };

      template<typename LHS, typename RHS>
      struct Go<LHS, RHS, true>
      {
        static Boxed_Value go(opers t_oper, const Boxed_Numeric &t_lhs, const Boxed_Numeric &t_rhs)
        {
          if (t_oper > boolean_flag && t_oper < non_const_flag)
          {
            return boolean::go<LHS, RHS>(t_oper, boxed_cast<const LHS &>(t_lhs.bv), boxed_cast<const RHS &>(t_rhs.bv));
          } else if (t_oper > non_const_flag && t_oper < non_const_int_flag) {
            return binary::go<LHS, RHS>(t_oper, boxed_cast<LHS &>(t_lhs.bv), boxed_cast<const RHS &>(t_rhs.bv));
          } else if (t_oper > non_const_int_flag && t_oper < const_int_flag) {
            throw boost::bad_any_cast();
          } else if (t_oper > const_int_flag && t_oper < const_flag) {
            throw boost::bad_any_cast();
          } else if (t_oper > const_flag) {
            return const_binary::go<LHS, RHS>(t_oper, boxed_cast<const LHS &>(t_lhs.bv), boxed_cast<const RHS &>(t_rhs.bv));
          } else {
            throw boost::bad_any_cast();
          }
        }
      };

      template<typename LHS, bool Float>
        static Boxed_Value oper_rhs(opers t_oper, const Boxed_Numeric &t_lhs, const Boxed_Numeric &t_rhs)
        {
          const Type_Info &inp_ = t_rhs.bv.get_type_info();

          if (inp_ == typeid(double))
          {
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

        static Boxed_Value oper(opers t_oper, const Boxed_Numeric &t_lhs, const Boxed_Numeric &t_rhs)
        {
          const Type_Info &inp_ = t_lhs.bv.get_type_info();

          if (inp_ == typeid(double))
          {
            return oper_rhs<double, true>(t_oper, t_lhs, t_rhs);
          } else if (inp_ == typeid(long double)) {
            return oper_rhs<long double, true>(t_oper, t_lhs, t_rhs);
          } else if (inp_ == typeid(float)) {
            return oper_rhs<float, true>(t_oper, t_lhs, t_rhs);
          } else if (inp_ == typeid(char)) {
            return oper_rhs<char, false>(t_oper, t_lhs, t_rhs);
          } else if (inp_ == typeid(int)) {
            return oper_rhs<int, false>(t_oper, t_lhs, t_rhs);
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
      Boxed_Numeric(const Boxed_Value &v)
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


      bool operator==(const Boxed_Numeric &r) const
      {
        return boxed_cast<bool>(oper(equals, *this, r));
      }

      bool operator<(const Boxed_Numeric &r) const
      {
        return boxed_cast<bool>(oper(less_than, *this, r));
      }

      bool operator>(const Boxed_Numeric &r) const
      {
        return boxed_cast<bool>(oper(greater_than, *this, r));
      }

      bool operator>=(const Boxed_Numeric &r) const
      {
        return boxed_cast<bool>(oper(greater_than_equal, *this, r));
      }

      bool operator<=(const Boxed_Numeric &r) const
      {
        return boxed_cast<bool>(oper(less_than_equal, *this, r));
      }

      bool operator!=(const Boxed_Numeric &r) const
      {
        return boxed_cast<bool>(oper(not_equal, *this, r));
      }

      Boxed_Value operator--() const
      {
        return oper(pre_decrement, *this, var(0));
      }

      Boxed_Value operator++() const
      {
        return oper(pre_increment, *this, var(0));
      }

      Boxed_Value operator+(const Boxed_Numeric &r) const
      {
        return oper(sum, *this, r);
      }

      Boxed_Value operator+() const
      {
        return oper(unary_plus, *this, Boxed_Value(0));
      }

      Boxed_Value operator-() const
      {
        return oper(unary_minus, *this, Boxed_Value(0));
      }

      Boxed_Value operator-(const Boxed_Numeric &r) const
      {
        return oper(difference, *this, r);
      }

      Boxed_Value operator&=(const Boxed_Numeric &r) const
      {
        return oper(assign_bitwise_and, *this, r);
      }

      Boxed_Value operator=(const Boxed_Numeric &r) const
      {
        return oper(assign, *this, r);
      }

      Boxed_Value operator|=(const Boxed_Numeric &r) const
      {
        return oper(assign_bitwise_or, *this, r);
      }

      Boxed_Value operator^=(const Boxed_Numeric &r) const
      {
        return oper(assign_bitwise_xor, *this, r);
      }

      Boxed_Value operator%=(const Boxed_Numeric &r) const
      {
        return oper(assign_remainder, *this, r);
      }

      Boxed_Value operator<<=(const Boxed_Numeric &r) const
      {
        return oper(assign_shift_left, *this, r);
      }

      Boxed_Value operator>>=(const Boxed_Numeric &r) const
      {
        return oper(assign_shift_right, *this, r);
      }

      Boxed_Value operator&(const Boxed_Numeric &r) const
      {
        return oper(bitwise_and, *this, r);
      }

      Boxed_Value operator~() const
      {
        return oper(bitwise_complement, *this, Boxed_Value(0));
      }

      Boxed_Value operator^(const Boxed_Numeric &r) const
      {
        return oper(bitwise_xor, *this, r);
      }

      Boxed_Value operator|(const Boxed_Numeric &r) const
      {
        return oper(bitwise_or, *this, r);
      }

      Boxed_Value operator*=(const Boxed_Numeric &r) const
      {
        return oper(assign_product, *this, r);
      }
      Boxed_Value operator/=(const Boxed_Numeric &r) const
      {
        return oper(assign_quotient, *this, r);
      }
      Boxed_Value operator+=(const Boxed_Numeric &r) const
      {
        return oper(assign_sum, *this, r);
      }
      Boxed_Value operator-=(const Boxed_Numeric &r) const
      {
        return oper(assign_difference, *this, r);
      }

      Boxed_Value operator/(const Boxed_Numeric &r) const
      {
        return oper(quotient, *this, r);
      }

      Boxed_Value operator<<(const Boxed_Numeric &r) const
      {
        return oper(shift_left, *this, r);
      }

      Boxed_Value operator*(const Boxed_Numeric &r) const
      {
        return oper(product, *this, r);
      }

      Boxed_Value operator%(const Boxed_Numeric &r) const
      {
        return oper(remainder, *this, r);
      }

      Boxed_Value operator>>(const Boxed_Numeric &r) const
      {
        return oper(shift_right, *this, r);
      }

      Boxed_Value bv;
  };

  namespace detail
  {
    /**
     * Cast_Helper for converting from Boxed_Value to Boxed_Numeric
     */
    template<>
      struct Cast_Helper<Boxed_Numeric>
      {
        typedef Boxed_Numeric Result_Type;

        static Result_Type cast(const Boxed_Value &ob)
        {
          return Boxed_Numeric(ob);
        }
      };

    /**
     * Cast_Helper for converting from Boxed_Value to Boxed_Numeric
     */
    template<>
      struct Cast_Helper<const Boxed_Numeric &> : Cast_Helper<Boxed_Numeric>
      {
      };
      
    /**
     * Cast_Helper for converting from Boxed_Value to Boxed_Numeric
     */
    template<>
      struct Cast_Helper<const Boxed_Numeric> : Cast_Helper<Boxed_Numeric>
      {
      };  
  }
  
}



#endif

