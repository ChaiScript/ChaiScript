// This file is distributed under the BSD License.
// See "license.txt" for details.
// Copyright 2009, Jonathan Turner (jturner@minnow-lang.org)
// and Jason Turner (lefticus@gmail.com)
// http://www.chaiscript.com

#include <boost/preprocessor.hpp>
#include <boost/preprocessor/arithmetic/inc.hpp>

#define param(z,n,text) BOOST_PP_CAT(text, BOOST_PP_INC(n)) 

#ifndef  BOOST_PP_IS_ITERATING
#ifndef __bind_first_hpp__
#define __bind_first_hpp__

#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/ref.hpp>
#include <boost/function_types/result_type.hpp>
#include <boost/function_types/function_type.hpp>
#include <boost/function_types/parameter_types.hpp>
#include <boost/function_types/function_arity.hpp>

#define BOOST_PP_ITERATION_LIMITS ( 0, 8 )
#define BOOST_PP_FILENAME_1 <chaiscript/dispatchkit/bind_first.hpp>

namespace chaiscript
{
  namespace detail
  {
    template<int, typename T>
      struct bind_helper 
      {
        template<typename F, typename V>
          static boost::function<T> go(const F &f, const V &v)
          {
            return boost::function<T>();
          }
      };

#include BOOST_PP_ITERATE()

    template<typename T>
      struct pop_front
      {
        typedef typename boost::function_types::function_type<
          typename boost::mpl::push_front<
          typename boost::mpl::pop_front<typename
          boost::function_types::parameter_types<T> >::type,
          typename boost::function_types::result_type<T>::type
            >::type
            >::type type;
      };

    template<typename T, typename U>
      boost::function<typename pop_front<T>::type> bind_first(T t, const U &u)
      {
        return bind_helper<boost::function_types::function_arity<T>::value,
               typename pop_front<T>::type>::go(t, u);
      }
  }
}

# endif
#else
# define n BOOST_PP_ITERATION()
# define m BOOST_PP_INC(n)


    template<typename T>
      struct bind_helper<m, T>
      {
        template<typename F, typename V>
          static boost::function<T> go(const F &f, const V &v)
          {
            return boost::bind(f, v BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM(n, param, _));
          }
      };


#endif
