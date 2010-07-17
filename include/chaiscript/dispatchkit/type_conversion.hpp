#ifndef __CHAISCRIPT_DISPATCH_TYPE_CONVERSION_HPP__
#define __CHAISCRIPT_DISPATCH_TYPE_CONVERSION_HPP__

#include "boxed_value.hpp"
#include "type_info.hpp"
#include <boost/type_traits/remove_reference.hpp>
#include <boost/type_traits/remove_const.hpp>
#include <boost/type_traits/remove_pointer.hpp>
#include <boost/type_traits/add_const.hpp>

namespace chaiscript 
{
  namespace detail
  {
    class Type_Conversion_Impl
    {
      public:
        ~Type_Conversion_Impl()
        { }

        virtual Boxed_Value convert(const Boxed_Value &v) const = 0;

        Type_Info from() const
        {
          return m_from;
        }

        Type_Info to() const
        {
          return m_to;
        }

      protected:
        Type_Conversion_Impl(const Type_Info &t_from, const Type_Info &t_to)
          : m_from(t_from), m_to(t_to)
        {
        }

      private:
        Type_Info m_from;
        Type_Info m_to;

    };

    template<typename From, typename To>
    class Dynamic_Cast_Conversion : public Type_Conversion_Impl
    {
      public:
        typedef From From_Type;
        typedef To To_Type;

        Dynamic_Cast_Conversion()
          : Type_Conversion_Impl(user_type<From_Type>(), user_type<To_Type>())
        { }

        virtual ~Dynamic_Cast_Conversion()
        { }

        virtual Boxed_Value convert(const Boxed_Value &v) const
        {
          if (v.is_pointer())
          {
            if (v.is_const())
            {
              return const_pointer_convert(v);
            } else {
              return pointer_convert(v);
            }
          } else {
            if (v.is_const())
            {
              return const_reference_convert(v);
            } else {
              return reference_convert(v);
            }
          }
        }

      private:
        Boxed_Value pointer_convert(const Boxed_Value &v) const
        {
          boost::shared_ptr<To_Type> to = 
            boost::dynamic_pointer_cast<To_Type>(boxed_cast<boost::shared_ptr<From_Type> >(v));

          if (to) {
            return Boxed_Value(to);
          } else {
            throw bad_boxed_cast(v.get_type_info(), typeid(To_Type));
          }
        }

        Boxed_Value const_pointer_convert(const Boxed_Value &v) const
        {
          boost::shared_ptr<typename boost::add_const<To_Type>::type> to = 
            boost::dynamic_pointer_cast<typename boost::add_const<To_Type>::type>(boxed_cast<boost::shared_ptr<typename boost::add_const<From_Type>::type> >(v));

          if (to) {
            return Boxed_Value(to);
          } else {
            throw bad_boxed_cast(v.get_type_info(), typeid(To_Type));
          }
        }

        Boxed_Value reference_convert(const Boxed_Value &v) const
        {
          To_Type *to = 
            dynamic_cast<To_Type *>(boxed_cast<From_Type *>(v));

          if (to) {
            return Boxed_Value(to);
          } else {
            throw bad_boxed_cast(v.get_type_info(), typeid(To_Type));
          }
        }

        Boxed_Value const_reference_convert(const Boxed_Value &v) const
        {
          typename boost::add_const<To_Type>::type *to = 
            dynamic_cast<typename boost::add_const<To_Type>::type *>(boxed_cast<typename boost::add_const<From_Type>::type *>(v));

          if (to) {
            return Boxed_Value(to);
          } else {
            throw bad_boxed_cast(v.get_type_info(), typeid(To_Type));
          }
        }

    };
  }


  class Type_Conversion
  {
    public:
      Type_Conversion(const boost::shared_ptr<detail::Type_Conversion_Impl> t_impl)
        : m_impl(t_impl)
      {
      }

      Boxed_Value convert(const Boxed_Value &v) const
      {
        return m_impl->convert(v);
      }

      Type_Info from() const
      {
        return m_impl->from();
      }

      Type_Info to() const
      {
        return m_impl->to();
      }

    private:
      boost::shared_ptr<detail::Type_Conversion_Impl> m_impl;

  };


  template<typename From, typename To>
  Type_Conversion dynamic_cast_conversion()
  {
    typedef typename boost::remove_const<
      typename boost::remove_pointer<
        typename boost::remove_reference<From>::type>::type>::type Cleaned_From;

    typedef typename boost::remove_const<
      typename boost::remove_pointer<
        typename boost::remove_reference<To>::type>::type>::type Cleaned_To;

    return Type_Conversion(boost::shared_ptr<detail::Type_Conversion_Impl>(new detail::Dynamic_Cast_Conversion<Cleaned_From, Cleaned_To>()));
  }
}


#endif

