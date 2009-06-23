#ifndef __boxed_value_hpp__
#define __boxed_value_hpp__

#include "type_info.hpp"
#include <map>
#include <boost/shared_ptr.hpp>
#include <boost/any.hpp>
#include <boost/function.hpp>
#include <boost/ref.hpp>
#include <boost/bind.hpp>

namespace dispatchkit
{
  class Boxed_Value
  {
    public:
      struct Void_Type
      {
      };

    private:
      struct Data
      {
        struct Shared_Ptr_Proxy
        {
          virtual ~Shared_Ptr_Proxy()
          {
          }

          virtual bool unique(boost::any *) = 0;
          virtual long use_count(boost::any *) = 0;
        };

        template<typename T>
          struct Shared_Ptr_Proxy_Impl : Shared_Ptr_Proxy
        {
          virtual ~Shared_Ptr_Proxy_Impl()
          {
          }

          virtual bool unique(boost::any *a)
          {
            boost::shared_ptr<T> *ptr = boost::any_cast<boost::shared_ptr<T> >(a);
            return ptr->unique();
          }

          virtual long use_count(boost::any *a)
          {
            boost::shared_ptr<T> *ptr = boost::any_cast<boost::shared_ptr<T> >(a);
            return ptr->use_count();
          }
        };

        Data(const Type_Info &ti,
            const boost::any &to,
            bool tr,
            const boost::shared_ptr<Shared_Ptr_Proxy> &t_proxy = boost::shared_ptr<Shared_Ptr_Proxy>())
          : m_type_info(ti), m_obj(to), 
          m_is_ref(tr), m_ptr_proxy(t_proxy)
        {
        }

        Data &operator=(const Data &rhs)
        {
          m_type_info = rhs.m_type_info;
          m_obj = rhs.m_obj;
          m_is_ref = rhs.m_is_ref;
          m_ptr_proxy = rhs.m_ptr_proxy;

          return *this;
        }

        static bool get_false()
        {
          return false;
        }

        Type_Info m_type_info;
        boost::any m_obj;
        bool m_is_ref;
        boost::shared_ptr<Shared_Ptr_Proxy> m_ptr_proxy;
      };

      struct Object_Cache
      {
        boost::shared_ptr<Data> get(Boxed_Value::Void_Type)
        {
          return boost::shared_ptr<Data> (new Data(
                Get_Type_Info<void>::get(),
                boost::any(), 
                false)
              );
        }

        template<typename T>
          boost::shared_ptr<Data> get(boost::shared_ptr<T> obj)
          {
            boost::shared_ptr<Data> data(new Data(
                  Get_Type_Info<T>::get(), 
                  boost::any(obj), 
                  false,
                  boost::shared_ptr<Data::Shared_Ptr_Proxy>(new Data::Shared_Ptr_Proxy_Impl<T>()))
                );

            std::map<void *, Data >::iterator itr
              = m_ptrs.find(obj.get());

            if (itr != m_ptrs.end())
            {
              (*data) = (itr->second);
            } else {
              m_ptrs.insert(std::make_pair(obj.get(), *data));
            }

            return data;
          }

        template<typename T>
          boost::shared_ptr<Data> get(boost::reference_wrapper<T> obj)
          {
            boost::shared_ptr<Data> data(new Data(
                  Get_Type_Info<T>::get(),
                  boost::any(obj), 
                  true)
                );

            std::map<void *, Data >::iterator itr
              = m_ptrs.find(obj.get_pointer());

            if (itr != m_ptrs.end())
            {
//              std::cout << "Reference wrapper ptr found, using it" << std::endl;
              (*data) = (itr->second);
            } 

            return data;
          }

        template<typename T>
          boost::shared_ptr<Data> get(const T& t)
          {
            boost::shared_ptr<Data> data(new Data(
                  Get_Type_Info<T>::get(), 
                  boost::any(boost::shared_ptr<T>(new T(t))), 
                  false,
                  boost::shared_ptr<Data::Shared_Ptr_Proxy>(new Data::Shared_Ptr_Proxy_Impl<T>()))
                );

            boost::shared_ptr<T> *ptr = boost::any_cast<boost::shared_ptr<T> >(&data->m_obj);

            m_ptrs.insert(std::make_pair(ptr->get(), *data));
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

        void cull()
        {
          std::map<void *, Data >::iterator itr = m_ptrs.begin();

          while (itr != m_ptrs.end())
          {
            if (itr->second.m_ptr_proxy->unique(&itr->second.m_obj) == 1)
            {
              std::map<void *, Data >::iterator todel = itr;
              //          std::cout << "Releasing unique ptr " << std::endl;
              ++itr;
              m_ptrs.erase(todel);
            } else {
              ++itr;
            }
          }

          //      std::cout << "References held: " << m_ptrs.size() << std::endl;
        }


        std::map<void *, Data > m_ptrs;
      };

    public:
      template<typename T>
        explicit Boxed_Value(T t)
        : m_data(get_object_cache().get(t))
        {
          get_object_cache().cull();
        }

      Boxed_Value(const Boxed_Value &t_so)
        : m_data(t_so.m_data)
      {
      }

      Boxed_Value()
        : m_data(get_object_cache().get())    
      {
      }

      ~Boxed_Value()
      {
      }


      Object_Cache &get_object_cache()
      {
        static Object_Cache oc;
        return oc;
      }    


      Boxed_Value assign(const Boxed_Value &rhs)
      {
        (*m_data) = (*rhs.m_data);
        return *this;
      }

      Boxed_Value &operator=(const Boxed_Value &rhs)
      {
        m_data = rhs.m_data;
        return *this;
      }

      const Type_Info &get_type_info() const
      {
        return m_data->m_type_info;
      }

      bool is_unknown() const
      {
        return m_data->m_type_info.m_is_unknown;
      }

      boost::any get() const
      {
        return m_data->m_obj;
      }

      bool is_ref() const
      {
        return m_data->m_is_ref;
      }

    private:
      boost::shared_ptr<Data> m_data;
  };


  //cast_help specializations
  template<typename Result>
    struct Cast_Helper
    {
      typedef typename boost::reference_wrapper<typename boost::add_const<Result>::type > Result_Type;

      Result_Type operator()(Boxed_Value ob)
      {
        if (ob.is_ref())
        {
          return boost::cref((boost::any_cast<boost::reference_wrapper<Result> >(ob.get())).get());
        } else {
          return boost::cref(*(boost::any_cast<boost::shared_ptr<Result> >(ob.get())));   
        }
      }
    };

  template<typename Result>
    struct Cast_Helper<const Result &>
    {
      typedef typename boost::reference_wrapper<typename boost::add_const<Result>::type > Result_Type;
        
      Result_Type operator()(Boxed_Value ob)
      {
        if (ob.is_ref())
        {
          return boost::cref((boost::any_cast<boost::reference_wrapper<Result> >(ob.get())).get());
        } else {
          return boost::cref(*(boost::any_cast<boost::shared_ptr<Result> >(ob.get())));   
        }
      }
    };

  template<typename Result>
    struct Cast_Helper<const Result *>
    {
      typedef const Result * Result_Type;

      Result_Type operator()(Boxed_Value ob)
      {
        if (ob.is_ref())
        {
          return (boost::any_cast<boost::reference_wrapper<Result> >(ob.get())).get_pointer();
        } else {
          return (boost::any_cast<boost::shared_ptr<Result> >(ob.get())).get();
        }
      }
    };

  template<typename Result>
    struct Cast_Helper<Result *>
    {
      typedef Result * Result_Type;

      Result_Type operator()(Boxed_Value ob)
      {
        if (ob.is_ref())
        {
          return (boost::any_cast<boost::reference_wrapper<Result> >(ob.get())).get_pointer();
        } else {
          return (boost::any_cast<boost::shared_ptr<Result> >(ob.get())).get();
        }
      }
    };

  template<typename Result>
    struct Cast_Helper<Result &>
    {
      typedef typename boost::reference_wrapper<Result> Result_Type;

      Result_Type operator()(Boxed_Value ob)
      {
        if (ob.is_ref())
        {
          return boost::any_cast<boost::reference_wrapper<Result> >(ob.get());
        } else {
          return boost::ref(*(boost::any_cast<boost::shared_ptr<Result> >(ob.get())));
        }
      }
    };

  template<typename Result>
    struct Cast_Helper<typename boost::shared_ptr<Result> >
    {
      typedef typename boost::shared_ptr<Result> Result_Type;
        
      Result_Type operator()(Boxed_Value ob)
      {
        return boost::any_cast<boost::shared_ptr<Result> >(ob.get());
      }
    };


  template<>
    struct Cast_Helper<Boxed_Value>
    {
      typedef Boxed_Value Result_Type;

      Result_Type operator()(Boxed_Value ob)
      {
        return ob;    
      }
    };


  struct Boxed_POD_Value
  {
    Boxed_POD_Value(const Boxed_Value &v)
      : d(0), i(0), m_isfloat(false)
    {
      if (!v.get_type_info().m_type_info)
      {
        throw boost::bad_any_cast();
      }

      const std::type_info &inp_ = *v.get_type_info().m_type_info;

      if (inp_ == typeid(double))
      {
        d = Cast_Helper<double>()(v);
        m_isfloat = true;
      } else if (inp_ == typeid(float)) {
        d = Cast_Helper<float>()(v);
        m_isfloat = true;
      } else if (inp_ == typeid(bool)) {
        i = Cast_Helper<bool>()(v);
      } else if (inp_ == typeid(char)) {
        i = Cast_Helper<char>()(v);
      } else if (inp_ == typeid(int)) {
        i = Cast_Helper<int>()(v);
      } else if (inp_ == typeid(unsigned int)) {
        i = Cast_Helper<unsigned int>()(v);
      } else if (inp_ == typeid(long)) {
        i = Cast_Helper<long>()(v);
      } else if (inp_ == typeid(unsigned long)) {
        i = Cast_Helper<unsigned long>()(v);
      } else if (inp_ == typeid(int8_t)) {
        i = Cast_Helper<int8_t>()(v);
      } else if (inp_ == typeid(int16_t)) {
        i = Cast_Helper<int16_t>()(v);
      } else if (inp_ == typeid(int32_t)) {
        i = Cast_Helper<int32_t>()(v);
      } else if (inp_ == typeid(int64_t)) {
        i = Cast_Helper<int64_t>()(v);
      } else if (inp_ == typeid(uint8_t)) {
        i = Cast_Helper<uint8_t>()(v);
      } else if (inp_ == typeid(uint16_t)) {
        i = Cast_Helper<uint16_t>()(v);
      } else if (inp_ == typeid(uint32_t)) {
        i = Cast_Helper<uint32_t>()(v);
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
        return Boxed_Value(i + r.i);
      }

      return Boxed_Value(((m_isfloat)?d:i) + ((r.m_isfloat)?r.d:r.i));
    }

    Boxed_Value operator*(const Boxed_POD_Value &r) const
    {
      if (!m_isfloat && !r.m_isfloat)
      {
        return Boxed_Value(i * r.i);
      }

      return Boxed_Value(((m_isfloat)?d:i) * ((r.m_isfloat)?r.d:r.i));
    }

    Boxed_Value operator/(const Boxed_POD_Value &r) const
    {
      if (!m_isfloat && !r.m_isfloat)
      {
        return Boxed_Value(i / r.i);
      }

      return Boxed_Value(((m_isfloat)?d:i) / ((r.m_isfloat)?r.d:r.i));
    }

    Boxed_Value operator-(const Boxed_POD_Value &r) const
    {
      if (!m_isfloat && !r.m_isfloat)
      {
        return Boxed_Value(i - r.i);
      }

      return Boxed_Value(((m_isfloat)?d:i) - ((r.m_isfloat)?r.d:r.i));
    }

    double d;
    int64_t i;

    bool m_isfloat;
  };

  template<>
    struct Cast_Helper<Boxed_POD_Value>
    {
      Boxed_POD_Value operator()(Boxed_Value ob)
      {
        return Boxed_POD_Value(ob);
      }
    };
}

#endif

