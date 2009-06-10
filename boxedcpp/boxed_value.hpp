#ifndef __boxed_value_hpp__
#define __boxed_value_hpp__

#include "type_info.hpp"
#include <boost/shared_ptr.hpp>
#include <boost/any.hpp>

#include <boost/ref.hpp>

class Boxed_Value
{
  private:
    struct Data
    {
      Data(const Type_Info &ti,
           const boost::any &to,
           bool tr)
        : m_type_info(ti), m_obj(to), m_is_ref(tr)
      {
      }

      Data &operator=(const Data &rhs)
      {
        m_type_info = rhs.m_type_info;
        m_obj = rhs.m_obj;
        m_is_ref = rhs.m_is_ref;

        return *this;
      }

      Type_Info m_type_info;
      boost::any m_obj;
      bool m_is_ref;
    };

  public:
    struct Void_Type
    {
    };

    template<typename T>
      explicit Boxed_Value(boost::shared_ptr<T> obj)
       : m_data(new Data(
           Get_Type_Info<T>()(), 
           boost::any(obj), 
           false)
         )
      {
      }

    template<typename T>
      explicit Boxed_Value(boost::reference_wrapper<T> obj)
       : m_data(new Data(
             Get_Type_Info<T>()(),
             boost::any(obj), 
             true)
           )
      {
      }

    template<typename T>
      explicit Boxed_Value(const T& t)
       : m_data(new Data(
             Get_Type_Info<T>()(), 
             boost::any(boost::shared_ptr<T>(new T(t))), 
             false)
           )
      {
      }

    Boxed_Value(Boxed_Value::Void_Type)
      : m_data(new Data(
            Get_Type_Info<void>()(),
            boost::any(), 
            false)
          )
    {
    }

    Boxed_Value(const Boxed_Value &t_so)
      : m_data(t_so.m_data)
    {
    }

    Boxed_Value()
      : m_data(new Data(
            Type_Info(),
            boost::any(),
            false)
          )
    {
    }

    Boxed_Value assign(const Boxed_Value &rhs)
    {
      (*m_data) = (*rhs.m_data);
      return *this;
    }

    Boxed_Value &operator=(const Boxed_Value &rhs)
    {
      m_data = rhs.m_data;
      /*
      std::cout << "operator= called" << std::endl;
      m_data->m_obj = rhs.m_data->m_obj;
      m_data->m_type_info = rhs.m_data->m_type_info;
      m_data->m_is_ref = rhs.m_data->m_is_ref;
      (*m_data) = (*rhs.m_data);
      */
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
  typename boost::reference_wrapper<typename boost::add_const<Result>::type > operator()(Boxed_Value ob)
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
  typename boost::reference_wrapper<typename boost::add_const<Result>::type > operator()(Boxed_Value ob)
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
  const Result *operator()(Boxed_Value ob)
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
  Result *operator()(Boxed_Value ob)
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
  typename boost::reference_wrapper<Result> operator()(Boxed_Value ob)
  {
    if (ob.is_ref())
    {
      return boost::any_cast<boost::reference_wrapper<Result> >(ob.get());
    } else {
      return boost::ref(*(boost::any_cast<boost::shared_ptr<Result> >(ob.get())));
    }
  }
};

template<>
struct Cast_Helper<Boxed_Value>
{
  Boxed_Value operator()(Boxed_Value ob)
  {
    return ob;    
  }
};


struct Boxed_POD_Value
{
  Boxed_POD_Value(const Boxed_Value &v)
    : d(0), i(0), m_isfloat(false)
  {
    const int inp_ = int(v.get_type_info().m_type_info);

    const int char_ = int(&typeid(char));
    const int bool_ = int(&typeid(bool));

    const int double_ = int(&typeid(double));
    const int float_ = int(&typeid(float));

    const int long_ = int(&typeid(long));
    const int unsigned_long_ = int(&typeid(unsigned long));
    const int int_ = int(&typeid(int));
    const int unsigned_int_ = int(&typeid(unsigned int));

    const int uint8_t_ = int(&typeid(uint8_t));
    const int uint16_t_ = int(&typeid(uint16_t));
    const int uint32_t_ = int(&typeid(uint32_t));
//    const int uint64_t_ = int(&typeid(uint64_t));

    const int int8_t_ = int(&typeid(int8_t));
    const int int16_t_ = int(&typeid(int16_t));
    const int int32_t_ = int(&typeid(int32_t));
    const int int64_t_ = int(&typeid(int64_t));

    if (inp_ == double_)
    {
      d = Cast_Helper<double>()(v);
      m_isfloat = true;
    } else if (inp_ == float_) {
      d = Cast_Helper<float>()(v);
      m_isfloat = true;
    } else if (inp_ == bool_ ) {
      i = Cast_Helper<bool>()(v);
    } else if (inp_ == char_) {
      i = Cast_Helper<char>()(v);
    } else if (inp_ == int_) {
      i = Cast_Helper<int>()(v);
    } else if (inp_ == unsigned_int_) {
      i = Cast_Helper<unsigned int>()(v);
    } else if (inp_ == long_) {
      i = Cast_Helper<long>()(v);
    } else if (inp_ == unsigned_long_) {
      i = Cast_Helper<unsigned long>()(v);
    } else if (inp_ == int8_t_) {
      i = Cast_Helper<int8_t>()(v);
    } else if (inp_ == int16_t_) {
      i = Cast_Helper<int16_t>()(v);
    } else if (inp_ == int32_t_) {
      i = Cast_Helper<int32_t>()(v);
    } else if (inp_ == int64_t_) {
      i = Cast_Helper<int64_t>()(v);
    } else if (inp_ == uint8_t_) {
      i = Cast_Helper<uint8_t>()(v);
    } else if (inp_ == uint16_t_) {
      i = Cast_Helper<uint16_t>()(v);
    } else if (inp_ == uint32_t_) {
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




#endif

