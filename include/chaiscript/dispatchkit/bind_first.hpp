// This file is distributed under the BSD License.
// See "license.txt" for details.
// Copyright 2009-2012, Jonathan Turner (jonathan@emptycrate.com)
// Copyright 2009-2015, Jason Turner (jason@emptycrate.com)
// http://www.chaiscript.com

#ifndef CHAISCRIPT_BIND_FIRST_HPP_
#define CHAISCRIPT_BIND_FIRST_HPP_

#include <functional>

namespace chaiscript
{
  namespace detail
  {

    struct Placeholder
    {
      static std::tuple<decltype(std::placeholders::_1),decltype(std::placeholders::_2),decltype(std::placeholders::_3),decltype(std::placeholders::_4),decltype(std::placeholders::_5),decltype(std::placeholders::_6),decltype(std::placeholders::_7),decltype(std::placeholders::_8),decltype(std::placeholders::_9),decltype(std::placeholders::_10)> placeholder() {
        return std::tuple<decltype(std::placeholders::_1),decltype(std::placeholders::_2),decltype(std::placeholders::_3),decltype(std::placeholders::_4),decltype(std::placeholders::_5),decltype(std::placeholders::_6),decltype(std::placeholders::_7),decltype(std::placeholders::_8),decltype(std::placeholders::_9),decltype(std::placeholders::_10)>(std::placeholders::_1,std::placeholders::_2,std::placeholders::_3,std::placeholders::_4,std::placeholders::_5,std::placeholders::_6,std::placeholders::_7,std::placeholders::_8,std::placeholders::_9,std::placeholders::_10);
      }
    };

    template<int count, int maxcount, typename Sig>
      struct Bind_First
      {
        template<typename F, typename ... InnerParams>
          static std::function<Sig> bind(F&& f, InnerParams ... innerparams)
          {
            return Bind_First<count - 1, maxcount, Sig>::bind(std::forward<F>(f), innerparams..., std::get<maxcount - count>(Placeholder::placeholder()));
          } 
      };

    template<int maxcount, typename Sig>
      struct Bind_First<0, maxcount, Sig>
      {
        template<typename F, typename ... InnerParams>
          static std::function<Sig> bind(F&& f, InnerParams ... innerparams)
          {
    return std::bind(std::forward<F>(f), innerparams...);
          }
      };


    template<typename O, typename Ret, typename P1, typename ... Param>
      std::function<Ret (Param...)> bind_first(Ret (*f)(P1, Param...), O&& o)
      {
        return Bind_First<sizeof...(Param), sizeof...(Param), Ret (Param...)>::bind(f, std::forward<O>(o));
      }

    template<typename O, typename Ret, typename Class, typename ... Param>
      std::function<Ret (Param...)> bind_first(Ret (Class::*f)(Param...), O&& o)
      {
        return Bind_First<sizeof...(Param), sizeof...(Param), Ret (Param...)>::bind(f, std::forward<O>(o));
      }

    template<typename O, typename Ret, typename Class, typename ... Param>
      std::function<Ret (Param...)> bind_first(Ret (Class::*f)(Param...) const, O&& o)
      {
        return Bind_First<sizeof...(Param), sizeof...(Param), Ret (Param...)>::bind(f, std::forward<O>(o));
      }

    template<typename O, typename Ret, typename P1, typename ... Param>
      std::function<Ret (Param...)> bind_first(const std::function<Ret (P1, Param...)> &f, O&& o)
      {
        return Bind_First<sizeof...(Param), sizeof...(Param), Ret (Param...)>::bind(f, std::forward<O>(o));
      }

    template<typename O, typename Ret, typename P1, typename ... Param>
      std::function<Ret (Param...)> bind_first(std::function<Ret (P1, Param...)> &&f, O&& o)
      {
        return Bind_First<sizeof...(Param), sizeof...(Param), Ret (Param...)>::bind(std::move(f), std::forward<O>(o));
      }

  }
}


#endif
