// This file is distributed under the BSD License.
// See "license.txt" for details.
// Copyright 2009-2012, Jonathan Turner (jonathan@emptycrate.com)
// Copyright 2009-2016, Jason Turner (jason@emptycrate.com)
// http://www.chaiscript.com

#ifndef CHAISCRIPT_TYPE_INFO_HPP_
#define CHAISCRIPT_TYPE_INFO_HPP_

#include <memory>
#include <type_traits>
#include <typeinfo>
#include <string>

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
      CHAISCRIPT_CONSTEXPR Type_Info(bool t_is_const, bool t_is_reference, bool t_is_pointer, bool t_is_void, 
          bool t_is_arithmetic, const std::type_info *t_ti, const std::type_info *t_bare_ti)
        : m_type_info(t_ti), m_bare_type_info(t_bare_ti),
          m_flags((t_is_const << is_const_flag)
                + (t_is_reference << is_reference_flag)
                + (t_is_pointer << is_pointer_flag)
                + (t_is_void << is_void_flag)
                + (t_is_arithmetic << is_arithmetic_flag))
      {
      }

      CHAISCRIPT_CONSTEXPR Type_Info()
        : m_type_info(nullptr), m_bare_type_info(nullptr),
          m_flags(1 << is_undef_flag)
      {
      }

#if !defined(_MSC_VER) || _MSC_VER != 1800
      Type_Info(Type_Info&&) = default;
      Type_Info& operator=(Type_Info&&) = default;
#endif

      Type_Info(const Type_Info&) = default;
      Type_Info& operator=(const Type_Info&) = default;


      CHAISCRIPT_CONSTEXPR bool operator<(const Type_Info &ti) const CHAISCRIPT_NOEXCEPT
      {
        return m_type_info < ti.m_type_info;
      }

      CHAISCRIPT_CONSTEXPR bool operator==(const Type_Info &ti) const CHAISCRIPT_NOEXCEPT
      {
        return ti.m_type_info == m_type_info
          || (ti.m_type_info && m_type_info && *ti.m_type_info == *m_type_info);
      }

      CHAISCRIPT_CONSTEXPR bool operator==(const std::type_info &ti) const CHAISCRIPT_NOEXCEPT
      {
        return m_type_info != nullptr && (*m_type_info) == ti;
      }

      CHAISCRIPT_CONSTEXPR bool bare_equal(const Type_Info &ti) const CHAISCRIPT_NOEXCEPT
      {
        return ti.m_bare_type_info == m_bare_type_info 
          || (ti.m_bare_type_info && m_bare_type_info && *ti.m_bare_type_info == *m_bare_type_info);
      }

      CHAISCRIPT_CONSTEXPR bool bare_equal_type_info(const std::type_info &ti) const CHAISCRIPT_NOEXCEPT
      {
        return m_bare_type_info != nullptr 
          && (*m_bare_type_info) == ti;
      }

      CHAISCRIPT_CONSTEXPR bool is_const() const CHAISCRIPT_NOEXCEPT { return (m_flags & (1 << is_const_flag)) != 0; }
      CHAISCRIPT_CONSTEXPR bool is_reference() const CHAISCRIPT_NOEXCEPT { return (m_flags & (1 << is_reference_flag)) != 0; }
      CHAISCRIPT_CONSTEXPR bool is_void() const CHAISCRIPT_NOEXCEPT { return (m_flags & (1 << is_void_flag)) != 0; }
      CHAISCRIPT_CONSTEXPR bool is_arithmetic() const CHAISCRIPT_NOEXCEPT { return (m_flags & (1 << is_arithmetic_flag)) != 0; }
      CHAISCRIPT_CONSTEXPR bool is_undef() const CHAISCRIPT_NOEXCEPT { return (m_flags & (1 << is_undef_flag)) != 0; }
      CHAISCRIPT_CONSTEXPR bool is_pointer() const CHAISCRIPT_NOEXCEPT { return (m_flags & (1 << is_pointer_flag)) != 0; }

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

      CHAISCRIPT_CONSTEXPR const std::type_info *bare_type_info() const
      {
        return m_bare_type_info;
      }

    private:
      const std::type_info *m_type_info;
      const std::type_info *m_bare_type_info;
      unsigned int m_flags;
      static const int is_const_flag = 0;
      static const int is_reference_flag = 1;
      static const int is_pointer_flag = 2;
      static const int is_void_flag = 3;
      static const int is_arithmetic_flag = 4;
      static const int is_undef_flag = 5;
  };

  namespace detail
  {
    /// Helper used to create a Type_Info object
    template<typename T>
      struct Get_Type_Info
      {
        typedef T type;

        static Type_Info get()
        {
          return Type_Info(std::is_const<typename std::remove_pointer<typename std::remove_reference<T>::type>::type>::value, 
              std::is_reference<T>::value, std::is_pointer<T>::value, 
              std::is_void<T>::value,
              (std::is_arithmetic<T>::value || std::is_arithmetic<typename std::remove_reference<T>::type>::value)
                && !std::is_same<typename std::remove_const<typename std::remove_reference<T>::type>::type, bool>::value,
              &typeid(T),
              &typeid(typename Bare_Type<T>::type));
        }
      };

    template<typename T>
      struct Get_Type_Info<std::shared_ptr<T> >
      {
        typedef T type;

        static Type_Info get()
        {
          return Type_Info(std::is_const<T>::value, std::is_reference<T>::value, std::is_pointer<T>::value, 
              std::is_void<T>::value,
              std::is_arithmetic<T>::value && !std::is_same<typename std::remove_const<typename std::remove_reference<T>::type>::type, bool>::value,
              &typeid(std::shared_ptr<T> ), 
              &typeid(typename Bare_Type<T>::type));
        }
      };

    template<typename T>
      struct Get_Type_Info<const std::shared_ptr<T> &>
      {
        typedef T type;

        static Type_Info get()
        {
          return Type_Info(std::is_const<T>::value, std::is_reference<T>::value, std::is_pointer<T>::value, 
              std::is_void<T>::value,
              std::is_arithmetic<T>::value && !std::is_same<typename std::remove_const<typename std::remove_reference<T>::type>::type, bool>::value,
              &typeid(const std::shared_ptr<T> &), 
              &typeid(typename Bare_Type<T>::type));
        }
      };

    template<typename T>
      struct Get_Type_Info<std::reference_wrapper<T> >
      {
        typedef T type;

        static Type_Info get()
        {
          return Type_Info(std::is_const<T>::value, std::is_reference<T>::value, std::is_pointer<T>::value, 
              std::is_void<T>::value,
              std::is_arithmetic<T>::value && !std::is_same<typename std::remove_const<typename std::remove_reference<T>::type>::type, bool>::value,
              &typeid(std::reference_wrapper<T> ), 
              &typeid(typename Bare_Type<T>::type));
        }
      };

    template<typename T>
      struct Get_Type_Info<const std::reference_wrapper<T> &>
      {
        typedef T type;

        static Type_Info get()
        {
          return Type_Info(std::is_const<T>::value, std::is_reference<T>::value, std::is_pointer<T>::value, 
              std::is_void<T>::value,
              std::is_arithmetic<T>::value && !std::is_same<typename std::remove_const<typename std::remove_reference<T>::type>::type, bool>::value,
              &typeid(const std::reference_wrapper<T> &), 
              &typeid(typename Bare_Type<T>::type));
        }
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
   Type_Info user_type(const T &/*t*/)
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
   Type_Info user_type()
  {
    return detail::Get_Type_Info<T>::get();
  }

}

#endif

