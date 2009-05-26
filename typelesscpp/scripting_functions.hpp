#include <boost/preprocessor.hpp>

#define gettypeinfo(z,n,text)  ti.push_back(Get_Type_Info<Param ## n>()());
#define casthelper(z,n,text) ,Cast_Helper<Param ## n>()(params[n]) 


#ifndef  BOOST_PP_IS_ITERATING
#ifndef __scripting_functions_hpp__
#define __scripting_functions_hpp__

#include "scripting_object.hpp"
#include "scripting_type_info.hpp"
#include <string>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <stdexcept>
#include <vector>

// handle_return implementations
template<typename Ret>
struct Handle_Return
{
  Scripting_Object operator()(const boost::function<Ret ()> &f)
  {
    return Scripting_Object(f());
  }
};

template<typename Ret>
struct Handle_Return<Ret &>
{
  Scripting_Object operator()(const boost::function<Ret &()> &f)
  {
    return Scripting_Object(boost::ref(f()));
  }
};

template<>
struct Handle_Return<void>
{
  Scripting_Object operator()(const boost::function<void ()> &f)
  {
    f();
    return Scripting_Object();
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
Scripting_Object call_func(const boost::function<Ret ()> &f, const std::vector<Scripting_Object> &params)
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
  Param_List_Builder &operator<<(const Scripting_Object &so)
  {
    objects.push_back(so);
    return *this;
  }

  template<typename T>
    Param_List_Builder &operator<<(T t)
    {
      objects.push_back(Scripting_Object(t));
      return *this;
    }

  operator const std::vector<Scripting_Object> &() const
  {
    return objects;
  }

  std::vector<Scripting_Object> objects;
  
};


#define BOOST_PP_ITERATION_LIMITS ( 1, 10 )
#define BOOST_PP_FILENAME_1 "scripting_functions.hpp"
#include BOOST_PP_ITERATE()

class Function_Handler
{
  public:
    virtual Scripting_Object operator()(const std::vector<Scripting_Object> &params) = 0;
    virtual std::vector<Type_Info> get_param_types() = 0;

};

template<typename Func>
class Function_Handler_Impl : public Function_Handler
{
  public:
    Function_Handler_Impl(const Func &f)
      : m_f(f)
    {
    }

    virtual Scripting_Object operator()(const std::vector<Scripting_Object> &params)
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
Scripting_Object call_func(const boost::function<Ret (BOOST_PP_ENUM_PARAMS(n, Param))> &f, 
    const std::vector<Scripting_Object> &params)
{
  if (params.size() != n)
  {
    throw std::range_error("Incorrect number of parameters");
  } else {
    return Handle_Return<Ret>()(boost::bind(f BOOST_PP_REPEAT(n, casthelper, ~)));
  }
}

#endif

