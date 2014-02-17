// This file is distributed under the BSD License.
// See "license.txt" for details.
// Copyright 2009-2012, Jonathan Turner (jonathan@emptycrate.com)
// Copyright 2009-2014, Jason Turner (jason@emptycrate.com)
// http://www.chaiscript.com

#ifndef CHAISCRIPT_BIND_FIRST_HPP_
#define CHAISCRIPT_BIND_FIRST_HPP_

#include <functional>

namespace chaiscript
{
  namespace detail
  {

    template<int>
      struct Placeholder
      {
      };

    template<>
      struct Placeholder<1>
      {
        static decltype(std::placeholders::_1) value() { return std::placeholders::_1; }
      };

    template<>
      struct Placeholder<2>
      {
        static decltype(std::placeholders::_2) value() { return std::placeholders::_2; }
      };

    template<>
      struct Placeholder<3>
      {
        static decltype(std::placeholders::_3) value() { return std::placeholders::_3; }
      };

    template<>
      struct Placeholder<4>
      {
        static decltype(std::placeholders::_4) value() { return std::placeholders::_4; }
      };

    template<>
      struct Placeholder<5>
      {
        static decltype(std::placeholders::_5) value() { return std::placeholders::_5; }
      };

    template<>
      struct Placeholder<6>
      {
        static decltype(std::placeholders::_6) value() { return std::placeholders::_6; }
      };

    template<>
      struct Placeholder<7>
      {
        static decltype(std::placeholders::_7) value() { return std::placeholders::_7; }
      };

    template<>
      struct Placeholder<8>
      {
        static decltype(std::placeholders::_8) value() { return std::placeholders::_8; }
      };

    template<>
      struct Placeholder<9>
      {
        static decltype(std::placeholders::_9) value() { return std::placeholders::_9; }
      };

    template<>
      struct Placeholder<10>
      {
        static decltype(std::placeholders::_10) value() { return std::placeholders::_10; }
      };


    template<int count, int maxcount, typename Sig>
      struct Bind_First
      {
        template<typename F, typename ... InnerParams>
          static std::function<Sig> bind(F f, InnerParams ... innerparams)
          {
            return Bind_First<count - 1, maxcount, Sig>::bind(f, innerparams..., Placeholder<maxcount - count + 1>::value());
          } 
      };

    template<int maxcount, typename Sig>
      struct Bind_First<0, maxcount, Sig>
      {
        template<typename F, typename ... InnerParams>
          static std::function<Sig> bind(F f, InnerParams ... innerparams)
          {
            return std::bind(f, innerparams...);
          }
      };


    template<typename O, typename Ret, typename P1, typename ... Param>
      std::function<Ret (Param...)> bind_first(Ret (*f)(P1, Param...), O o)
      {
        return Bind_First<sizeof...(Param), sizeof...(Param), Ret (Param...)>::bind(f, o);
      }

    template<typename O, typename Ret, typename Class, typename ... Param>
      std::function<Ret (Param...)> bind_first(Ret (Class::*f)(Param...), O o)
      {
        return Bind_First<sizeof...(Param), sizeof...(Param), Ret (Param...)>::bind(f, o);
      }

    template<typename O, typename Ret, typename Class, typename ... Param>
      std::function<Ret (Param...)> bind_first(Ret (Class::*f)(Param...) const, O o)
      {
        return Bind_First<sizeof...(Param), sizeof...(Param), Ret (Param...)>::bind(f, o);
      }

    template<typename O, typename Ret, typename P1, typename ... Param>
      std::function<Ret (Param...)> bind_first(const std::function<Ret (P1, Param...)> &f, O o)
      {
        return Bind_First<sizeof...(Param), sizeof...(Param), Ret (Param...)>::bind(f, o);
      }


  }
}


#endif
