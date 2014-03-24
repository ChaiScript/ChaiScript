// This file is distributed under the BSD License.
// See "license.txt" for details.
// Copyright 2009-2012, Jonathan Turner (jonathan@emptycrate.com)
// Copyright 2009-2014, Jason Turner (jason@emptycrate.com)
// http://www.chaiscript.com


#ifndef CHAISCRIPT_PROXY_CONSTRUCTORS_HPP_
#define CHAISCRIPT_PROXY_CONSTRUCTORS_HPP_

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
      template<typename Class, typename ... Params>
        std::shared_ptr<Class> constructor_(Params ... params)
        {
          return std::shared_ptr<Class>(new Class(params...));
        }

      template<typename Class, typename ... Params  >
        Proxy_Function build_constructor_(Class (*)(Params...))
        {
          typedef std::shared_ptr<Class> (sig)(Params...);
          return Proxy_Function(new Proxy_Function_Impl<sig>(std::function<sig>(&(constructor_<Class, Params...>))));
        }
    }
  }


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

#endif

