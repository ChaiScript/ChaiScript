#ifndef __scripting_object_hpp__
#define __scripting_object_hpp__

#include "scripting_type_info.hpp"
#include <boost/shared_ptr.hpp>
#include <boost/any.hpp>

#include <boost/ref.hpp>

class Scripting_Object
{
  public:
    template<typename T>
      explicit Scripting_Object(boost::shared_ptr<T> obj)
       : m_type_info(Get_Type_Info<T>()()), m_obj(obj), m_is_ref(false)
      {
      }

    template<typename T>
      explicit Scripting_Object(boost::reference_wrapper<T> obj)
       : m_type_info(Get_Type_Info<T>()()), m_obj(obj), m_is_ref(true)
      {
      }

    template<typename T>
      explicit Scripting_Object(const T& t)
       : m_type_info(Get_Type_Info<T>()()), m_obj(boost::shared_ptr<T>(new T(t))), m_is_ref(false)
      {
      }

    Scripting_Object(const Scripting_Object &t_so)
      : m_type_info(t_so.m_type_info), m_obj(t_so.m_obj), m_is_ref(t_so.m_is_ref)
    {
    }

    Scripting_Object()
      : m_type_info(Get_Type_Info<void>()()), m_is_ref(false)
    {
    }

    const Type_Info &get_type_info()
    {
      return m_type_info;
    }

    boost::any get() const
    {
      return m_obj;
    }

    bool is_ref() const
    {
      return m_is_ref;
    }

  private:
    Type_Info m_type_info;
    boost::any m_obj;
    bool m_is_ref;
};


//cast_help specializations
template<typename Result>
struct Cast_Helper
{
  typename boost::reference_wrapper<typename boost::add_const<Result>::type > operator()(Scripting_Object ob)
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
  typename boost::reference_wrapper<typename boost::add_const<Result>::type > operator()(Scripting_Object ob)
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
struct Cast_Helper<Result *>
{
  Result *operator()(Scripting_Object ob)
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
  typename boost::reference_wrapper<Result> operator()(Scripting_Object ob)
  {
    if (ob.is_ref())
    {
      return boost::any_cast<boost::reference_wrapper<Result> >(ob.get());
    } else {
      return boost::ref(*(boost::any_cast<boost::shared_ptr<Result> >(ob.get())));
    }
  }
};

#endif

