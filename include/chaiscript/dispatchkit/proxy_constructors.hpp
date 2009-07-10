#include <boost/preprocessor.hpp>

#ifndef  BOOST_PP_IS_ITERATING
#ifndef __proxy_constructors_hpp__
#define __proxy_constructors_hpp__

#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>


#define BOOST_PP_ITERATION_LIMITS ( 0, 10 )
//#define BOOST_PP_FILENAME_1 "proxy_constructors.hpp"
#define BOOST_PP_FILENAME_1 <chaiscript/dispatchkit/proxy_constructors.hpp>
#include BOOST_PP_ITERATE()
# endif
#else
# define n BOOST_PP_ITERATION()

namespace dispatchkit
{
  template<typename Class BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM_PARAMS(n, typename Param) >
    boost::shared_ptr<Class> constructor( BOOST_PP_ENUM_BINARY_PARAMS(n, Param, p) )
    {
      return boost::shared_ptr<Class>(new Class( BOOST_PP_ENUM_PARAMS(n, p) ));
    }

  template<typename Class BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM_PARAMS(n, typename Param) >
    boost::function<boost::shared_ptr<Class> (BOOST_PP_ENUM_PARAMS(n, Param))> build_constructor()
    {
      typedef boost::shared_ptr<Class> (*func)(BOOST_PP_ENUM_PARAMS(n, Param));
      return boost::function<boost::shared_ptr<Class> (BOOST_PP_ENUM_PARAMS(n, Param))>(func(&(constructor<Class BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM_PARAMS(n, Param)>)));
    }
}

#endif

