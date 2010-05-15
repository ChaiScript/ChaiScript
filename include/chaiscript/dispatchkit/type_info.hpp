// This file is distributed under the BSD License.
// See "license.txt" for details.
// Copyright 2009-2010, Jonathan Turner (jturner@minnow-lang.org)
// and Jason Turner (lefticus@gmail.com)
// http://www.chaiscript.com

#ifndef __type_info_hpp__
#define __type_info_hpp__

#include <boost/type_traits/is_const.hpp>
#include <boost/type_traits/is_void.hpp>
#include <boost/type_traits/is_reference.hpp>
#include <boost/type_traits/is_pointer.hpp>
#include <boost/type_traits/remove_const.hpp>
#include <boost/type_traits/remove_reference.hpp>
#include <boost/type_traits/remove_pointer.hpp>
#include <boost/ref.hpp>

namespace chaiscript
{
  /**
   * compile time deduced information about a type
   */
  class Type_Info
  {
    public:
      Type_Info(bool t_is_const, bool t_is_reference, bool t_is_pointer, bool t_is_void, 
          const std::type_info *t_ti, const std::type_info *t_bareti)
        : m_is_const(t_is_const), m_is_reference(t_is_reference), m_is_pointer(t_is_pointer),
        m_is_void(t_is_void),
        m_type_info(t_ti), m_bare_type_info(t_bareti),
        m_is_undef(false)
      {
      }

      Type_Info()
        : m_is_const(false), m_is_reference(false), m_is_pointer(false),
        m_is_void(false), m_type_info(0), m_bare_type_info(0),
        m_is_undef(true)
      {
      }

      Type_Info(const Type_Info &ti)
        : m_is_const(ti.m_is_const), m_is_reference(ti.m_is_reference), 
        m_is_pointer(ti.m_is_pointer),
        m_is_void(ti.m_is_void), m_type_info(ti.m_type_info), 
        m_bare_type_info(ti.m_bare_type_info),
        m_is_undef(ti.m_is_undef)
      {
      }

      Type_Info &operator=(const Type_Info &ti)
      {
        m_is_const = ti.m_is_const;
        m_is_reference = ti.m_is_reference;
        m_is_pointer = ti.m_is_pointer;
        m_is_void = ti.m_is_void;
        m_type_info = ti.m_type_info;
        m_bare_type_info = ti.m_bare_type_info;
        m_is_undef = ti.m_is_undef;
        return *this;
      }

      bool operator<(const Type_Info &ti) const
      {
        return m_type_info < ti.m_type_info;
      }

      bool operator==(const Type_Info &ti) const
      {
        return ti.m_type_info == m_type_info 
          || (ti.m_type_info && m_type_info && *ti.m_type_info == *m_type_info);
      }

      bool operator==(const std::type_info &ti) const
      {
        return m_type_info != 0 && (*m_type_info) == ti;
      }

      bool bare_equal(const Type_Info &ti) const
      {
        return ti.m_bare_type_info == m_bare_type_info 
          || (ti.m_bare_type_info && m_bare_type_info && *ti.m_bare_type_info == *m_bare_type_info);
      }

      bool is_const() const { return m_is_const; }
      bool is_reference() const { return m_is_reference; }
      bool is_void() const { return m_is_void; }
      bool is_undef() const { return m_is_undef || m_bare_type_info == 0; }
      bool is_pointer() const { return m_is_pointer; }

      std::string name() const
      {
        if (m_type_info)
        {
          return m_type_info->name();
        } else {
          return "";
        }
      }

      std::string bare_name() const
      {
        if (m_bare_type_info)
        {
          return m_bare_type_info->name();
        } else {
          return "";
        }
      }

    private:
      bool m_is_const;
      bool m_is_reference;
      bool m_is_pointer;
      bool m_is_void;
      const std::type_info *m_type_info;
      const std::type_info *m_bare_type_info;
      bool m_is_undef;
  };

  namespace detail
  {
    /**
     * Helper used to create a Type_Info object
     */
    template<typename T>
      struct Get_Type_Info
      {
        static Type_Info get()
        {
          return Type_Info(boost::is_const<T>::value, boost::is_reference<T>::value, boost::is_pointer<T>::value, 
              boost::is_void<T>::value,
              &typeid(T), 
              &typeid(typename boost::remove_const<typename boost::remove_pointer<typename boost::remove_reference<T>::type>::type>::type));
        }
      };

    template<typename T>
      struct Get_Type_Info<boost::shared_ptr<T> >
      {
        static Type_Info get()
        {
          return Type_Info(boost::is_const<T>::value, boost::is_reference<T>::value, boost::is_pointer<T>::value, 
              boost::is_void<T>::value,
              &typeid(boost::shared_ptr<T> ), 
              &typeid(typename boost::remove_const<typename boost::remove_pointer<typename boost::remove_reference<T>::type>::type>::type));
        }
      };

    template<typename T>
      struct Get_Type_Info<const boost::shared_ptr<T> &>
      {
        static Type_Info get()
        {
          return Type_Info(boost::is_const<T>::value, boost::is_reference<T>::value, boost::is_pointer<T>::value, 
              boost::is_void<T>::value,
              &typeid(const boost::shared_ptr<T> &), 
              &typeid(typename boost::remove_const<typename boost::remove_pointer<typename boost::remove_reference<T>::type>::type>::type));
        }
      };

    template<typename T>
      struct Get_Type_Info<boost::reference_wrapper<T> >
      {
        static Type_Info get()
        {
          return Type_Info(boost::is_const<T>::value, boost::is_reference<T>::value, boost::is_pointer<T>::value, 
              boost::is_void<T>::value,
              &typeid(boost::reference_wrapper<T> ), 
              &typeid(typename boost::remove_const<typename boost::remove_pointer<typename boost::remove_reference<T>::type>::type>::type));
        }
      };
  }
  
  template<typename T>
  Type_Info user_type(T)
  {
    return detail::Get_Type_Info<T>::get();
  }


  template<typename T>
  Type_Info user_type()
  {
    return detail::Get_Type_Info<T>::get();
  }

}

#endif

