// This file is distributed under the BSD License.
// See LICENSE.TXT for details.
// Copyright 2009, Jonathan Turner (jonathan.d.turner@gmail.com) 
// and Jason Turner (lefticus@gmail.com)
// http://www.chaiscript.com

#include <boost/preprocessor.hpp>

#define addparam(z,n,text)  params.push_back(Boxed_Value(BOOST_PP_CAT(p, n) ));
#define curry(z,n,text)  BOOST_PP_CAT(_, BOOST_PP_INC(n))


#ifndef  BOOST_PP_IS_ITERATING
#ifndef __function_call_hpp__
#define __function_call_hpp__

#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <string>
#include <vector>
#include "proxy_functions.hpp"

namespace dispatchkit
{
  /**
   * Internal helper class for handling the return
   * value of a build_function_caller
   */
  template<typename Ret>
    class Function_Caller_Ret
    {
      public:
        Function_Caller_Ret()
        {
        }

        Ret call(const std::vector<std::pair<std::string, boost::shared_ptr<Proxy_Function> > > &t_funcs, 
                 const std::vector<Boxed_Value> &params)
        {
          return boxed_cast<Ret>(dispatch(t_funcs, params));
        }
    };

  /**
   * Specialization for void return types
   */
  template<>
    class Function_Caller_Ret<void>
    {
      public:
        Function_Caller_Ret()
        {
        }

        void call(const std::vector<std::pair<std::string, boost::shared_ptr<Proxy_Function> > > &t_funcs, 
                  const std::vector<Boxed_Value> &params)
        {
          dispatch(t_funcs, params);
        }
    };
}

#define BOOST_PP_ITERATION_LIMITS ( 0, 9 )
#define BOOST_PP_FILENAME_1 <chaiscript/dispatchkit/function_call.hpp>
#include BOOST_PP_ITERATE()

namespace dispatchkit
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
      build_function_caller(const std::vector<std::pair<std::string, boost::shared_ptr<Proxy_Function> > > &funcs)
      {
        FunctionType *p;
        return build_function_caller_helper(p, funcs);
      }

  /**
   * Build a function caller for a particular Proxy_Function object
   * useful in the case that a function is being pass out from scripting back
   * into code
   * example: 
   * void my_function(boost::shared_ptr<Proxy_Function> f)
   * {
   *   boost::function<void (int)> local_f = 
   *      build_function_caller(f);
   * }
   * \returns A boost::function object for dispatching
   * \param[in] func A function to execute.
   */
  template<typename FunctionType>
    boost::function<FunctionType>
      build_function_caller(boost::shared_ptr<Proxy_Function> func)
      {
        std::vector<std::pair<std::string, boost::shared_ptr<Proxy_Function> > > funcs;
        funcs.push_back(std::make_pair(std::string(), func));
        return build_function_caller<FunctionType>(funcs);
      }

  /**
   * Helper for automatically unboxing a Boxed_Value that contains a function object
   * and creating a typesafe C++ function caller from it.
   */
  template<typename FunctionType>
    boost::function<FunctionType>
      build_function_caller(const Boxed_Value &bv)
      {
        return build_function_caller<FunctionType>(boxed_cast<boost::shared_ptr<Proxy_Function> >(bv));
      }

  /**
   * Helper for calling script code as if it were native C++ code
   * example:
   * boost::function<int (int, int)> f = build_functor(chai, "func(x, y){x+y}");
   * \return a boost::function representing the passed in script
   * \param[in] e ScriptEngine to build the script execution from
   * \param[in] script Script code to build a function from
   */
  template<typename FunctionType, typename ScriptEngine>
    boost::function<FunctionType> build_functor(ScriptEngine &e, const std::string &script)
    {
      return build_function_caller<FunctionType>(e.evaluate_string(script));
    }
}

# endif
#else
# define n BOOST_PP_ITERATION()

namespace dispatchkit
{
  /**
   * used internally for unwrapping a function call's types
   */
  template<typename Ret BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM_PARAMS(n, typename Param) >
    Ret function_caller(const std::vector<std::pair<std::string, boost::shared_ptr<Proxy_Function> > > &funcs 
        BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM_BINARY_PARAMS(n, Param, p) )
    {
      std::vector<Boxed_Value> params;

      BOOST_PP_REPEAT(n, addparam, ~)

      return Function_Caller_Ret<Ret>().call(funcs, params);
    }

  /**
   * used internally for unwrapping a function call's types
   */
  template<typename Ret BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM_PARAMS(n, typename Param) >
    boost::function<Ret (BOOST_PP_ENUM_PARAMS(n, Param)) > 
      build_function_caller_helper(Ret (BOOST_PP_ENUM_PARAMS(n, Param)), const std::vector<std::pair<std::string, boost::shared_ptr<Proxy_Function> > > &funcs)
      {
        return boost::bind(&function_caller<Ret BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM_PARAMS(n, Param)>, funcs
            BOOST_PP_ENUM_TRAILING(n, curry, ~));
      }


}

#endif

