#include <boost/preprocessor.hpp>

#define gettypeinfo(z,n,text)  ti.push_back(Get_Type_Info<Param ## n>()());
#define casthelper(z,n,text) ,Cast_Helper<Param ## n>()(params[n]) 


#ifndef  BOOST_PP_IS_ITERATING
#ifndef __proxy_functions_hpp__
#define __proxy_functions_hpp__

#include "boxed_value.hpp"
#include "type_info.hpp"
#include <string>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <stdexcept>
#include <vector>

// handle_return implementations
template<typename Ret>
struct Handle_Return
{
  Boxed_Value operator()(const boost::function<Ret ()> &f)
  {
    return Boxed_Value(f());
  }
};

template<typename Ret>
struct Handle_Return<Ret &>
{
  Boxed_Value operator()(const boost::function<Ret &()> &f)
  {
    return Boxed_Value(boost::ref(f()));
  }
};

template<>
struct Handle_Return<void>
{
  Boxed_Value operator()(const boost::function<void ()> &f)
  {
    f();
    return Boxed_Value();
  }
};


// Build param type list (variadic)
template<typename Ret>
std::vector<Type_Info> build_param_type_list(const boost::function<Ret ()> &f)
{
  std::vector<Type_Info> ti;
  ti.push_back(Get_Type_Info<Ret>()());
  return ti;
}

// call_func implementations (variadic)
template<typename Ret>
Boxed_Value call_func(const boost::function<Ret ()> &f, const std::vector<Boxed_Value> &params)
{
  if (params.size() != 0)
  {
    throw std::range_error("Incorrect number of parameters");
  } else {
    return Handle_Return<Ret>()(f);
  }
}


struct Param_List_Builder
{
  Param_List_Builder &operator<<(const Boxed_Value &so)
  {
    objects.push_back(so);
    return *this;
  }

  template<typename T>
    Param_List_Builder &operator<<(T t)
    {
      objects.push_back(Boxed_Value(t));
      return *this;
    }

  operator const std::vector<Boxed_Value> &() const
  {
    return objects;
  }

  std::vector<Boxed_Value> objects;
  
};


#define BOOST_PP_ITERATION_LIMITS ( 1, 10 )
#define BOOST_PP_FILENAME_1 "proxy_functions.hpp"
#include BOOST_PP_ITERATE()

class Proxy_Function
{
  public:
    virtual Boxed_Value operator()(const std::vector<Boxed_Value> &params) = 0;
    virtual std::vector<Type_Info> get_param_types() = 0;

};

template<typename Func>
class Proxy_Function_Impl : public Proxy_Function
{
  public:
    Proxy_Function_Impl(const Func &f)
      : m_f(f)
    {
    }

    virtual Boxed_Value operator()(const std::vector<Boxed_Value> &params)
    {
      return call_func(m_f, params);
    }

    virtual std::vector<Type_Info> get_param_types()
    {
      return build_param_type_list(m_f);
    }

  private:
    Func m_f;
};



# endif
#else
# define n BOOST_PP_ITERATION()
 
template<typename Ret, BOOST_PP_ENUM_PARAMS(n, typename Param) >
std::vector<Type_Info> build_param_type_list(const boost::function<Ret (BOOST_PP_ENUM_PARAMS(n, Param))> &f)
{
  std::vector<Type_Info> ti;
  ti.push_back(Get_Type_Info<Ret>()());

  BOOST_PP_REPEAT(n, gettypeinfo, ~)

  return ti;
}

template<typename Ret, BOOST_PP_ENUM_PARAMS(n, typename Param)>
Boxed_Value call_func(const boost::function<Ret (BOOST_PP_ENUM_PARAMS(n, Param))> &f, 
    const std::vector<Boxed_Value> &params)
{
  if (params.size() != n)
  {
    throw std::range_error("Incorrect number of parameters");
  } else {
    return Handle_Return<Ret>()(boost::bind(f BOOST_PP_REPEAT(n, casthelper, ~)));
  }
}

#endif

