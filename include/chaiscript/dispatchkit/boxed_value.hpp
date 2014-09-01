// This file is distributed under the BSD License.
// See "license.txt" for details.
// Copyright 2009-2012, Jonathan Turner (jonathan@emptycrate.com)
// Copyright 2009-2014, Jason Turner (jason@emptycrate.com)
// http://www.chaiscript.com

#ifndef CHAISCRIPT_BOXED_VALUE_HPP_
#define CHAISCRIPT_BOXED_VALUE_HPP_

#include <functional>
#include <map>
#include <memory>
#include <type_traits>

#include "../chaiscript_threading.hpp"
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
            const chaiscript::detail::Any &to,
            bool tr,
            const void *t_void_ptr)
          : m_type_info(ti), m_obj(to), m_data_ptr(ti.is_const()?nullptr:const_cast<void *>(t_void_ptr)), m_const_data_ptr(t_void_ptr),
            m_is_ref(tr)
        {
        }

        Data &operator=(const Data &rhs)
        {
          m_type_info = rhs.m_type_info;
          m_obj = rhs.m_obj;
          m_is_ref = rhs.m_is_ref;
          m_data_ptr = rhs.m_data_ptr;
          m_const_data_ptr = rhs.m_const_data_ptr;

          if (rhs.m_attrs)
          {
            m_attrs = std::unique_ptr<std::map<std::string, Boxed_Value>>(new std::map<std::string, Boxed_Value>(*rhs.m_attrs));
          }

          return *this;
        }

        Data(const Data &) = delete;

        ~Data()
        {
        }

        Type_Info m_type_info;
        chaiscript::detail::Any m_obj;
        void *m_data_ptr;
        const void *m_const_data_ptr;
        bool m_is_ref;
        std::unique_ptr<std::map<std::string, Boxed_Value>> m_attrs;
      };

      struct Object_Data
      {
        static std::shared_ptr<Data> get(Boxed_Value::Void_Type)
        {
          return std::make_shared<Data>(
                detail::Get_Type_Info<void>::get(),
                chaiscript::detail::Any(), 
                false,
                nullptr)
              ;
        }

        template<typename T>
          static std::shared_ptr<Data> get(const std::shared_ptr<T> *obj)
          {
            return get(*obj);
          }

        template<typename T>
          static std::shared_ptr<Data> get(const std::shared_ptr<T> &obj)
          {
            return std::make_shared<Data>(
                  detail::Get_Type_Info<T>::get(), 
                  chaiscript::detail::Any(obj), 
                  false,
                  obj.get()
                );
          }

        template<typename T>
          static std::shared_ptr<Data> get(T *t)
          {
            return get(std::ref(*t));
          }

        template<typename T>
          static std::shared_ptr<Data> get(std::reference_wrapper<T> obj)
          {
            return std::make_shared<Data>(
                  detail::Get_Type_Info<T>::get(),
                  chaiscript::detail::Any(obj),
                  true,
                  &obj.get()
                );
          }

        template<typename T>
          static std::shared_ptr<Data> get(const T& t)
          {
            auto p = std::make_shared<T>(t);
            return std::make_shared<Data>(
                  detail::Get_Type_Info<T>::get(), 
                  chaiscript::detail::Any(p), 
                  false,
                  p.get()
                );
          }

        static std::shared_ptr<Data> get()
        {
          return std::make_shared<Data>(
                Type_Info(),
                chaiscript::detail::Any(),
                false,
                nullptr 
              );
        }

      };

    public:
      /// Basic Boxed_Value constructor
      template<typename T>
        explicit Boxed_Value(T t)
        : m_data(Object_Data::get(t))
        {
        }

      /// Copy constructor - each copy shares the same data pointer
      Boxed_Value(const Boxed_Value &t_so)
        : m_data(t_so.m_data)
      {
      }

      /// Unknown-type constructor
      Boxed_Value()
        : m_data(Object_Data::get())
      {
      }

      ~Boxed_Value()
      {
      }

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

      /// shared data assignment, same as copy construction
      Boxed_Value &operator=(const Boxed_Value &rhs)
      {
        Boxed_Value temp(rhs);
        swap(temp);
        return *this;
      }

      const Type_Info &get_type_info() const
      {
        return m_data->m_type_info;
      }

      /// return true if the object is uninitialized
      bool is_undef() const
      {
        return m_data->m_type_info.is_undef();
      }

      bool is_const() const
      {
        return m_data->m_type_info.is_const();
      }

      bool is_type(const Type_Info &ti) const
      {
        return m_data->m_type_info.bare_equal(ti);
      }

      bool is_null() const
      {
        return (m_data->m_data_ptr == nullptr && m_data->m_const_data_ptr == nullptr);
      }

      const chaiscript::detail::Any & get() const
      {
        return m_data->m_obj;
      }

      bool is_ref() const
      {
        return m_data->m_is_ref;
      }

      bool is_pointer() const
      {
        return !is_ref();
      }

      void *get_ptr() const
      {
        return m_data->m_data_ptr;
      }

      const void *get_const_ptr() const
      {
        return m_data->m_const_data_ptr;
      }

      Boxed_Value get_attr(const std::string &t_name)
      {
        if (!m_data->m_attrs)
        {
          m_data->m_attrs = std::unique_ptr<std::map<std::string, Boxed_Value>>(new std::map<std::string, Boxed_Value>());
        }

        return (*m_data->m_attrs)[t_name];
      }

      Boxed_Value &copy_attrs(const Boxed_Value &t_obj)
      {
        if (t_obj.m_data->m_attrs)
        {
          m_data->m_attrs = std::unique_ptr<std::map<std::string, Boxed_Value>>(new std::map<std::string, Boxed_Value>(*t_obj.m_data->m_attrs));
        }
        return *this;
      }


      /// \returns true if the two Boxed_Values share the same internal type
      static bool type_match(Boxed_Value l, Boxed_Value r)
      {
        return l.get_type_info() == r.get_type_info();
      }

    private:
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
        return Boxed_Value(std::shared_ptr<typename std::add_const<T>::type >(new T(t)));
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



}

#endif

