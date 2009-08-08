// This file is distributed under the BSD License.
// See "license.txt" for details.
// Copyright 2009, Jonathan Turner (jturner@minnow-lang.org)
// and Jason Turner (lefticus@gmail.com)
// http://www.chaiscript.com

#ifndef __handle_return_hpp__
#define __handle_return_hpp__

#include "boxed_value.hpp"
#include "type_info.hpp"
#include <string>
#include <boost/function.hpp>

#include <stdexcept>
#include <vector>

namespace chaiscript
{
  /**
   * Used internally for handling a return value from a Proxy_Function call
   */
  template<typename Ret>
    struct Handle_Return
    {
      static Boxed_Value call(const boost::function<Ret ()> &f)
      {
        return Boxed_Value(f());
      }
    };

  template<typename Ret>
    struct Handle_Return<boost::shared_ptr<Ret> &>
    {
      static Boxed_Value call(const boost::function<boost::shared_ptr<Ret> & ()> &f)
      {
        return Boxed_Value(f());
      }
    };

  template<typename Ret>
    struct Handle_Return<const boost::shared_ptr<Ret> &>
    {
      static Boxed_Value call(const boost::function<const boost::shared_ptr<Ret> & ()> &f)
      {
        return Boxed_Value(f());
      }
    };

  /**
   * Used internally for handling a return value from a Proxy_Function call
   */
  template<typename Ret>
    struct Handle_Return<Ret &>
    {
      static Boxed_Value call(const boost::function<Ret &()> &f)
      {
        return Boxed_Value(boost::ref(f()));
      }
    };

  /**
   * Used internally for handling a return value from a Proxy_Function call
   */
  template<>
    struct Handle_Return<Boxed_Value>
    {
      static Boxed_Value call(const boost::function<Boxed_Value ()> &f)
      {
        return f();
      }
    };

  /**
   * Used internally for handling a return value from a Proxy_Function call
   */
  template<>
    struct Handle_Return<Boxed_Value &>
    {
      static Boxed_Value call(const boost::function<Boxed_Value &()> &f)
      {
        return f();
      }
    };

  /**
   * Used internally for handling a return value from a Proxy_Function call
   */
  template<>
    struct Handle_Return<void>
    {
      static Boxed_Value call(const boost::function<void ()> &f)
      {
        f();
        return Boxed_Value(Boxed_Value::Void_Type());
      }
    };
}

#endif
