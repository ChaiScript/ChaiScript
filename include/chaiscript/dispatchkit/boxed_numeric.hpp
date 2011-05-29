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

      struct equals
      {
        static const bool lhs_const = true;
#pragma GCC diagnostic ignored "-Wsign-compare"
        template<typename T, typename U>
        static bool go(const T &t, const U &u) { return t == u; }
      };

      struct less_than
      {
        static const bool lhs_const = true;
#pragma GCC diagnostic ignored "-Wsign-compare"
        template<typename T, typename U>
        static bool go(const T &t, const U &u) { return t < u; }
      };

      struct greater_than
      {
        static const bool lhs_const = true;
#pragma GCC diagnostic ignored "-Wsign-compare"
        template<typename T, typename U>
        static bool go(const T &t, const U &u) { return t > u; }
      };

      struct greater_than_equal
      {
        static const bool lhs_const = true;
#pragma GCC diagnostic ignored "-Wsign-compare"
        template<typename T, typename U>
        static bool go(const T &t, const U &u) { return t >= u; }
      };

      struct less_than_equal
      {
        static const bool lhs_const = true;
#pragma GCC diagnostic ignored "-Wsign-compare"
        template<typename T, typename U>
        static bool go(const T &t, const U &u) { return t <= u; }
      };

      struct not_equal
      {
        static const bool lhs_const = true;
#pragma GCC diagnostic ignored "-Wsign-compare"
        template<typename T, typename U>
        static bool go(const T &t, const U &u) { return t != u; }
      };

      struct add
      {
        static const bool lhs_const = true;
        template<typename T, typename U>
        static  Boxed_Value go(const T &t, const U &u) { return const_var(t + u); }
      };

      struct subtract
      {
        static const bool lhs_const = true;
        template<typename T, typename U>
        static  Boxed_Value go(const T &t, const U &u) { return const_var(t - u); }
      };

      struct multiply 
      {
        static const bool lhs_const = true;
        template<typename T, typename U>
        static  Boxed_Value go(const T &t, const U &u) { return const_var(t * u); }
      };

      struct divide 
      {
        static const bool lhs_const = true;
        template<typename T, typename U>
        static  Boxed_Value go(const T &t, const U &u) { return const_var(t / u); }
      };

      struct assign_product
      {
        static const bool lhs_const = false;
        template<typename T, typename U>
        static  Boxed_Value go(T &t, const U &u) { return var(&(t *= u)); }
      };

      struct assign_quotient
      {
        static const bool lhs_const = false;
        template<typename T, typename U>
        static  Boxed_Value go(T &t, const U &u) { return var(&(t /= u)); }
      };

      struct assign_sum
      {
        static const bool lhs_const = false;
        template<typename T, typename U>
        static  Boxed_Value go(T &t, const U &u) { return var(&(t += u)); }
      };

      struct assign
      {
        static const bool lhs_const = false;
        template<typename T, typename U>
        static  Boxed_Value go(T &t, const U &u) { return var(&(t.get() = u.get())); }
      };

      struct assign_difference
      {
        static const bool lhs_const = false;
        template<typename T, typename U>
        static  Boxed_Value go(T &t, const U &u) { return var(&(t -= u)); }
      };

      struct assign_bitwise_and
      {
        static const bool lhs_const = false;
        template<typename T, typename U>
        static  Boxed_Value go(T &t, const U &u) { return var(&(t &= u)); }
      };

      struct assign_bitwise_or
      {
        static const bool lhs_const = false;
        template<typename T, typename U>
        static  Boxed_Value go(T &t, const U &u) { return var(&(t |= u)); }
      };

      struct assign_bitwise_xor
      {
        static const bool lhs_const = false;
        template<typename T, typename U>
        static  Boxed_Value go(T &t, const U &u) { return var(&(t ^= u)); }
      };

      struct assign_remainder
      {
        static const bool lhs_const = false;
        template<typename T, typename U>
        static  Boxed_Value go(T &t, const U &u) { return var(&(t %= u)); }
      };

      struct assign_bitshift_left
      {
        static const bool lhs_const = false;
        template<typename T, typename U>
        static  Boxed_Value go(T &t, const U &u) { return var(&(t <<= u)); }
      };

      struct assign_bitshift_right
      {
        static const bool lhs_const = false;
        template<typename T, typename U>
        static  Boxed_Value go(T &t, const U &u) { return var(&(t >>= u)); }
      };


      struct pre_increment
      {
        static const bool lhs_const = false;
        template<typename T, typename U>
          static Boxed_Value go(T &t, const U) { return var(&++t); }
      };

      struct unary_plus
      {
        static const bool lhs_const = true;
        template<typename T, typename U>
          static Boxed_Value go(const T &t, const U) { return const_var(+t); }
      };

      struct bitwise_complement
      {
        static const bool lhs_const = true;
        template<typename T, typename U>
          static Boxed_Value go(const T &t, const U) { return const_var(~t); }
      };

      struct unary_minus
      {
        static const bool lhs_const = true;
        template<typename T, typename U>
          static Boxed_Value go(const T &t, const U) { return const_var(-t); }
      };

      struct bitwise_and 
      {
        static const bool lhs_const = true;
        template<typename T, typename U>
          static Boxed_Value go(const T &t, const U &u) { return const_var(t & u); }
      };

      struct bitwise_xor
      {
        static const bool lhs_const = true;
        template<typename T, typename U>
          static Boxed_Value go(const T &t, const U &u) { return const_var(t ^ u); }
      };

      struct bitwise_or
      {
        static const bool lhs_const = true;
        template<typename T, typename U>
          static Boxed_Value go(const T &t, const U &u) { return const_var(t | u); }
      };

      struct remainder
      {
        static const bool lhs_const = true;
        template<typename T, typename U>
          static Boxed_Value go(const T &t, const U &u) { return const_var(t % u); }
      };

      struct left_shift
      {
        static const bool lhs_const = true;
        template<typename T, typename U>
          static Boxed_Value go(const T &t, const U &u) { return const_var(t << u); }
      };

      struct right_shift
      {
        static const bool lhs_const = true;
        template<typename T, typename U>
          static Boxed_Value go(const T &t, const U &u) { return const_var(t >> u); }
      };

      struct pre_decrement
      {
        static const bool lhs_const = false;

        template<typename U>
        static Boxed_Value go(boost::reference_wrapper<bool>, const U) { std::cout<< "why where?"<< std::endl; throw boost::bad_any_cast(); }

        template<typename T, typename U>
          static Boxed_Value go(T &t, const U) { return var(&--t); }
      };

      template<typename Ret, typename O, typename L>
        static Ret oper_lhs(L l, const Boxed_Numeric &r)
        {
          const Type_Info &inp_ = r.bv.get_type_info();

          if (inp_ == typeid(double))
          {
            return O::go(l, boxed_cast<const double &>(r.bv));
          } else if (inp_ == typeid(float)) {
            return O::go(l, boxed_cast<const float&>(r.bv));
          } else if (inp_ == typeid(long double)) {
            return O::go(l, boxed_cast<const long double&>(r.bv));
          } else if (inp_ == typeid(bool)) {
            return O::go(l, boxed_cast<const bool&>(r.bv));
          } else if (inp_ == typeid(char)) {
            return O::go(l, boxed_cast<const char&>(r.bv));
          } else if (inp_ == typeid(unsigned int)) {
            return O::go(l, boxed_cast<const unsigned int&>(r.bv));
          } else if (inp_ == typeid(long)) {
            return O::go(l, boxed_cast<const long&>(r.bv));
          } else if (inp_ == typeid(unsigned long)) {
            return O::go(l, boxed_cast<const unsigned long&>(r.bv));
          } else if (inp_ == typeid(boost::int8_t)) {
            return O::go(l, boxed_cast<const boost::int8_t &>(r.bv));
          } else if (inp_ == typeid(boost::int16_t)) {
            return O::go(l, boxed_cast<const boost::int16_t &>(r.bv));
          } else if (inp_ == typeid(boost::int32_t)) {
            return O::go(l, boxed_cast<const boost::int32_t &>(r.bv));
          } else if (inp_ == typeid(boost::int64_t)) {
            return O::go(l, boxed_cast<const boost::int64_t &>(r.bv));
          } else if (inp_ == typeid(boost::uint8_t)) {
            return O::go(l, boxed_cast<const boost::uint8_t &>(r.bv));
          } else if (inp_ == typeid(boost::uint16_t)) {
            return O::go(l, boxed_cast<const boost::uint16_t &>(r.bv));
          } else if (inp_ == typeid(boost::uint32_t)) {
            return O::go(l, boxed_cast<const boost::uint32_t &>(r.bv));
          } else if (inp_ == typeid(boost::uint64_t)) {
            return O::go(l, boxed_cast<const boost::uint64_t &>(r.bv));
          } else {
            throw boost::bad_any_cast();
          }
        } 

      template<typename Ret, typename O>
        static Ret oper(const Boxed_Numeric &l, const Boxed_Numeric &r)
        {
          const Type_Info &inp_ = l.bv.get_type_info();

          if (inp_ == typeid(double))
          {
            return oper_lhs<Ret, O>(boxed_cast<typename lhs_type<O, double>::type>(l.bv), r);
          } else if (inp_ == typeid(long double)) {
            return oper_lhs<Ret, O>(boxed_cast<typename lhs_type<O, long double>::type>(l.bv), r);
          } else if (inp_ == typeid(float)) {
            return oper_lhs<Ret, O>(boxed_cast<typename lhs_type<O, float>::type>(l.bv), r);
          } else if (inp_ == typeid(bool)) {
            return oper_lhs<Ret, O>(boxed_cast<typename lhs_type<O, bool>::type>(l.bv), r);
          } else if (inp_ == typeid(char)) {
            return oper_lhs<Ret, O>(boxed_cast<typename lhs_type<O, char>::type>(l.bv), r);
          } else if (inp_ == typeid(int)) {
            return oper_lhs<Ret, O>(boxed_cast<typename lhs_type<O, int>::type>(l.bv), r);
          } else if (inp_ == typeid(unsigned int)) {
            return oper_lhs<Ret, O>(boxed_cast<typename lhs_type<O, unsigned int>::type>(l.bv), r);
          } else if (inp_ == typeid(long)) {
            return oper_lhs<Ret, O>(boxed_cast<typename lhs_type<O, long>::type>(l.bv), r);
          } else if (inp_ == typeid(unsigned long)) {
            return oper_lhs<Ret, O>(boxed_cast<typename lhs_type<O, unsigned long>::type>(l.bv), r);
          } else if (inp_ == typeid(boost::int8_t)) {
            return oper_lhs<Ret, O>(boxed_cast<typename lhs_type<O, boost::int8_t>::type>(l.bv), r);
          } else if (inp_ == typeid(boost::int16_t)) {
            return oper_lhs<Ret, O>(boxed_cast<typename lhs_type<O, boost::int32_t>::type>(l.bv), r);
          } else if (inp_ == typeid(boost::int32_t)) {
            return oper_lhs<Ret, O>(boxed_cast<typename lhs_type<O, boost::int32_t>::type>(l.bv), r);
          } else if (inp_ == typeid(boost::int64_t)) {
            return oper_lhs<Ret, O>(boxed_cast<typename lhs_type<O, boost::int64_t>::type>(l.bv), r);
          } else if (inp_ == typeid(boost::uint8_t)) {
            return oper_lhs<Ret, O>(boxed_cast<typename lhs_type<O, boost::uint8_t>::type>(l.bv), r);
          } else if (inp_ == typeid(boost::uint16_t)) {
            return oper_lhs<Ret, O>(boxed_cast<typename lhs_type<O, boost::uint16_t>::type>(l.bv), r);
          } else if (inp_ == typeid(boost::uint32_t)) {
            return oper_lhs<Ret, O>(boxed_cast<typename lhs_type<O, boost::uint32_t>::type>(l.bv), r);
          } else if (inp_ == typeid(boost::uint64_t)) {
            return oper_lhs<Ret, O>(boxed_cast<typename lhs_type<O, boost::uint64_t>::type>(l.bv), r);
          } else {
            throw boost::bad_any_cast();
          }
        }
      

      template<typename Ret, typename O, typename L>
        static Ret oper_int_lhs(L l, const Boxed_Numeric &r)
        {
          const Type_Info &inp_ = r.bv.get_type_info();

          if (inp_ == typeid(bool)) {
            return O::go(l, boxed_cast<const bool&>(r.bv));
          } else if (inp_ == typeid(char)) {
            return O::go(l, boxed_cast<const char&>(r.bv));
          } else if (inp_ == typeid(unsigned int)) {
            return O::go(l, boxed_cast<const unsigned int&>(r.bv));
          } else if (inp_ == typeid(long)) {
            return O::go(l, boxed_cast<const long&>(r.bv));
          } else if (inp_ == typeid(unsigned long)) {
            return O::go(l, boxed_cast<const unsigned long&>(r.bv));
          } else if (inp_ == typeid(boost::int8_t)) {
            return O::go(l, boxed_cast<const boost::int8_t &>(r.bv));
          } else if (inp_ == typeid(boost::int16_t)) {
            return O::go(l, boxed_cast<const boost::int16_t &>(r.bv));
          } else if (inp_ == typeid(boost::int32_t)) {
            return O::go(l, boxed_cast<const boost::int32_t &>(r.bv));
          } else if (inp_ == typeid(boost::int64_t)) {
            return O::go(l, boxed_cast<const boost::int64_t &>(r.bv));
          } else if (inp_ == typeid(boost::uint8_t)) {
            return O::go(l, boxed_cast<const boost::uint8_t &>(r.bv));
          } else if (inp_ == typeid(boost::uint16_t)) {
            return O::go(l, boxed_cast<const boost::uint16_t &>(r.bv));
          } else if (inp_ == typeid(boost::uint32_t)) {
            return O::go(l, boxed_cast<const boost::uint32_t &>(r.bv));
          } else if (inp_ == typeid(boost::uint64_t)) {
            return O::go(l, boxed_cast<const boost::uint64_t &>(r.bv));
          } else {
            throw boost::bad_any_cast();
          }
        } 

      template<typename Ret, typename O>
        static Ret oper_int(const Boxed_Numeric &l, const Boxed_Numeric &r)
        {
          const Type_Info &inp_ = l.bv.get_type_info();

          if (inp_ == typeid(bool)) {
            return oper_int_lhs<Ret, O>(boxed_cast<typename lhs_type<O, bool>::type>(l.bv), r);
          } else if (inp_ == typeid(char)) {
            return oper_int_lhs<Ret, O>(boxed_cast<typename lhs_type<O, char>::type>(l.bv), r);
          } else if (inp_ == typeid(int)) {
            return oper_int_lhs<Ret, O>(boxed_cast<typename lhs_type<O, int>::type>(l.bv), r);
          } else if (inp_ == typeid(unsigned int)) {
            return oper_int_lhs<Ret, O>(boxed_cast<typename lhs_type<O, unsigned int>::type>(l.bv), r);
          } else if (inp_ == typeid(long)) {
            return oper_int_lhs<Ret, O>(boxed_cast<typename lhs_type<O, long>::type>(l.bv), r);
          } else if (inp_ == typeid(unsigned long)) {
            return oper_int_lhs<Ret, O>(boxed_cast<typename lhs_type<O, unsigned long>::type>(l.bv), r);
          } else if (inp_ == typeid(boost::int8_t)) {
            return oper_int_lhs<Ret, O>(boxed_cast<typename lhs_type<O, boost::int8_t>::type>(l.bv), r);
          } else if (inp_ == typeid(boost::int16_t)) {
            return oper_int_lhs<Ret, O>(boxed_cast<typename lhs_type<O, boost::int32_t>::type>(l.bv), r);
          } else if (inp_ == typeid(boost::int32_t)) {
            return oper_int_lhs<Ret, O>(boxed_cast<typename lhs_type<O, boost::int32_t>::type>(l.bv), r);
          } else if (inp_ == typeid(boost::int64_t)) {
            return oper_int_lhs<Ret, O>(boxed_cast<typename lhs_type<O, boost::int64_t>::type>(l.bv), r);
          } else if (inp_ == typeid(boost::uint8_t)) {
            return oper_int_lhs<Ret, O>(boxed_cast<typename lhs_type<O, boost::uint8_t>::type>(l.bv), r);
          } else if (inp_ == typeid(boost::uint16_t)) {
            return oper_int_lhs<Ret, O>(boxed_cast<typename lhs_type<O, boost::uint16_t>::type>(l.bv), r);
          } else if (inp_ == typeid(boost::uint32_t)) {
            return oper_int_lhs<Ret, O>(boxed_cast<typename lhs_type<O, boost::uint32_t>::type>(l.bv), r);
          } else if (inp_ == typeid(boost::uint64_t)) {
            return oper_int_lhs<Ret, O>(boxed_cast<typename lhs_type<O, boost::uint64_t>::type>(l.bv), r);
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
        return oper<bool, equals>(*this, r);
      }

      bool operator<(const Boxed_Numeric &r) const
      {
        return oper<bool, less_than>(*this, r);
      }

      bool operator>(const Boxed_Numeric &r) const
      {
        return oper<bool, greater_than>(*this, r);
      }

      bool operator>=(const Boxed_Numeric &r) const
      {
        return oper<bool, greater_than_equal>(*this, r);
      }

      bool operator<=(const Boxed_Numeric &r) const
      {
        return oper<bool, less_than_equal>(*this, r);
      }

      bool operator!=(const Boxed_Numeric &r) const
      {
        return oper<bool, not_equal>(*this, r);
      }

      Boxed_Value operator--() const
      {
        return oper<Boxed_Value, pre_decrement>(*this, var(0));
      }

      Boxed_Value operator++() const
      {
        return oper<Boxed_Value, pre_increment>(*this, var(0));
      }

      Boxed_Value operator+(const Boxed_Numeric &r) const
      {
        return oper<Boxed_Value, add>(*this, r);
      }

      Boxed_Value operator+() const
      {
        return oper<Boxed_Value, unary_plus>(*this, Boxed_Value(0));
      }

      Boxed_Value operator-() const
      {
        return oper<Boxed_Value, unary_minus>(*this, Boxed_Value(0));
      }

      Boxed_Value operator-(const Boxed_Numeric &r) const
      {
        return oper<Boxed_Value, subtract>(*this, r);
      }

      Boxed_Value operator&=(const Boxed_Numeric &r) const
      {
        return oper_int<Boxed_Value, assign_bitwise_and>(*this, r);
      }

      Boxed_Value operator=(const Boxed_Numeric &r) const
      {
        return oper<Boxed_Value, assign>(*this, r);
      }

      Boxed_Value operator|=(const Boxed_Numeric &r) const
      {
        return oper_int<Boxed_Value, assign_bitwise_or>(*this, r);
      }

      Boxed_Value operator^=(const Boxed_Numeric &r) const
      {
        return oper_int<Boxed_Value, assign_bitwise_xor>(*this, r);
      }

      Boxed_Value operator%=(const Boxed_Numeric &r) const
      {
        return oper_int<Boxed_Value, assign_remainder>(*this, r);
      }

      Boxed_Value operator<<=(const Boxed_Numeric &r) const
      {
        return oper_int<Boxed_Value, assign_bitshift_left>(*this, r);
      }

      Boxed_Value operator>>=(const Boxed_Numeric &r) const
      {
        return oper_int<Boxed_Value, assign_bitshift_right>(*this, r);
      }

      Boxed_Value operator&(const Boxed_Numeric &r) const
      {
        return oper_int<Boxed_Value, bitwise_and>(*this, r);
      }

      Boxed_Value operator~() const
      {
        return oper_int<Boxed_Value, bitwise_complement>(*this, Boxed_Value(0));
      }

      Boxed_Value operator^(const Boxed_Numeric &r) const
      {
        return oper_int<Boxed_Value, bitwise_xor>(*this, r);
      }

      Boxed_Value operator|(const Boxed_Numeric &r) const
      {
        return oper_int<Boxed_Value, bitwise_or>(*this, r);
      }

      Boxed_Value operator*=(const Boxed_Numeric &r) const
      {
        return oper<Boxed_Value, assign_product>(*this, r);
      }
      Boxed_Value operator/=(const Boxed_Numeric &r) const
      {
        return oper<Boxed_Value, assign_quotient>(*this, r);
      }
      Boxed_Value operator+=(const Boxed_Numeric &r) const
      {
        return oper<Boxed_Value, assign_sum>(*this, r);
      }
      Boxed_Value operator-=(const Boxed_Numeric &r) const
      {
        return oper<Boxed_Value, assign_difference>(*this, r);
      }

      Boxed_Value operator/(const Boxed_Numeric &r) const
      {
        return oper<Boxed_Value, divide>(*this, r);
      }

      Boxed_Value operator<<(const Boxed_Numeric &r) const
      {
        return oper_int<Boxed_Value, left_shift>(*this, r);
      }

      Boxed_Value operator*(const Boxed_Numeric &r) const
      {
        return oper<Boxed_Value, multiply>(*this, r);
      }

      Boxed_Value operator%(const Boxed_Numeric &r) const
      {
        return oper_int<Boxed_Value, remainder>(*this, r);
      }

      Boxed_Value operator>>(const Boxed_Numeric &r) const
      {
        return oper_int<Boxed_Value, right_shift>(*this, r);
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

