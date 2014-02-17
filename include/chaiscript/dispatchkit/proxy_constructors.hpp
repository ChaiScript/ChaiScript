// This file is distributed under the BSD License.
// See "license.txt" for details.
// Copyright 2009-2012, Jonathan Turner (jonathan@emptycrate.com)
// Copyright 2009-2014, Jason Turner (jason@emptycrate.com)
// http://www.chaiscript.com

#include <boost/preprocessor.hpp>

#ifndef  BOOST_PP_IS_ITERATING
#ifndef CHAISCRIPT_PROXY_CONSTRUCTORS_HPP_
#define CHAISCRIPT_PROXY_CONSTRUCTORS_HPP_

#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>


#define BOOST_PP_ITERATION_LIMITS ( 0, 10 )
#define BOOST_PP_FILENAME_1 <chaiscript/dispatchkit/proxy_constructors.hpp>
#include BOOST_PP_ITERATE()
# endif

namespace chaiscript
{
  /// \brief Generates a constructor function for use with ChaiScript
  /// 
  /// \tparam T The signature of the constructor to generate. In the form of: ClassType (ParamType1, ParamType2, ...)
  /// 
  /// Example:
  /// \code
  ///    chaiscript::ChaiScript chai;
  ///    // Create a new function that creates a MyClass object using the (int, float) constructor
  ///    // and call that function "MyClass" so that it appears as a normal constructor to the user.
  ///    chai.add(constructor<MyClass (int, float)>(), "MyClass");
  /// \endcode
  template<typename T>
    Proxy_Function constructor()
    {
      T *f = 0;
      return (dispatch::detail::build_constructor_(f));
    }
}

#else
# define n BOOST_PP_ITERATION()

namespace chaiscript
{
  namespace dispatch
  {
    namespace detail
    {
      /**
       * A constructor function, used for creating a new object
       * of a given type with a given set of params
       */
      template<typename Class BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM_PARAMS(n, typename Param) >
        boost::shared_ptr<Class> constructor_( BOOST_PP_ENUM_BINARY_PARAMS(n, Param, p) )
        {
          return boost::shared_ptr<Class>(new Class( BOOST_PP_ENUM_PARAMS(n, p) ));
        }

      /**
       * Helper function for build a constructor function
       * example:
       * dispatchengine.register_function(build_constructor<MyClass, int, const std::string&>, "MyClass");
       * \todo See if it is possible to make this not be a variadic function
       */
      template<typename Class BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM_PARAMS(n, typename Param) >
        Proxy_Function build_constructor_(Class (*)(BOOST_PP_ENUM_PARAMS(n, Param)))
        {
          typedef boost::shared_ptr<Class> (sig)(BOOST_PP_ENUM_PARAMS(n, Param));
          return Proxy_Function(new Proxy_Function_Impl<sig>(boost::function<sig>(&(constructor_<Class BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM_PARAMS(n, Param)>))));
        }
    }
  }
}
#undef n

#endif

