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
      template<typename T, bool Const>
      struct choose_const 
      {
        typedef T& type;
      };

      template<typename T>
        struct choose_const<T, true>
        {
          typedef const T& type;
        };

      template<typename O, typename T>
        struct lhs_type
        {
          typedef typename choose_const<T, O::lhs_const>::type type;
        };

      struct boolean
      {
        enum oper
        {
          equals,
          less_than,
          greater_than,
          less_than_equal,
          greater_than_equal,
          not_equal
        };

        oper m_oper;

        boolean(boolean::oper t_oper)
          : m_oper(t_oper)
        {
        }

        static const bool lhs_const = true;

#pragma GCC diagnostic ignored "-Wsign-compare"
        template<typename T, typename U>
        bool go(const T &t, const U &u) const 
        {
          switch (m_oper)
          {
            case equals:
              return t == u;
            case less_than:
              return t < u;
            case greater_than:
              return t > u;
            case less_than_equal:
              return t <= u;
            case greater_than_equal:
              return t >= u;
            case not_equal:
              return t != u;
          }
          throw boost::bad_any_cast();        
        }
      };

      struct binary 
      {
        enum oper
        {
          assign,
          pre_increment,
          pre_decrement,
          assign_product,
          assign_sum,
          assign_quotient,
          assign_difference,
        };

        oper m_oper;

        binary(binary::oper t_oper)
          : m_oper(t_oper)
        {
        }

        static const bool lhs_const = false;

        template<typename T, typename U>
        Boxed_Value go(T &t, const U &u) const 
        {
          switch (m_oper)
          {
            case assign:
              return var(&(t.get() = u.get()));
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
          }
          throw boost::bad_any_cast();        
        }
      };


      struct binary_int
      {
        enum oper
        {
          assign_bitwise_and,
          assign_bitwise_or,
          assign_shift_left,
          assign_shift_right,
          assign_remainder,
          assign_bitwise_xor,
        };

        oper m_oper;

        binary_int(binary_int::oper t_oper)
          : m_oper(t_oper)
        {
        }

        static const bool lhs_const = false;

        template<typename T, typename U>
        Boxed_Value go(T &t, const U &u) const 
        {
          switch (m_oper)
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
          }
          throw boost::bad_any_cast();        
        }
      };

      struct const_binary_int
      {
        enum oper
        {
          shift_left,
          shift_right,
          remainder,
          bitwise_and,
          bitwise_or,
          bitwise_xor,
          bitwise_complement
        };

        oper m_oper;

        const_binary_int(const_binary_int::oper t_oper)
          : m_oper(t_oper)
        {
        }

        static const bool lhs_const = true;

        template<typename T, typename U>
        Boxed_Value go(const T &t, const U &u) const 
        {
          switch (m_oper)
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
          }
          throw boost::bad_any_cast();        
        }
      };

      struct const_binary
      {
        enum oper
        {
          sum,
          quotient,
          product,
          difference,
          unary_plus,
          unary_minus
        };

        oper m_oper;

        const_binary(const_binary::oper t_oper)
          : m_oper(t_oper)
        {
        }

        static const bool lhs_const = true;

        template<typename T, typename U>
        Boxed_Value go(const T &t, const U &u) const 
        {
          switch (m_oper)
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
          }
          throw boost::bad_any_cast();        
        }
      };

      template<typename Ret, typename O, typename L>
        static Ret oper_lhs(const O &t_o, L l, const Boxed_Numeric &r)
        {
          const Type_Info &inp_ = r.bv.get_type_info();

          if (inp_ == typeid(double))
          {
            return t_o.go(l, boxed_cast<const double &>(r.bv));
          } else if (inp_ == typeid(float)) {
            return t_o.go(l, boxed_cast<const float&>(r.bv));
          } else if (inp_ == typeid(long double)) {
            return t_o.go(l, boxed_cast<const long double&>(r.bv));
          } else if (inp_ == typeid(char)) {
            return t_o.go(l, boxed_cast<const char&>(r.bv));
          } else if (inp_ == typeid(unsigned int)) {
            return t_o.go(l, boxed_cast<const unsigned int&>(r.bv));
          } else if (inp_ == typeid(long)) {
            return t_o.go(l, boxed_cast<const long&>(r.bv));
          } else if (inp_ == typeid(unsigned long)) {
            return t_o.go(l, boxed_cast<const unsigned long&>(r.bv));
          } else if (inp_ == typeid(boost::int8_t)) {
            return t_o.go(l, boxed_cast<const boost::int8_t &>(r.bv));
          } else if (inp_ == typeid(boost::int16_t)) {
            return t_o.go(l, boxed_cast<const boost::int16_t &>(r.bv));
          } else if (inp_ == typeid(boost::int32_t)) {
            return t_o.go(l, boxed_cast<const boost::int32_t &>(r.bv));
          } else if (inp_ == typeid(boost::int64_t)) {
            return t_o.go(l, boxed_cast<const boost::int64_t &>(r.bv));
          } else if (inp_ == typeid(boost::uint8_t)) {
            return t_o.go(l, boxed_cast<const boost::uint8_t &>(r.bv));
          } else if (inp_ == typeid(boost::uint16_t)) {
            return t_o.go(l, boxed_cast<const boost::uint16_t &>(r.bv));
          } else if (inp_ == typeid(boost::uint32_t)) {
            return t_o.go(l, boxed_cast<const boost::uint32_t &>(r.bv));
          } else if (inp_ == typeid(boost::uint64_t)) {
            return t_o.go(l, boxed_cast<const boost::uint64_t &>(r.bv));
          } else {
            throw boost::bad_any_cast();
          }
        } 

      template<typename Ret, typename O>
        static Ret oper(const O& t_o, const Boxed_Numeric &l, const Boxed_Numeric &r)
        {
          const Type_Info &inp_ = l.bv.get_type_info();

          if (inp_ == typeid(double))
          {
            return oper_lhs<Ret>(t_o, boxed_cast<typename lhs_type<O, double>::type>(l.bv), r);
          } else if (inp_ == typeid(long double)) {
            return oper_lhs<Ret>(t_o, boxed_cast<typename lhs_type<O, long double>::type>(l.bv), r);
          } else if (inp_ == typeid(float)) {
            return oper_lhs<Ret>(t_o, boxed_cast<typename lhs_type<O, float>::type>(l.bv), r);
          } else if (inp_ == typeid(char)) {
            return oper_lhs<Ret>(t_o, boxed_cast<typename lhs_type<O, char>::type>(l.bv), r);
          } else if (inp_ == typeid(int)) {
            return oper_lhs<Ret>(t_o, boxed_cast<typename lhs_type<O, int>::type>(l.bv), r);
          } else if (inp_ == typeid(unsigned int)) {
            return oper_lhs<Ret>(t_o, boxed_cast<typename lhs_type<O, unsigned int>::type>(l.bv), r);
          } else if (inp_ == typeid(long)) {
            return oper_lhs<Ret>(t_o, boxed_cast<typename lhs_type<O, long>::type>(l.bv), r);
          } else if (inp_ == typeid(unsigned long)) {
            return oper_lhs<Ret>(t_o, boxed_cast<typename lhs_type<O, unsigned long>::type>(l.bv), r);
          } else if (inp_ == typeid(boost::int8_t)) {
            return oper_lhs<Ret>(t_o, boxed_cast<typename lhs_type<O, boost::int8_t>::type>(l.bv), r);
          } else if (inp_ == typeid(boost::int16_t)) {
            return oper_lhs<Ret>(t_o, boxed_cast<typename lhs_type<O, boost::int32_t>::type>(l.bv), r);
          } else if (inp_ == typeid(boost::int32_t)) {
            return oper_lhs<Ret>(t_o, boxed_cast<typename lhs_type<O, boost::int32_t>::type>(l.bv), r);
          } else if (inp_ == typeid(boost::int64_t)) {
            return oper_lhs<Ret>(t_o, boxed_cast<typename lhs_type<O, boost::int64_t>::type>(l.bv), r);
          } else if (inp_ == typeid(boost::uint8_t)) {
            return oper_lhs<Ret>(t_o, boxed_cast<typename lhs_type<O, boost::uint8_t>::type>(l.bv), r);
          } else if (inp_ == typeid(boost::uint16_t)) {
            return oper_lhs<Ret>(t_o, boxed_cast<typename lhs_type<O, boost::uint16_t>::type>(l.bv), r);
          } else if (inp_ == typeid(boost::uint32_t)) {
            return oper_lhs<Ret>(t_o, boxed_cast<typename lhs_type<O, boost::uint32_t>::type>(l.bv), r);
          } else if (inp_ == typeid(boost::uint64_t)) {
            return oper_lhs<Ret>(t_o, boxed_cast<typename lhs_type<O, boost::uint64_t>::type>(l.bv), r);
          } else {
            throw boost::bad_any_cast();
          }
        }
      

      template<typename Ret, typename O, typename L>
        static Ret oper_int_lhs(const O &t_o, L l, const Boxed_Numeric &r)
        {
          const Type_Info &inp_ = r.bv.get_type_info();

          if (inp_ == typeid(char)) {
            return t_o.go(l, boxed_cast<const char&>(r.bv));
          } else if (inp_ == typeid(unsigned int)) {
            return t_o.go(l, boxed_cast<const unsigned int&>(r.bv));
          } else if (inp_ == typeid(long)) {
            return t_o.go(l, boxed_cast<const long&>(r.bv));
          } else if (inp_ == typeid(unsigned long)) {
            return t_o.go(l, boxed_cast<const unsigned long&>(r.bv));
          } else if (inp_ == typeid(boost::int8_t)) {
            return t_o.go(l, boxed_cast<const boost::int8_t &>(r.bv));
          } else if (inp_ == typeid(boost::int16_t)) {
            return t_o.go(l, boxed_cast<const boost::int16_t &>(r.bv));
          } else if (inp_ == typeid(boost::int32_t)) {
            return t_o.go(l, boxed_cast<const boost::int32_t &>(r.bv));
          } else if (inp_ == typeid(boost::int64_t)) {
            return t_o.go(l, boxed_cast<const boost::int64_t &>(r.bv));
          } else if (inp_ == typeid(boost::uint8_t)) {
            return t_o.go(l, boxed_cast<const boost::uint8_t &>(r.bv));
          } else if (inp_ == typeid(boost::uint16_t)) {
            return t_o.go(l, boxed_cast<const boost::uint16_t &>(r.bv));
          } else if (inp_ == typeid(boost::uint32_t)) {
            return t_o.go(l, boxed_cast<const boost::uint32_t &>(r.bv));
          } else if (inp_ == typeid(boost::uint64_t)) {
            return t_o.go(l, boxed_cast<const boost::uint64_t &>(r.bv));
          } else {
            throw boost::bad_any_cast();
          }
        } 

      template<typename Ret, typename O>
        static Ret oper_int(const O &t_o, const Boxed_Numeric &l, const Boxed_Numeric &r)
        {
          const Type_Info &inp_ = l.bv.get_type_info();

          if (inp_ == typeid(char)) {
            return oper_int_lhs<Ret>(t_o, boxed_cast<typename lhs_type<O, char>::type>(l.bv), r);
          } else if (inp_ == typeid(int)) {
            return oper_int_lhs<Ret>(t_o, boxed_cast<typename lhs_type<O, int>::type>(l.bv), r);
          } else if (inp_ == typeid(unsigned int)) {
            return oper_int_lhs<Ret>(t_o, boxed_cast<typename lhs_type<O, unsigned int>::type>(l.bv), r);
          } else if (inp_ == typeid(long)) {
            return oper_int_lhs<Ret>(t_o, boxed_cast<typename lhs_type<O, long>::type>(l.bv), r);
          } else if (inp_ == typeid(unsigned long)) {
            return oper_int_lhs<Ret>(t_o, boxed_cast<typename lhs_type<O, unsigned long>::type>(l.bv), r);
          } else if (inp_ == typeid(boost::int8_t)) {
            return oper_int_lhs<Ret>(t_o, boxed_cast<typename lhs_type<O, boost::int8_t>::type>(l.bv), r);
          } else if (inp_ == typeid(boost::int16_t)) {
            return oper_int_lhs<Ret>(t_o, boxed_cast<typename lhs_type<O, boost::int32_t>::type>(l.bv), r);
          } else if (inp_ == typeid(boost::int32_t)) {
            return oper_int_lhs<Ret>(t_o, boxed_cast<typename lhs_type<O, boost::int32_t>::type>(l.bv), r);
          } else if (inp_ == typeid(boost::int64_t)) {
            return oper_int_lhs<Ret>(t_o, boxed_cast<typename lhs_type<O, boost::int64_t>::type>(l.bv), r);
          } else if (inp_ == typeid(boost::uint8_t)) {
            return oper_int_lhs<Ret>(t_o, boxed_cast<typename lhs_type<O, boost::uint8_t>::type>(l.bv), r);
          } else if (inp_ == typeid(boost::uint16_t)) {
            return oper_int_lhs<Ret>(t_o, boxed_cast<typename lhs_type<O, boost::uint16_t>::type>(l.bv), r);
          } else if (inp_ == typeid(boost::uint32_t)) {
            return oper_int_lhs<Ret>(t_o, boxed_cast<typename lhs_type<O, boost::uint32_t>::type>(l.bv), r);
          } else if (inp_ == typeid(boost::uint64_t)) {
            return oper_int_lhs<Ret>(t_o, boxed_cast<typename lhs_type<O, boost::uint64_t>::type>(l.bv), r);
          } else {
            throw boost::bad_any_cast();
          }
        };
      
    public:
      Boxed_Numeric(const Boxed_Value &v)
        : bv(v)
      {
        const Type_Info &inp_ = v.get_type_info();

        if (!inp_.is_arithmetic())
        {
          throw boost::bad_any_cast();
        }
      }


      bool operator==(const Boxed_Numeric &r) const
      {
        return oper<bool>(boolean(boolean::equals), *this, r);
      }

      bool operator<(const Boxed_Numeric &r) const
      {
        return oper<bool>(boolean(boolean::less_than), *this, r);
      }

      bool operator>(const Boxed_Numeric &r) const
      {
        return oper<bool>(boolean(boolean::greater_than), *this, r);
      }

      bool operator>=(const Boxed_Numeric &r) const
      {
        return oper<bool>(boolean(boolean::greater_than_equal), *this, r);
      }

      bool operator<=(const Boxed_Numeric &r) const
      {
        return oper<bool>(boolean(boolean::less_than_equal), *this, r);
      }

      bool operator!=(const Boxed_Numeric &r) const
      {
        return oper<bool>(boolean(boolean::not_equal), *this, r);
      }

      Boxed_Value operator--() const
      {
        return oper<Boxed_Value>(binary(binary::pre_decrement), *this, var(0));
      }

      Boxed_Value operator++() const
      {
        return oper<Boxed_Value>(binary(binary::pre_increment), *this, var(0));
      }

      Boxed_Value operator+(const Boxed_Numeric &r) const
      {
        return oper<Boxed_Value>(const_binary(const_binary::sum), *this, r);
      }

      Boxed_Value operator+() const
      {
        return oper<Boxed_Value>(const_binary(const_binary::unary_plus), *this, Boxed_Value(0));
      }

      Boxed_Value operator-() const
      {
        return oper<Boxed_Value>(const_binary(const_binary::unary_minus), *this, Boxed_Value(0));
      }

      Boxed_Value operator-(const Boxed_Numeric &r) const
      {
        return oper<Boxed_Value>(const_binary(const_binary::difference), *this, r);
      }

      Boxed_Value operator&=(const Boxed_Numeric &r) const
      {
        return oper_int<Boxed_Value>(binary_int(binary_int::assign_bitwise_or), *this, r);
      }

      Boxed_Value operator=(const Boxed_Numeric &r) const
      {
        return oper<Boxed_Value>(binary(binary::assign), *this, r);
      }

      Boxed_Value operator|=(const Boxed_Numeric &r) const
      {
        return oper_int<Boxed_Value>(binary_int(binary_int::assign_bitwise_or), *this, r);
      }

      Boxed_Value operator^=(const Boxed_Numeric &r) const
      {
        return oper_int<Boxed_Value>(binary_int(binary_int::assign_bitwise_xor), *this, r);
      }

      Boxed_Value operator%=(const Boxed_Numeric &r) const
      {
        return oper_int<Boxed_Value>(binary_int(binary_int::assign_remainder), *this, r);
      }

      Boxed_Value operator<<=(const Boxed_Numeric &r) const
      {
        return oper_int<Boxed_Value>(binary_int(binary_int::assign_shift_left), *this, r);
      }

      Boxed_Value operator>>=(const Boxed_Numeric &r) const
      {
        return oper_int<Boxed_Value>(binary_int(binary_int::assign_shift_right), *this, r);
      }

      Boxed_Value operator&(const Boxed_Numeric &r) const
      {
        return oper_int<Boxed_Value>(const_binary_int(const_binary_int::bitwise_and), *this, r);
      }

      Boxed_Value operator~() const
      {
        return oper_int<Boxed_Value>(const_binary_int(const_binary_int::bitwise_complement), *this, Boxed_Value(0));
      }

      Boxed_Value operator^(const Boxed_Numeric &r) const
      {
        return oper_int<Boxed_Value>(const_binary_int(const_binary_int::bitwise_xor), *this, r);
      }

      Boxed_Value operator|(const Boxed_Numeric &r) const
      {
        return oper_int<Boxed_Value>(const_binary_int(const_binary_int::bitwise_or), *this, r);
      }

      Boxed_Value operator*=(const Boxed_Numeric &r) const
      {
        return oper<Boxed_Value>(binary(binary::assign_product), *this, r);
      }
      Boxed_Value operator/=(const Boxed_Numeric &r) const
      {
        return oper<Boxed_Value>(binary(binary::assign_quotient), *this, r);
      }
      Boxed_Value operator+=(const Boxed_Numeric &r) const
      {
        return oper<Boxed_Value>(binary(binary::assign_sum), *this, r);
      }
      Boxed_Value operator-=(const Boxed_Numeric &r) const
      {
        return oper<Boxed_Value>(binary(binary::assign_difference), *this, r);
      }

      Boxed_Value operator/(const Boxed_Numeric &r) const
      {
        return oper<Boxed_Value>(const_binary(const_binary::quotient), *this, r);
      }

      Boxed_Value operator<<(const Boxed_Numeric &r) const
      {
        return oper_int<Boxed_Value>(const_binary_int(const_binary_int::shift_left), *this, r);
      }

      Boxed_Value operator*(const Boxed_Numeric &r) const
      {
        return oper<Boxed_Value>(const_binary(const_binary::product), *this, r);
      }

      Boxed_Value operator%(const Boxed_Numeric &r) const
      {
        return oper_int<Boxed_Value>(const_binary_int(const_binary_int::remainder), *this, r);
      }

      Boxed_Value operator>>(const Boxed_Numeric &r) const
      {
        return oper_int<Boxed_Value>(const_binary_int(const_binary_int::shift_right), *this, r);
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

