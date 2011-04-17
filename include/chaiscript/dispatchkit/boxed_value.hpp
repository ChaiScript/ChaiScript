// This file is distributed under the BSD License.
// See "license.txt" for details.
// Copyright 2009-2011, Jonathan Turner (jonathan@emptycrate.com)
// and Jason Turner (jason@emptycrate.com)
// http://www.chaiscript.com

#ifndef CHAISCRIPT_BOXED_VALUE_HPP_
#define CHAISCRIPT_BOXED_VALUE_HPP_

#include "type_info.hpp"

#include "../chaiscript_threading.hpp"

#include <map>
#include <boost/shared_ptr.hpp>
#include <boost/any.hpp>
#include <boost/function.hpp>
#include <boost/ref.hpp>
#include <boost/bind.hpp>
#include <boost/cstdint.hpp>
#include <boost/type_traits/add_const.hpp>
#include <boost/integer_traits.hpp>

namespace chaiscript 
{
  /// \brief A wrapper for holding any valid C++ type. All types in ChaiScript are Boxed_Value objects
  /// \sa chaiscript::boxed_cast
  class Boxed_Value
  {
    public:
      /**
       * used for explicitly creating a "void" object
       */
      struct Void_Type
      {
      };

    private:
      /**
       * structure which holds the internal state of a Boxed_Value
       */
      struct Data
      {
        template<typename T>
          static bool is_null(boost::any *a)
          {
            boost::shared_ptr<T> *ptr = boost::any_cast<boost::shared_ptr<T> >(a);
            return ptr->get() == 0;
          }

        Data(const Type_Info &ti,
            const boost::any &to,
            bool tr,
            const boost::function<bool (boost::any*)> &t_is_null = boost::function<bool (boost::any*)>())
          : m_type_info(ti), m_obj(to), 
          m_is_ref(tr), m_is_null(t_is_null)
        {
        }

        Data &operator=(const Data &rhs)
        {
          m_type_info = rhs.m_type_info;
          m_obj = rhs.m_obj;
          m_is_ref = rhs.m_is_ref;
          m_is_null = rhs.m_is_null;

          return *this;
        }

        ~Data()
        {
        }

        Type_Info m_type_info;
        boost::any m_obj;
        bool m_is_ref;
        boost::function<bool (boost::any*)> m_is_null;
        std::vector<boost::shared_ptr<Data> > m_dependencies;
      };

      struct Object_Data
      {
        static boost::shared_ptr<Data> get(Boxed_Value::Void_Type)
        {
          return boost::shared_ptr<Data> (new Data(
                detail::Get_Type_Info<void>::get(),
                boost::any(), 
                false)
              );
        }

        template<typename T>
          static boost::shared_ptr<Data> get(const boost::shared_ptr<T> *obj)
          {
            return get(*obj);
          }

        template<typename T>
          static boost::shared_ptr<Data> get(const boost::shared_ptr<T> &obj)
          {
            return boost::shared_ptr<Data>(new Data(
                  detail::Get_Type_Info<T>::get(), 
                  boost::any(obj), 
                  false,
                  boost::function<bool (boost::any *)>(&Data::is_null<T>))
                );
          }

        template<typename T>
          static boost::shared_ptr<Data> get(T *t)
          {
            return get(boost::ref(*t));
          }

        template<typename T>
          static boost::shared_ptr<Data> get(boost::reference_wrapper<T> obj)
          {
            return boost::shared_ptr<Data>(new Data(
                  detail::Get_Type_Info<T>::get(),
                  boost::any(obj), 
                  true)
                );
          }

        template<typename T>
          static boost::shared_ptr<Data> get(const T& t)
          {
            return boost::shared_ptr<Data>(new Data(
                  detail::Get_Type_Info<T>::get(), 
                  boost::any(boost::shared_ptr<T>(new T(t))), 
                  false,
                  boost::function<bool (boost::any *)>(&Data::is_null<T>))
                );
          }

        static boost::shared_ptr<Data> get()
        {
          return boost::shared_ptr<Data> (new Data(
                Type_Info(),
                boost::any(),
                false)
              );
        }

      };

    public:
      /**
       * Basic Boxed_Value constructor
       */
      template<typename T>
        explicit Boxed_Value(T t)
        : m_data(Object_Data::get(t))
        {
        }

      /**
       * Copy constructor - each copy shares the same data pointer
       */
      Boxed_Value(const Boxed_Value &t_so)
        : m_data(t_so.m_data)
      {
      }

      /**
       * Unknown-type constructor
       */
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

      /**
       * copy the values stored in rhs.m_data to m_data
       * m_data pointers are not shared in this case
       */
      Boxed_Value assign(const Boxed_Value &rhs)
      {
        (*m_data) = (*rhs.m_data);
        return *this;
      }

      /**
       * shared data assignment, same as copy construction
       */
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

      /**
       * return true if the object is uninitialized
       */
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
        if (m_data->m_is_null)
        {
          return m_data->m_is_null(&m_data->m_obj);
        } else {
          return false;
        }
      }

      const boost::any & get() const
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

      void clear_dependencies()
      {
        m_data->m_dependencies.clear();
      }

      template<typename InItr>
        void add_dependencies(InItr begin, const InItr &end)
        {
          while (begin != end)
          {
            if (begin->m_data != m_data)
            {
              m_data->m_dependencies.push_back(begin->m_data);
            }
            ++begin;
          }
        }


    private:
      boost::shared_ptr<Data> m_data;
  };

  /// \brief Creates a Boxed_Value. If the object passed in is a value type, it is copied. If it is a pointer, boost::shared_ptr, or boost::reference_type
  ///        a copy is not made.
  /// \param t The value to box
  /// 
  /// Example:
  /// \code
  /// int i;
  /// chaiscript::ChaiScript chai;
  /// chai.add(chaiscript::var(i), "i");
  /// chai.add(chaiscript::var(&i), "ip");
  /// \endcode
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
        return Boxed_Value(boost::shared_ptr<typename boost::add_const<T>::type >(new T(t)));
      }

    /// \brief Takes a pointer to a value, adds const to the pointed to type and returns an immutable Boxed_Value.
    ///        Does not copy the pointed to value.
    /// \param[in] t Pointer to make immutable
    /// \returns Immutable Boxed_Value
    /// \sa Boxed_Value::is_const
    template<typename T>
      Boxed_Value const_var_impl(T *t)
      {
        return Boxed_Value( const_cast<typename boost::add_const<T>::type *>(t) );
      }

    /// \brief Takes a boost::shared_ptr to a value, adds const to the pointed to type and returns an immutable Boxed_Value.
    ///        Does not copy the pointed to value.
    /// \param[in] t Pointer to make immutable
    /// \returns Immutable Boxed_Value
    /// \sa Boxed_Value::is_const
    template<typename T>
      Boxed_Value const_var_impl(const boost::shared_ptr<T> &t)
      {
        return Boxed_Value( boost::const_pointer_cast<typename boost::add_const<T>::type>(t) );
      }

    /// \brief Takes a boost::reference_wrapper value, adds const to the referenced type and returns an immutable Boxed_Value.
    ///        Does not copy the referenced value.
    /// \param[in] t Reference object to make immutable
    /// \returns Immutable Boxed_Value
    /// \sa Boxed_Value::is_const
    template<typename T>
      Boxed_Value const_var_impl(const boost::reference_wrapper<T> &t)
      {
        return Boxed_Value( boost::cref(t.get()) );
      }
  }

  /// \brief Takes an object and returns an immutable Boxed_Value. If the object is a boost::reference or pointer type
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
  template<typename T>
    Boxed_Value const_var(const T &t)
    {
      return detail::const_var_impl(t);
    }



  /// \returns true if the two Boxed_Values share the same internal type
  static bool type_match(Boxed_Value l, Boxed_Value r)
  {
    return l.get_type_info() == r.get_type_info();
  }
}

#endif

