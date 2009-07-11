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
#define BOOST_PP_FILENAME_1 "function_call.hpp"
#include BOOST_PP_ITERATE()

namespace dispatchkit
{ 
  template<typename FunctionType>
    boost::function<FunctionType>
      build_function_caller(const std::vector<std::pair<std::string, boost::shared_ptr<Proxy_Function> > > &funcs)
      {
        FunctionType *p;
        return build_function_caller_helper(p, funcs);
      }

  template<typename FunctionType>
    boost::function<FunctionType>
      build_function_caller(boost::shared_ptr<Proxy_Function> func)
      {
        std::vector<std::pair<std::string, boost::shared_ptr<Proxy_Function> > > funcs;
        funcs.push_back(std::make_pair(std::string(), func));
        return build_function_caller<FunctionType>(funcs);
      }

  template<typename FunctionType>
    boost::function<FunctionType>
      build_function_caller(const Boxed_Value &bv)
      {
        return build_function_caller<FunctionType>(boxed_cast<boost::shared_ptr<Proxy_Function> >(bv));
      }

     
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
  template<typename Ret BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM_PARAMS(n, typename Param) >
    Ret function_caller(const std::vector<std::pair<std::string, boost::shared_ptr<Proxy_Function> > > &funcs 
        BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM_BINARY_PARAMS(n, Param, p) )
    {
      std::vector<Boxed_Value> params;

      BOOST_PP_REPEAT(n, addparam, ~)

      return Function_Caller_Ret<Ret>().call(funcs, params);
    }

  template<typename Ret BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM_PARAMS(n, typename Param) >
    boost::function<Ret (BOOST_PP_ENUM_PARAMS(n, Param)) > 
      build_function_caller_helper(Ret (BOOST_PP_ENUM_PARAMS(n, Param)), const std::vector<std::pair<std::string, boost::shared_ptr<Proxy_Function> > > &funcs)
      {
        return boost::bind(&function_caller<Ret BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM_PARAMS(n, Param)>, funcs
            BOOST_PP_ENUM_TRAILING(n, curry, ~));
      }


}

#endif

