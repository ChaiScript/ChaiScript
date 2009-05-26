#ifndef __scripting_function_hpp__
#define __scripting_function_hpp__


#include "scripting_object.hpp"

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



// call_func implementations todo: handle reference return types
// to be made variadic
template<typename Ret, typename Param1, typename Param2>
Scripting_Object call_func(const boost::function<Ret (Param1, Param2)> &f, const std::vector<Scripting_Object> &params)
{
  if (params.size() != 2)
  {
    throw std::range_error("Incorrect number of parameters");
  } else {
    return Handle_Return<Ret>()(boost::bind(f, Cast_Helper<Param1>()(params[0]), Cast_Helper<Param2>()(params[1])));
  }
}

template<typename Ret, typename Param1>
Scripting_Object call_func(const boost::function<Ret (Param1)> &f, const std::vector<Scripting_Object> &params)
{
  if (params.size() != 1)
  {
    throw std::range_error("Incorrect number of parameters");
  } else {
    return Handle_Return<Ret>()(boost::bind(f, Cast_Helper<Param1>()(params[0])));
  }
}

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



class Function_Handler
{
  public:
    virtual Scripting_Object operator()(const std::vector<Scripting_Object> &params) = 0;
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

  private:
    Func m_f;
};

std::vector<Scripting_Object> build_param_list(const Scripting_Object &so)
{
  std::vector<Scripting_Object> sos;
  sos.push_back(so);
  return sos;
}
std::vector<Scripting_Object> build_param_list(const Scripting_Object &so1, const Scripting_Object &so2)
{
  std::vector<Scripting_Object> sos;
  sos.push_back(so1);
  sos.push_back(so2);
  return sos;
}
std::vector<Scripting_Object> build_param_list(const Scripting_Object &so1,  const Scripting_Object &so2, const Scripting_Object &so3)
{
  std::vector<Scripting_Object> sos;
  sos.push_back(so1);
  sos.push_back(so2);
  sos.push_back(so3);
  return sos;
}



#endif

