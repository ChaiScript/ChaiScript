// This file is distributed under the BSD License.
// See "license.txt" for details.
// Copyright 2009, Jonathan Turner (jturner@minnow-lang.org)
// and Jason Turner (lefticus@gmail.com)
// http://www.chaiscript.com

#ifndef __register_function_hpp__
#define __register_function_hpp__

#include "dispatchkit.hpp"
#include "bind_first.hpp"
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/function_types/components.hpp>
#include <boost/function_types/function_type.hpp>

namespace chaiscript
{
  namespace detail
  {
    /**
     * Helper function for register_member function
     */
    template<typename T, typename Class>
      T &get_member(T Class::* m, Class *obj)
      {
        return (obj->*m);
      }

    template<typename T>
    boost::function<T> mk_boost_fun(const boost::function<T> &f)
    {
      return f;
    }

    template<typename T>
    boost::function< 
             typename boost::function_types::function_type<boost::function_types::components<T> >::type 
          >  mk_boost_fun(T t)
    {
      return 
          boost::function< 
             typename boost::function_types::function_type<boost::function_types::components<T> >::type 
          >(t);
    }

    template<typename T, typename Class>
      Proxy_Function fun_helper(T Class::* m)
      {
        return fun_helper(boost::function<T& (Class *)>(boost::bind(&detail::get_member<T, Class>, m, _1)));
      }


    template<typename T>
      Proxy_Function fun_helper(const boost::function<T> &f)
      {
        return Proxy_Function(new Proxy_Function_Impl<T>(f));
      }
  }

  template<typename T>
    Proxy_Function fun(const boost::function<T> &f)
    {
      return detail::fun_helper(f);
    }

  template<typename T>
    Proxy_Function fun(T t)
    {
      return detail::fun_helper(detail::mk_boost_fun(t));
    }


  template<typename T, typename Q>
    Proxy_Function fun(T t, const Q &q)
    {
      return detail::fun_helper(detail::bind_first(t, q));
    }

  template<typename T, typename Q, typename R>
    Proxy_Function fun(T t, const Q &q, const R &r)
    {
      return detail::fun_helper(detail::bind_first(detail::bind_first(t, q), r));
    }
   
}


#endif

