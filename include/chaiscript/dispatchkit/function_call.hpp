// This file is distributed under the BSD License.
// See "license.txt" for details.
// Copyright 2009-2010, Jonathan Turner (jturner@minnow-lang.org)
// and Jason Turner (lefticus@gmail.com)
// http://www.chaiscript.com

#ifndef __function_call_hpp__
#define __function_call_hpp__

#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <string>
#include <vector>
#include "proxy_functions.hpp"
#include "function_call_detail.hpp"

namespace chaiscript
{
  /**
   * Build a function caller that knows how to dispatch on a set of functions
   * example: 
   * boost::function<void (int)> f = 
   *      build_function_caller(dispatchkit.get_function("print"));
   * \returns A boost::function object for dispatching
   * \param[in] funcs the set of functions to dispatch on.
   */
  template<typename FunctionType>
    boost::function<FunctionType>
      functor(const std::vector<std::pair<std::string, Proxy_Function > > &funcs)
      {
        FunctionType *p=0;
        return detail::build_function_caller_helper(p, funcs);
      }

  /**
   * Build a function caller for a particular Proxy_Function object
   * useful in the case that a function is being pass out from scripting back
   * into code
   * example: 
   * void my_function(Proxy_Function f)
   * {
   *   boost::function<void (int)> local_f = 
   *      build_function_caller(f);
   * }
   * \returns A boost::function object for dispatching
   * \param[in] func A function to execute.
   */
  template<typename FunctionType>
    boost::function<FunctionType>
      functor(Proxy_Function func)
      {
        std::vector<std::pair<std::string, Proxy_Function > > funcs;
        funcs.push_back(std::make_pair(std::string(), func));
        return functor<FunctionType>(funcs);
      }

  /**
   * Helper for automatically unboxing a Boxed_Value that contains a function object
   * and creating a typesafe C++ function caller from it.
   */
  template<typename FunctionType>
    boost::function<FunctionType>
      functor(const Boxed_Value &bv)
      {
        return functor<FunctionType>(boxed_cast<Proxy_Function >(bv));
      }

}

#endif

