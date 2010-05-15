// This file is distributed under the BSD License.
// See "license.txt" for details.
// Copyright 2009-2010, Jonathan Turner (jturner@minnow-lang.org)
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
#include <boost/function_types/is_member_object_pointer.hpp>
#include <boost/function_types/is_member_function_pointer.hpp>

namespace chaiscript
{
  namespace detail
  {
    template<bool Object, bool MemFn>
      struct Fun_Helper
      {
        template<typename T>
        static Proxy_Function go(T t)
        {
          return Proxy_Function(
              new Proxy_Function_Impl<
                  typename boost::function_types::function_type<boost::function_types::components<T> >::type> (
                    boost::function< 
                      typename boost::function_types::function_type<boost::function_types::components<T> >::type 
                      >(t)));
        }      
      };

    template<>
      struct Fun_Helper<false, true>
      {
        template<typename T>
        static Proxy_Function go(T t)
        {
          return Proxy_Function(
              new Proxy_Function_Impl<
                  typename boost::function_types::function_type<boost::function_types::components<T> >::type> (
                    boost::function< 
                      typename boost::function_types::function_type<boost::function_types::components<T> >::type 
                    >(boost::mem_fn(t))));
        }      
      };


    template<>
      struct Fun_Helper<true, false>
      {
        template<typename T, typename Class>
          static Proxy_Function go(T Class::* m)
          {
            return Proxy_Function(new Attribute_Access<T, Class>(m));
          }
      };

  }

  template<typename T>
    Proxy_Function fun(const boost::function<T> &f)
    {
      return Proxy_Function(new Proxy_Function_Impl<T>(f));
    }

  template<typename T>
    Proxy_Function fun(T t)
    {
        return detail::Fun_Helper<boost::function_types::is_member_object_pointer<T>::value, boost::function_types::is_member_function_pointer<T>::value>::go(t);
    }

  template<typename T, typename Q>
    Proxy_Function fun(T t, const Q &q)
    {
      return fun(bind_first(t, q));
    }

  template<typename T, typename Q, typename R>
    Proxy_Function fun(T t, const Q &q, const R &r)
    {
      return fun(bind_first(bind_first(t, q), r));
    }
   
}


#endif

