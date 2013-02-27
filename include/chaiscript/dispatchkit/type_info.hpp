// This file is distributed under the BSD License.
// See "license.txt" for details.
// Copyright 2009-2012, Jonathan Turner (jonathan@emptycrate.com)
// and Jason Turner (jason@emptycrate.com)
// http://www.chaiscript.com

#ifndef CHAISCRIPT_TYPE_INFO_HPP_
#define CHAISCRIPT_TYPE_INFO_HPP_

#include <string>
#include <typeinfo>
#include <memory>
#include <type_traits>

namespace chaiscript
{

  namespace detail
  {
    template<typename T>
      struct Bare_Type
      {
        typedef typename std::remove_cv<typename std::remove_pointer<typename std::remove_reference<T>::type>::type>::type type;
      };
  }

  /// \brief Compile time deduced information about a type
  class Type_Info
  {
    public:
      constexpr Type_Info(bool t_is_const, bool t_is_reference, bool t_is_pointer, bool t_is_void, 
          bool t_is_arithmetic, const std::type_info *t_ti, const std::type_info *t_bareti)
        : m_type_info(t_ti), m_bare_type_info(t_bareti),
        m_is_const(t_is_const), m_is_reference(t_is_reference), m_is_pointer(t_is_pointer),
        m_is_void(t_is_void), m_is_arithmetic(t_is_arithmetic),
        m_is_undef(false)
      {
      }

      Type_Info()
        : m_type_info(0), m_bare_type_info(0),
        m_is_const(false), m_is_reference(false), m_is_pointer(false),
        m_is_void(false), m_is_arithmetic(false), 
        m_is_undef(true)
      {
      }

      Type_Info(const Type_Info &ti)
        : m_type_info(ti.m_type_info), 
        m_bare_type_info(ti.m_bare_type_info),
        m_is_const(ti.m_is_const), m_is_reference(ti.m_is_reference), 
        m_is_pointer(ti.m_is_pointer),
        m_is_void(ti.m_is_void), m_is_arithmetic(ti.m_is_arithmetic),
        m_is_undef(ti.m_is_undef)
      {
      }

      Type_Info &operator=(const Type_Info &ti)
      {
        m_type_info = ti.m_type_info;
        m_bare_type_info = ti.m_bare_type_info;
        m_is_const = ti.m_is_const;
        m_is_reference = ti.m_is_reference;
        m_is_pointer = ti.m_is_pointer;
        m_is_void = ti.m_is_void;
        m_is_arithmetic = ti.m_is_arithmetic;
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

      bool bare_equal_type_info(const std::type_info &ti) const
      {
        return m_bare_type_info != 0 
          && (*m_bare_type_info) == ti;
      }

      bool is_const() const { return m_is_const; }
      bool is_reference() const { return m_is_reference; }
      bool is_void() const { return m_is_void; }
      bool is_arithmetic() const { return m_is_arithmetic; }
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
      const std::type_info *m_type_info;
      const std::type_info *m_bare_type_info;
      bool m_is_const;
      bool m_is_reference;
      bool m_is_pointer;
      bool m_is_void;
      bool m_is_arithmetic;
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
        typedef T type;

        constexpr static Type_Info get()
        {
          return Type_Info(std::is_const<typename std::remove_pointer<typename std::remove_reference<T>::type>::type>::value, std::is_reference<T>::value, std::is_pointer<T>::value, 
              std::is_void<T>::value,
              std::is_arithmetic<T>::value && !std::is_same<typename std::remove_const<T>::type, bool>::value,
              &typeid(T), 
              &typeid(typename Bare_Type<T>::type));
        }
      };

    template<typename T>
      struct Get_Type_Info<std::shared_ptr<T> >
      {
        typedef T type;

        constexpr static Type_Info get()
        {
          return Type_Info(std::is_const<T>::value, std::is_reference<T>::value, std::is_pointer<T>::value, 
              std::is_void<T>::value,
              std::is_arithmetic<T>::value && !std::is_same<typename std::remove_const<T>::type, bool>::value,
              &typeid(std::shared_ptr<T> ), 
              &typeid(typename Bare_Type<T>::type));
        }
      };

    template<typename T>
      struct Get_Type_Info<const std::shared_ptr<T> &>
      {
        typedef T type;

        constexpr static Type_Info get()
        {
          return Type_Info(std::is_const<T>::value, std::is_reference<T>::value, std::is_pointer<T>::value, 
              std::is_void<T>::value,
              std::is_arithmetic<T>::value && !std::is_same<typename std::remove_const<T>::type, bool>::value,
              &typeid(const std::shared_ptr<T> &), 
              &typeid(typename Bare_Type<T>::type));
        }
      };

    template<typename T>
      struct Get_Type_Info<std::reference_wrapper<T> >
      {
        typedef T type;

        constexpr static Type_Info get()
        {
          return Type_Info(std::is_const<T>::value, std::is_reference<T>::value, std::is_pointer<T>::value, 
              std::is_void<T>::value,
              std::is_arithmetic<T>::value && !std::is_same<typename std::remove_const<T>::type, bool>::value,
              &typeid(std::reference_wrapper<T> ), 
              &typeid(typename Bare_Type<T>::type));
        }
      };

    template<typename T>
      struct Get_Type_Info<const std::reference_wrapper<T> &>
      {
        typedef T type;

        constexpr static Type_Info get()
        {
          return Type_Info(std::is_const<T>::value, std::is_reference<T>::value, std::is_pointer<T>::value, 
              std::is_void<T>::value,
              std::is_arithmetic<T>::value && !std::is_same<typename std::remove_const<T>::type, bool>::value,
              &typeid(const std::reference_wrapper<T> &), 
              &typeid(typename Bare_Type<T>::type));
        }
      };

    template<typename T>
      struct Stripped_Type
      {
        typedef typename Bare_Type<typename detail::Get_Type_Info<T>::type>::type type;
      };
  }

  /// \brief Creates a Type_Info object representing the type passed in
  /// \tparam T Type of object to get a Type_Info for, derived from the passed in parameter
  /// \return Type_Info for T
  /// 
  /// \b Example:
  /// \code
  /// int i;
  /// chaiscript::Type_Info ti = chaiscript::user_type(i);
  /// \endcode
  template<typename T>
  constexpr Type_Info user_type(const T &/*t*/)
  {
    return detail::Get_Type_Info<T>::get();
  }


  /// \brief Creates a Type_Info object representing the templated type
  /// \tparam T Type of object to get a Type_Info for
  /// \return Type_Info for T
  /// 
  /// \b Example:
  /// \code
  /// chaiscript::Type_Info ti = chaiscript::user_type<int>();
  /// \endcode
  template<typename T>
  constexpr Type_Info user_type()
  {
    return detail::Get_Type_Info<T>::get();
  }

}

#endif

