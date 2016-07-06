// This file is distributed under the BSD License.
// See "license.txt" for details.
// Copyright 2009-2012, Jonathan Turner (jonathan@emptycrate.com)
// Copyright 2009-2016, Jason Turner (jason@emptycrate.com)
// http://www.chaiscript.com

#ifndef CHAISCRIPT_CALLABLE_TRAITS_HPP_
#define CHAISCRIPT_CALLABLE_TRAITS_HPP_

#include <memory>

namespace chaiscript {
  namespace dispatch {
    namespace detail {


      template<typename T>
        struct Arity
        {
        };

      template<typename Ret, typename ... Params>
      struct Arity<Ret (Params...)>
        {
          static const size_t arity = sizeof...(Params);
        };


      template<typename T>
        struct Function_Signature
        {
        };

      template<typename Ret, typename ... Params>
      struct Function_Signature<Ret (Params...)>
        {
          typedef Ret Return_Type;
          typedef Ret (Signature)(Params...);
        };

      template<typename Ret, typename T, typename ... Params>
      struct Function_Signature<Ret (T::*)(Params...) const>
        {
          typedef Ret Return_Type;
          typedef Ret (Signature)(Params...);
        };


      template<typename T>
        struct Callable_Traits
        {
          typedef typename Function_Signature<decltype(&T::operator())>::Signature Signature;
          typedef typename Function_Signature<decltype(&T::operator())>::Return_Type Return_Type;
        };
    }
  }
}

#endif

