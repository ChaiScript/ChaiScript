// This file is distributed under the BSD License.
// See "license.txt" for details.
// Copyright 2009-2012, Jonathan Turner (jonathan@emptycrate.com)
// Copyright 2009-2015, Jason Turner (jason@emptycrate.com)
// http://www.chaiscript.com

#ifndef CHAISCRIPT_BOXED_VALUE_HPP_
#define CHAISCRIPT_BOXED_VALUE_HPP_

#include <map>
#include <memory>
#include <type_traits>

#include "../chaiscript_defines.hpp"
#include "any.hpp"
#include "type_info.hpp"

namespace chaiscript 
{

  /// \brief A wrapper for holding any valid C++ type. All types in ChaiScript are Boxed_Value objects
  /// \sa chaiscript::boxed_cast
  class Boxed_Value
  {
    public:
      /// used for explicitly creating a "void" object
      struct Void_Type
      {
      };

    private:
      /// structure which holds the internal state of a Boxed_Value
      /// \todo Get rid of Any and merge it with this, reducing an allocation in the process
      struct Data
      {
        Data(const Type_Info &ti,
            chaiscript::detail::Any to,
            bool tr,
            const void *t_void_ptr,
            bool t_return_value)
          : m_type_info(ti), m_obj(std::move(to)), m_data_ptr(ti.is_const()?nullptr:const_cast<void *>(t_void_ptr)), m_const_data_ptr(t_void_ptr),
            m_is_ref(tr), m_return_value(t_return_value)
        {
        }

        Data &operator=(const Data &rhs)
        {
          m_type_info = rhs.m_type_info;
          m_obj = rhs.m_obj;
          m_is_ref = rhs.m_is_ref;
          m_data_ptr = rhs.m_data_ptr;
          m_const_data_ptr = rhs.m_const_data_ptr;
          m_return_value = rhs.m_return_value;

          if (rhs.m_attrs)
          {
            m_attrs = std::unique_ptr<std::map<std::string, std::shared_ptr<Data>>>(new std::map<std::string, std::shared_ptr<Data>>(*rhs.m_attrs));
          }

          return *this;
        }

        Data(const Data &) = delete;

#if !defined(__APPLE__) && (!defined(_MSC_VER) || _MSC_VER != 1800)
        Data(Data &&) = default;
        Data &operator=(Data &&rhs) = default;
#endif


        Type_Info m_type_info;
        chaiscript::detail::Any m_obj;
        void *m_data_ptr;
        const void *m_const_data_ptr;
        std::unique_ptr<std::map<std::string, std::shared_ptr<Data>>> m_attrs;
        bool m_is_ref;
        bool m_return_value;
      };

      struct Object_Data
      {
        static std::shared_ptr<Data> get(Boxed_Value::Void_Type, bool t_return_value)
        {
          return std::make_shared<Data>(
                detail::Get_Type_Info<void>::get(),
                chaiscript::detail::Any(), 
                false,
                nullptr,
                t_return_value)
              ;
        }

        template<typename T>
          static std::shared_ptr<Data> get(const std::shared_ptr<T> *obj, bool t_return_value)
          {
            return get(*obj, t_return_value);
          }

        template<typename T>
          static std::shared_ptr<Data> get(const std::shared_ptr<T> &obj, bool t_return_value)
          {
            return std::make_shared<Data>(
                  detail::Get_Type_Info<T>::get(), 
                  chaiscript::detail::Any(obj), 
                  false,
                  obj.get(),
                  t_return_value
                );
          }

        template<typename T>
          static std::shared_ptr<Data> get(std::shared_ptr<T> &&obj, bool t_return_value)
          {
            auto ptr = obj.get();
            return std::make_shared<Data>(
                  detail::Get_Type_Info<T>::get(), 
                  chaiscript::detail::Any(std::move(obj)), 
                  false,
                  ptr,
                  t_return_value
                );
          }

        template<typename T>
          static std::shared_ptr<Data> get(T *t, bool t_return_value)
          {
            return get(std::ref(*t), t_return_value);
          }

        template<typename T>
          static std::shared_ptr<Data> get(const T *t, bool t_return_value)
          {
            return get(std::cref(*t), t_return_value);
          }


        template<typename T>
          static std::shared_ptr<Data> get(std::reference_wrapper<T> obj, bool t_return_value)
          {
            auto p = &obj.get();
            return std::make_shared<Data>(
                  detail::Get_Type_Info<T>::get(),
                  chaiscript::detail::Any(std::move(obj)),
                  true,
                  p,
                  t_return_value
                );
          }

        template<typename T>
          static std::shared_ptr<Data> get(T t, bool t_return_value)
          {
            auto p = std::make_shared<T>(std::move(t));
            auto ptr = p.get();
            return std::make_shared<Data>(
                  detail::Get_Type_Info<T>::get(), 
                  chaiscript::detail::Any(std::move(p)),
                  false,
                  ptr,
                  t_return_value
                );
          }

        static std::shared_ptr<Data> get()
        {
          return std::make_shared<Data>(
                Type_Info(),
                chaiscript::detail::Any(),
                false,
                nullptr,
                false
              );
        }

      };

    public:
      /// Basic Boxed_Value constructor
        template<typename T,
          typename = typename std::enable_if<!std::is_same<Boxed_Value, typename std::decay<T>::type>::value>::type>
        explicit Boxed_Value(T &&t, bool t_return_value = false)
          : m_data(Object_Data::get(std::forward<T>(t), t_return_value))
        {
        }

      /// Unknown-type constructor
      Boxed_Value()
        : m_data(Object_Data::get())
      {
      }

#if !defined(_MSC_VER) || _MSC_VER != 1800
      Boxed_Value(Boxed_Value&&) = default;
      Boxed_Value& operator=(Boxed_Value&&) = default;
#endif

      Boxed_Value(const Boxed_Value&) = default;
      Boxed_Value& operator=(const Boxed_Value&) = default;

      void swap(Boxed_Value &rhs)
      {
        std::swap(m_data, rhs.m_data);
      }

      /// Copy the values stored in rhs.m_data to m_data.
      /// m_data pointers are not shared in this case
      Boxed_Value assign(const Boxed_Value &rhs)
      {
        (*m_data) = (*rhs.m_data);
        return *this;
      }

      const Type_Info &get_type_info() const CHAISCRIPT_NOEXCEPT
      {
        return m_data->m_type_info;
      }

      /// return true if the object is uninitialized
      bool is_undef() const CHAISCRIPT_NOEXCEPT
      {
        return m_data->m_type_info.is_undef();
      }

      bool is_const() const CHAISCRIPT_NOEXCEPT
      {
        return m_data->m_type_info.is_const();
      }

      bool is_type(const Type_Info &ti) const CHAISCRIPT_NOEXCEPT
      {
        return m_data->m_type_info.bare_equal(ti);
      }

      bool is_null() const CHAISCRIPT_NOEXCEPT
      {
        return (m_data->m_data_ptr == nullptr && m_data->m_const_data_ptr == nullptr);
      }

      const chaiscript::detail::Any & get() const CHAISCRIPT_NOEXCEPT
      {
        return m_data->m_obj;
      }

      bool is_ref() const CHAISCRIPT_NOEXCEPT
      {
        return m_data->m_is_ref;
      }

      bool is_return_value() const CHAISCRIPT_NOEXCEPT
      {
        return m_data->m_return_value;
      }

      void reset_return_value() const CHAISCRIPT_NOEXCEPT
      {
        m_data->m_return_value = false;
      }

      bool is_pointer() const CHAISCRIPT_NOEXCEPT
      {
        return !is_ref();
      }

      void *get_ptr() const CHAISCRIPT_NOEXCEPT
      {
        return m_data->m_data_ptr;
      }

      const void *get_const_ptr() const CHAISCRIPT_NOEXCEPT
      {
        return m_data->m_const_data_ptr;
      }

      Boxed_Value get_attr(const std::string &t_name)
      {
        if (!m_data->m_attrs)
        {
          m_data->m_attrs = std::unique_ptr<std::map<std::string, std::shared_ptr<Data>>>(new std::map<std::string, std::shared_ptr<Data>>());
        }

        auto &attr = (*m_data->m_attrs)[t_name];
        if (attr) {
          return Boxed_Value(attr, Internal_Construction());
        } else {
          Boxed_Value bv; //default construct a new one
          attr = bv.m_data;
          return bv;
        }
      }

      Boxed_Value &copy_attrs(const Boxed_Value &t_obj)
      {
        if (t_obj.m_data->m_attrs)
        {
          m_data->m_attrs = std::unique_ptr<std::map<std::string, std::shared_ptr<Data>>>(new std::map<std::string, std::shared_ptr<Data>>(*t_obj.m_data->m_attrs));
        }
        return *this;
      }


      /// \returns true if the two Boxed_Values share the same internal type
      static bool type_match(const Boxed_Value &l, const Boxed_Value &r) CHAISCRIPT_NOEXCEPT
      {
        return l.get_type_info() == r.get_type_info();
      }

    private:
      // necessary to avoid hitting the templated && constructor of Boxed_Value
      struct Internal_Construction{};

      Boxed_Value(const std::shared_ptr<Data> &t_data, Internal_Construction)
        : m_data(t_data) {
      }

      std::shared_ptr<Data> m_data;
  };

  /// @brief Creates a Boxed_Value. If the object passed in is a value type, it is copied. If it is a pointer, std::shared_ptr, or std::reference_type
  ///        a copy is not made.
  /// @param t The value to box
  ///
  /// Example:
  ///
  /// ~~~{.cpp}
  /// int i;
  /// chaiscript::ChaiScript chai;
  /// chai.add(chaiscript::var(i), "i");
  /// chai.add(chaiscript::var(&i), "ip");
  /// ~~~
  ///
  /// @sa @ref adding_objects
  template<typename T>
    Boxed_Value var(T t)
    {
      return Boxed_Value(t);
    }

  namespace detail {
    /// \brief Takes a value, copies it and returns a Boxed_Value object that is immutable
    /// \param[in] t Value to copy and make const
    /// \returns Immutable Boxed_Value 
    /// \sa Boxed_Value::is_const
    template<typename T>
      Boxed_Value const_var_impl(const T &t)
      {
        return Boxed_Value(std::make_shared<typename std::add_const<T>::type >(t));
      }

    /// \brief Takes a pointer to a value, adds const to the pointed to type and returns an immutable Boxed_Value.
    ///        Does not copy the pointed to value.
    /// \param[in] t Pointer to make immutable
    /// \returns Immutable Boxed_Value
    /// \sa Boxed_Value::is_const
    template<typename T>
      Boxed_Value const_var_impl(T *t)
      {
        return Boxed_Value( const_cast<typename std::add_const<T>::type *>(t) );
      }

    /// \brief Takes a std::shared_ptr to a value, adds const to the pointed to type and returns an immutable Boxed_Value.
    ///        Does not copy the pointed to value.
    /// \param[in] t Pointer to make immutable
    /// \returns Immutable Boxed_Value
    /// \sa Boxed_Value::is_const
    template<typename T>
      Boxed_Value const_var_impl(const std::shared_ptr<T> &t)
      {
        return Boxed_Value( std::const_pointer_cast<typename std::add_const<T>::type>(t) );
      }

    /// \brief Takes a std::reference_wrapper value, adds const to the referenced type and returns an immutable Boxed_Value.
    ///        Does not copy the referenced value.
    /// \param[in] t Reference object to make immutable
    /// \returns Immutable Boxed_Value
    /// \sa Boxed_Value::is_const
    template<typename T>
      Boxed_Value const_var_impl(const std::reference_wrapper<T> &t)
      {
        return Boxed_Value( std::cref(t.get()) );
      }
  }

  /// \brief Takes an object and returns an immutable Boxed_Value. If the object is a std::reference or pointer type
  ///        the value is not copied. If it is an object type, it is copied.
  /// \param[in] t Object to make immutable
  /// \returns Immutable Boxed_Value
  /// \sa chaiscript::Boxed_Value::is_const
  /// \sa chaiscript::var
  ///
  /// Example:
  /// \code
  /// enum Colors
  /// {
  ///   Blue,
  ///   Green,
  ///   Red
  /// };
  /// chaiscript::ChaiScript chai
  /// chai.add(chaiscript::const_var(Blue), "Blue"); // add immutable constant
  /// chai.add(chaiscript::const_var(Red), "Red");
  /// chai.add(chaiscript::const_var(Green), "Green");
  /// \endcode
  /// 
  /// \todo support C++11 strongly typed enums
  /// \sa \ref adding_objects
  template<typename T>
    Boxed_Value const_var(const T &t)
    {
      return detail::const_var_impl(t);
    }

#ifdef CHAISCRIPT_HAS_MAGIC_STATICS
  inline Boxed_Value const_var(bool b) {
    static auto t = detail::const_var_impl(true);
    static auto f = detail::const_var_impl(false);

    if (b) {
      return t;
    } else {
      return f;
    }
  }
#endif

}

#endif

