// This file is distributed under the BSD License.
// See "license.txt" for details.
// Copyright 2009-2010, Jonathan Turner (jonathan@emptycrate.com)
// and Jason Turner (jason@emptycrate.com)
// http://www.chaiscript.com

#ifndef CHAISCRIPT_BOXED_POD_VALUE_HPP_
#define CHAISCRIPT_BOXED_POD_VALUE_HPP_

#include "type_info.hpp"
#include "boxed_value.hpp"
#include "boxed_cast_helper.hpp"
#include <boost/any.hpp>
#include <boost/cstdint.hpp>
#include <boost/integer_traits.hpp>

namespace chaiscript 
{
  
  /**
   * Object which attempts to convert a Boxed_Value into a generic
   * POD type and provide generic POD type operations
   */
  struct Boxed_POD_Value
  {
    Boxed_POD_Value(const Boxed_Value &v)
      : d(0), i(0), m_isfloat(false)
    {
      if (v.get_type_info().is_undef())
      {
        throw boost::bad_any_cast();
      }

      const Type_Info &inp_ = v.get_type_info();

      if (inp_ == typeid(double))
      {
        d = boxed_cast<double>(v);
        m_isfloat = true;
      } else if (inp_ == typeid(float)) {
        d = boxed_cast<float>(v);
        m_isfloat = true;
      } else if (inp_ == typeid(bool)) {
        i = boxed_cast<bool>(v);
      } else if (inp_ == typeid(char)) {
        i = boxed_cast<char>(v);
      } else if (inp_ == typeid(int)) {
        i = boxed_cast<int>(v);
      } else if (inp_ == typeid(unsigned int)) {
        i = boxed_cast<unsigned int>(v);
      } else if (inp_ == typeid(long)) {
        i = boxed_cast<long>(v);
      } else if (inp_ == typeid(unsigned long)) {
        i = boxed_cast<unsigned long>(v);
      } else if (inp_ == typeid(boost::int8_t)) {
        i = boxed_cast<boost::int8_t>(v);
      } else if (inp_ == typeid(boost::int16_t)) {
        i = boxed_cast<boost::int16_t>(v);
      } else if (inp_ == typeid(boost::int32_t)) {
        i = boxed_cast<boost::int32_t>(v);
      } else if (inp_ == typeid(boost::int64_t)) {
        i = boxed_cast<boost::int64_t>(v);
      } else if (inp_ == typeid(boost::uint8_t)) {
        i = boxed_cast<boost::uint8_t>(v);
      } else if (inp_ == typeid(boost::uint16_t)) {
        i = boxed_cast<boost::uint16_t>(v);
      } else if (inp_ == typeid(boost::uint32_t)) {
        i = boxed_cast<boost::uint32_t>(v);
      } else {
        throw boost::bad_any_cast();
      }
    }

    bool operator==(const Boxed_POD_Value &r) const
    {
      return ((m_isfloat)?d:i) == ((r.m_isfloat)?r.d:r.i);
    }

    bool operator<(const Boxed_POD_Value &r) const
    {
      return ((m_isfloat)?d:i) < ((r.m_isfloat)?r.d:r.i);
    }

    bool operator>(const Boxed_POD_Value &r) const
    {
      return ((m_isfloat)?d:i) > ((r.m_isfloat)?r.d:r.i);
    }

    bool operator>=(const Boxed_POD_Value &r) const
    {
      return ((m_isfloat)?d:i) >= ((r.m_isfloat)?r.d:r.i);
    }

    bool operator<=(const Boxed_POD_Value &r) const
    {
      return ((m_isfloat)?d:i) <= ((r.m_isfloat)?r.d:r.i);
    }

    bool operator!=(const Boxed_POD_Value &r) const
    {
      return ((m_isfloat)?d:i) != ((r.m_isfloat)?r.d:r.i);
    }

    Boxed_Value operator+(const Boxed_POD_Value &r) const
    {
      if (!m_isfloat && !r.m_isfloat)
      {
        return smart_size(i + r.i);
      }

      return Boxed_Value(((m_isfloat)?d:i) + ((r.m_isfloat)?r.d:r.i));
    }

    Boxed_Value operator-(const Boxed_POD_Value &r) const
    {
      if (!m_isfloat && !r.m_isfloat)
      {
        return smart_size(i - r.i);
      }

      return Boxed_Value(((m_isfloat)?d:i) - ((r.m_isfloat)?r.d:r.i));
    }

    Boxed_Value operator&(const Boxed_POD_Value &r) const
    {
      if (!m_isfloat && !r.m_isfloat)
      {
        return Boxed_Value(i & r.i);
      }

      throw exception::bad_boxed_cast("& only valid for integer types");
    }

    Boxed_Value operator^(const Boxed_POD_Value &r) const
    {
      if (!m_isfloat && !r.m_isfloat)
      {
        return Boxed_Value(i ^ r.i);
      }

      throw exception::bad_boxed_cast("^ only valid for integer types");
    }

    Boxed_Value operator|(const Boxed_POD_Value &r) const
    {
      if (!m_isfloat && !r.m_isfloat)
      {
        return Boxed_Value(i | r.i);
      }

      throw exception::bad_boxed_cast("| only valid for integer types");
    }

    Boxed_Value operator/(const Boxed_POD_Value &r) const
    {
      if (!m_isfloat && !r.m_isfloat)
      {
        return smart_size(i / r.i);
      }

      return Boxed_Value(((m_isfloat)?d:i) / ((r.m_isfloat)?r.d:r.i));
    }

    Boxed_Value operator<<(const Boxed_POD_Value &r) const
    {
      if (!m_isfloat && !r.m_isfloat)
      {
        return smart_size(i << r.i);
      }

      throw exception::bad_boxed_cast("<< only valid for integer types");
    }


    Boxed_Value operator*(const Boxed_POD_Value &r) const
    {
      if (!m_isfloat && !r.m_isfloat)
      {
        return smart_size(i * r.i);
      }

      return Boxed_Value(((m_isfloat)?d:i) * ((r.m_isfloat)?r.d:r.i));
    }


    Boxed_Value operator%(const Boxed_POD_Value &r) const
    {
      if (!m_isfloat && !r.m_isfloat)
      {
        return smart_size(i % r.i);
      }

      throw exception::bad_boxed_cast("% only valid for integer types");
    }

    Boxed_Value operator>>(const Boxed_POD_Value &r) const
    {
      if (!m_isfloat && !r.m_isfloat)
      {
        return smart_size(i >> r.i);
      }

      throw exception::bad_boxed_cast(">> only valid for integer types");
    }

    Boxed_Value smart_size(boost::int64_t t_i) const
    {
      if (t_i < boost::integer_traits<int>::const_min
        || t_i > boost::integer_traits<int>::const_max)
      {
        return Boxed_Value(t_i);
      } else {
        return Boxed_Value(static_cast<int>(t_i));
      }
    }



    double d;
    boost::int64_t i;

    bool m_isfloat;
  };

  namespace detail
  {
    /**
     * Cast_Helper for converting from Boxed_Value to Boxed_POD_Value
     */
    template<>
      struct Cast_Helper<Boxed_POD_Value>
      {
        typedef Boxed_POD_Value Result_Type;

        static Result_Type cast(const Boxed_Value &ob)
        {
          return Boxed_POD_Value(ob);
        }
      };

    /**
     * Cast_Helper for converting from Boxed_Value to Boxed_POD_Value
     */
    template<>
      struct Cast_Helper<const Boxed_POD_Value &> : Cast_Helper<Boxed_POD_Value>
      {
      };
      
    /**
     * Cast_Helper for converting from Boxed_Value to Boxed_POD_Value
     */
    template<>
      struct Cast_Helper<const Boxed_POD_Value> : Cast_Helper<Boxed_POD_Value>
      {
      };  
  }
  
}



#endif

