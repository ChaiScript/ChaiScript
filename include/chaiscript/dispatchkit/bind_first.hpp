// This file is distributed under the BSD License.
// See "license.txt" for details.
// Copyright 2009-2012, Jonathan Turner (jonathan@emptycrate.com)
// Copyright 2009-2016, Jason Turner (jason@emptycrate.com)
// http://www.chaiscript.com

#ifndef CHAISCRIPT_BIND_FIRST_HPP_
#define CHAISCRIPT_BIND_FIRST_HPP_

#include <functional>

namespace chaiscript
{
  namespace detail
  {

    template<typename T>
      T* get_pointer(T *t)
      {
        return t;
      }

    template<typename T>
      T* get_pointer(const std::reference_wrapper<T> &t)
      {
        return &t.get();
      }

    template<typename O, typename Ret, typename P1, typename ... Param>
      std::function<Ret (Param...)> bind_first(Ret (*f)(P1, Param...), O&& o)
      {
        return std::function<Ret (Param...)>(
            [f, o](Param...param) -> Ret {
              return f(std::forward<O>(o), std::forward<Param>(param)...);
            }
          );
      }

    template<typename O, typename Ret, typename Class, typename ... Param>
      std::function<Ret (Param...)> bind_first(Ret (Class::*f)(Param...), O&& o)
      {
        return std::function<Ret (Param...)>(
            [f, o](Param...param) -> Ret {
              return (get_pointer(o)->*f)(std::forward<Param>(param)...);
            }
          );
      }

    template<typename O, typename Ret, typename Class, typename ... Param>
      std::function<Ret (Param...)> bind_first(Ret (Class::*f)(Param...) const, O&& o)
      {
        return std::function<Ret (Param...)>(
            [f, o](Param...param) -> Ret {
              return (get_pointer(o)->*f)(std::forward<Param>(param)...);
            }
          );

      }

    template<typename O, typename Ret, typename P1, typename ... Param>
      std::function<Ret (Param...)> bind_first(const std::function<Ret (P1, Param...)> &f, O&& o)
      {
        return std::function<Ret (Param...)>(
            [f, o](Param...param) -> Ret {
              return f(o, std::forward<Param>(param)...);
            });
      }


  }
}


#endif
