// This file is distributed under the BSD License.
// See "license.txt" for details.
// Copyright 2009-2012, Jonathan Turner (jonathan@emptycrate.com)
// Copyright 2009-2016, Jason Turner (jason@emptycrate.com)
// http://www.chaiscript.com


#ifndef CHAISCRIPT_PROXY_CONSTRUCTORS_HPP_
#define CHAISCRIPT_PROXY_CONSTRUCTORS_HPP_

#include "proxy_functions.hpp"

namespace chaiscript
{
  namespace dispatch
  {
    namespace detail
    {

      template<typename Class, typename ... Params, size_t ... I >
        Proxy_Function build_constructor_(Class (*)(Params...), std::index_sequence<I...>)
        {
          return [](){
            class Func final : public dispatch::Proxy_Function_Impl_Base
            {
              public:
                Func()
                  : dispatch::Proxy_Function_Impl_Base({user_type<std::shared_ptr<Class>>(), user_type<Params>()...})
                  {
                  }

                bool compare_types_with_cast(const std::vector<Boxed_Value> &params, const Type_Conversions_State &t_conversions) const override
                {
                  return compare_types_with_cast_impl<Params...>(params, t_conversions);
                }

              protected:
                Boxed_Value do_call(const std::vector<Boxed_Value> &params, const Type_Conversions_State &t_conversions) const override
                {
                  return Handle_Return<std::shared_ptr<Class>>::handle(std::make_shared<Class>(boxed_cast<Params>(params[I], &t_conversions)...));
                }
            };

            return chaiscript::make_shared<dispatch::Proxy_Function_Base, Func>();
          }();
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
      T *f = nullptr;
      return dispatch::detail::build_constructor_(f, std::make_index_sequence<dispatch::detail::Arity<T>::arity>());
    }

}

#endif

