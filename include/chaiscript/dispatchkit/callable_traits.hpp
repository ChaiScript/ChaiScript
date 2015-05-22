// This file is distributed under the BSD License.
// See "license.txt" for details.
// Copyright 2009-2012, Jonathan Turner (jonathan@emptycrate.com)
// Copyright 2009-2015, Jason Turner (jason@emptycrate.com)
// http://www.chaiscript.com

#ifndef CHAISCRIPT_CALLABLE_TRAITS_HPP_
#define CHAISCRIPT_CALLABLE_TRAITS_HPP_

namespace chaiscript {
  namespace dispatch {
    namespace detail {

      template<typename T>
        struct Function_Signature
        {
          typedef T Signature;
        };

      template<typename T>
        struct Callable_Traits
        {

          template<typename Ret, typename ... Param>
            static Ret deduce_ret_type(Ret (T::*)(Param...) const);

          template<typename Ret, typename ... Param>
            static Function_Signature<Ret (Param...)> deduce_sig_type(Ret (T::*)(Param...) const);

          typedef decltype(deduce_ret_type(&T::operator())) Return_Type;
          typedef typename decltype(deduce_sig_type(&T::operator()))::Signature Signature;
          typedef decltype(deduce_sig_type(&T::operator())) Signature_Object;
        };
    }
  }
}

#endif

