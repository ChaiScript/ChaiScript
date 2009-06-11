#include <boost/preprocessor.hpp>

#ifndef  BOOST_PP_IS_ITERATING
#ifndef __register_function_hpp__
#define __register_function_hpp__

#include <boxedcpp.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>


#define BOOST_PP_ITERATION_LIMITS ( 0, 10 )
#define BOOST_PP_FILENAME_1 "register_function.hpp"

#include BOOST_PP_ITERATE()

# endif
#else
# define n BOOST_PP_ITERATION()

template<typename Ret BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM_PARAMS(n, typename Param)>
void register_function(Dispatch_Engine &s, Ret (*f)(BOOST_PP_ENUM_PARAMS(n, Param)), const std::string &name)
{
  s.register_function(boost::function<Ret (BOOST_PP_ENUM_PARAMS(n, Param))>(f), name);
}

template<typename Ret, typename Class BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM_PARAMS(n, typename Param)>
void register_function(Dispatch_Engine &s, Ret (Class::*f)(BOOST_PP_ENUM_PARAMS(n, Param)), const std::string &name)
{
  s.register_function(boost::function<Ret (Class* BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM_PARAMS(n, Param))>(f), name);
}


#endif

