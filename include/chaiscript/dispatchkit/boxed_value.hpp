// This file is distributed under the BSD License.
// See "license.txt" for details.
// Copyright 2009-2010, Jonathan Turner (jturner@minnow-lang.org)
// and Jason Turner (lefticus@gmail.com)
// http://www.chaiscript.com

#ifndef __boxed_value_hpp__
#define __boxed_value_hpp__

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
  /**
   * Boxed_Value is the main tool of the dispatchkit. It allows
   * for boxed / untyped containment of any C++ object. It uses
   * boost::any internally but also provides user access the underlying
   * stored type information
   */
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
          static bool unique(boost::any *a)
          {
            boost::shared_ptr<T> *ptr = boost::any_cast<boost::shared_ptr<T> >(a);
            return ptr->unique();
          }

        template<typename T>
          static bool is_null(boost::any *a)
          {
            boost::shared_ptr<T> *ptr = boost::any_cast<boost::shared_ptr<T> >(a);
            return ptr->get() == 0;
          }

        Data(const Type_Info &ti,
            const boost::any &to,
            bool tr,
            const boost::function<bool (boost::any*)> &t_unique = boost::function<bool (boost::any*)>(),
            const boost::function<bool (boost::any*)> &t_is_null = boost::function<bool (boost::any*)>())
          : m_type_info(ti), m_obj(to), 
          m_is_ref(tr), m_unique(t_unique), m_is_null(t_is_null)
        {
        }

        Data &operator=(const Data &rhs)
        {
          m_type_info = rhs.m_type_info;
          m_obj = rhs.m_obj;
          m_is_ref = rhs.m_is_ref;
          m_unique = rhs.m_unique;
          m_is_null = rhs.m_is_null;

          return *this;
        }

        Type_Info m_type_info;
        boost::any m_obj;
        bool m_is_ref;
        boost::function<bool (boost::any*)> m_unique;
        boost::function<bool (boost::any*)> m_is_null;
      };

      /**
       * Cache of all created objects in the dispatch kit. Used to return the 
       * same shared_ptr if the same object is created more than once.
       * Also used for acquiring a shared_ptr of a reference object, if the 
       * value of the shared_ptr is known
       */
      struct Object_Cache
      {
        struct Object_Cache_Key
        {
          Object_Cache_Key(const void *t_obj, bool t_const, const std::type_info *t_objtype)
            : m_obj(t_obj), m_const(t_const), m_objtype(t_objtype)
          {
          }

          bool operator<(const Object_Cache_Key &rhs) const
          {
            //Take into account the fact that sometimes there are equiv type_info objects
            //that have different pointer values, so we have to dereference to use the built in
            //comparison operator
            return m_obj < rhs.m_obj
                   || (m_obj == rhs.m_obj && m_const < rhs.m_const)
                   || (m_obj == rhs.m_obj && m_const == rhs.m_const 
                       && m_objtype < rhs.m_objtype && !((*m_objtype) == (*rhs.m_objtype)));
          }

          bool operator==(const Object_Cache_Key &rhs) const
          {
            return m_obj == rhs.m_obj
              && m_const == rhs.m_const 
              && (m_objtype == rhs.m_objtype || (*m_objtype) == (*rhs.m_objtype) );
          }



          const void *m_obj;
          bool m_const;
          const std::type_info *m_objtype;
        };

        Object_Cache()
          : m_cullcount(0)
        {
        }

        boost::shared_ptr<Data> get(Boxed_Value::Void_Type)
        {
          return boost::shared_ptr<Data> (new Data(
                detail::Get_Type_Info<void>::get(),
                boost::any(), 
                false)
              );
        }

        template<typename T>
          boost::shared_ptr<Data> get(const boost::shared_ptr<T> *obj)
          {
            return get(*obj);
          }

        template<typename T>
          boost::shared_ptr<Data> get(const boost::shared_ptr<T> &obj)
          {
            bool b_const = boost::is_const<T>::value; 
            const std::type_info *ti = &typeid(typename Bare_Type<T>::type);

            boost::shared_ptr<Data> data(new Data(
                  detail::Get_Type_Info<T>::get(), 
                  boost::any(obj), 
                  false,
                  &Data::unique<T>,
                  &Data::is_null<T>)
                );

            std::map<Object_Cache_Key, Data>::iterator itr
              = m_ptrs.find(Object_Cache_Key(obj.get(), b_const, ti));

            if (itr != m_ptrs.end())
            {
              (*data) = (itr->second);
            } else {
              m_ptrs.insert(std::make_pair(Object_Cache_Key(obj.get(), b_const, ti), *data));
            }

            return data;
          }

        template<typename T>
          boost::shared_ptr<Data> get(T *t)
          {
            return get(boost::ref(*t));
          }

        template<typename T>
          boost::shared_ptr<Data> get(boost::reference_wrapper<T> obj)
          {
            bool b_const = boost::is_const<T>::value; 
            const std::type_info *ti = &typeid(typename Bare_Type<T>::type);

            boost::shared_ptr<Data> data(new Data(
                  detail::Get_Type_Info<T>::get(),
                  boost::any(obj), 
                  true)
                );

            std::map<Object_Cache_Key, Data >::iterator itr
              = m_ptrs.find(Object_Cache_Key(obj.get_pointer(), b_const, ti) );

            // If the ptr is found in the cache and it is the correct type,
            // return it. It may be the incorrect type when two variables share the
            // same memory location. Example:
            // struct test { int x; };
            // test t;
            // Both t and t.x share the same memory location, but do not represent
            // objects of the same type.
            if (itr != m_ptrs.end() 
                && itr->second.m_type_info.bare_equal(data->m_type_info))
            {
              (*data) = (itr->second);
            } 

            return data;
          }

        template<typename T>
          boost::shared_ptr<Data> get(const T& t)
          {
            const std::type_info *ti = &typeid(typename Bare_Type<T>::type);

            boost::shared_ptr<Data> data(new Data(
                  detail::Get_Type_Info<T>::get(), 
                  boost::any(boost::shared_ptr<T>(new T(t))), 
                  false,
                  &Data::unique<T>,
                  &Data::is_null<T>)
                );

            boost::shared_ptr<T> *ptr = boost::any_cast<boost::shared_ptr<T> >(&data->m_obj);

            m_ptrs.insert(std::make_pair(Object_Cache_Key(ptr->get(), false, ti), *data));
            return data;
          }

        boost::shared_ptr<Data> get()
        {
          return boost::shared_ptr<Data> (new Data(
                Type_Info(),
                boost::any(),
                false)
              );
        }

        /**
         * Drop objects from the cache where there is only one (ie, our)
         * reference to it, so it may be destructed
         */
        void cull(bool force = false)
        {
          
          ++m_cullcount;
          if (force || m_cullcount % 20 != 0)
          {
            return;
          }

          std::map<Object_Cache_Key, Data>::iterator itr = m_ptrs.begin();

          while (itr != m_ptrs.end())
          {
            if (itr->second.m_unique(&itr->second.m_obj))
            {
              std::map<Object_Cache_Key, Data >::iterator todel = itr;
              ++itr;
              m_ptrs.erase(todel);
            } else {
              ++itr;
            }
          }
        }

        std::map<Object_Cache_Key, Data> m_ptrs;
        int m_cullcount;
      };

    public:
      /**
       * Basic Boxed_Value constructor
       */
      template<typename T>
        explicit Boxed_Value(T t)
        : m_data(get_object_cache().get(t))
        {
          get_object_cache().cull();
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
        : m_data(get_object_cache().get())    
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
       * Return a reference to the static global Object_Cache
       */
      static Object_Cache &get_object_cache()
      {
        static chaiscript::threading::Thread_Storage<Object_Cache> oc;
        return *oc;
      }   

      static void clear_cache()
      {
        get_object_cache().m_ptrs.clear();
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

      boost::any get() const
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


    private:
      boost::shared_ptr<Data> m_data;
  };

  template<typename T>
    Boxed_Value var(T t)
    {
      return Boxed_Value(t);
    }

  template<typename T>
    Boxed_Value const_var(T *t)
    {
      return Boxed_Value( const_cast<typename boost::add_const<T>::type>(t) );
    }

  template<typename T>
    Boxed_Value const_var(const boost::shared_ptr<T> &t)
    {
      return Boxed_Value( boost::const_pointer_cast<typename boost::add_const<T>::type>(t) );
    }

  template<typename T>
    Boxed_Value const_var(const boost::reference_wrapper<T> &t)
    {
      return Boxed_Value( boost::cref(t.get()) );
    }

  template<typename T>
    Boxed_Value const_var(const T &t)
    {
      return Boxed_Value(boost::shared_ptr<typename boost::add_const<T>::type >(new T(t)));
    }


  /**
   * Return true if the two Boxed_Values share the same internal type
   */
  static bool type_match(Boxed_Value l, Boxed_Value r)
  {
    return l.get_type_info() == r.get_type_info();
  }
}

#endif

