// This file is distributed under the BSD License.
// See "license.txt" for details.
// Copyright 2009-2012, Jonathan Turner (jonathan@emptycrate.com)
// Copyright 2009-2016, Jason Turner (jason@emptycrate.com)
// http://www.chaiscript.com

#ifndef CHAISCRIPT_HANDLE_RETURN_HPP_
#define CHAISCRIPT_HANDLE_RETURN_HPP_

#include <functional>
#include <memory>
#include <type_traits>

#include "boxed_number.hpp"
#include "boxed_value.hpp"

namespace chaiscript {
class Boxed_Number;
}  // namespace chaiscript

namespace chaiscript
{
  namespace dispatch
  {
    template<class T, class U> class Proxy_Function_Callable_Impl;
    template<class T> class Assignable_Proxy_Function_Impl;

    namespace detail
    {
      /**
       * Used internally for handling a return value from a Proxy_Function call
       */
      template<typename Ret>
        struct Handle_Return
        {
          template<typename T,
                   typename = typename std::enable_if<std::is_pod<typename std::decay<T>::type>::value>::type>
          static Boxed_Value handle(T r)
          {
            return Boxed_Value(std::move(r), true);
          }

          template<typename T,
                   typename = typename std::enable_if<!std::is_pod<typename std::decay<T>::type>::value>::type>
          static Boxed_Value handle(T &&r)
          {
            return Boxed_Value(std::make_shared<T>(std::forward<T>(r)), true);
          }
        };

      template<typename Ret>
        struct Handle_Return<const std::function<Ret> &>
        {
          static Boxed_Value handle(const std::function<Ret> &f) {
            return Boxed_Value(
                chaiscript::make_shared<dispatch::Proxy_Function_Base, dispatch::Proxy_Function_Callable_Impl<Ret, std::function<Ret>>>(f)
              );
          }
        };

      template<typename Ret>
        struct Handle_Return<std::function<Ret>>
        {
          static Boxed_Value handle(const std::function<Ret> &f) {
            return Boxed_Value(
                chaiscript::make_shared<dispatch::Proxy_Function_Base, dispatch::Proxy_Function_Callable_Impl<Ret, std::function<Ret>>>(f)
              );
          }
        };

      template<typename Ret>
        struct Handle_Return<const std::shared_ptr<std::function<Ret>>>
        {
          static Boxed_Value handle(const std::shared_ptr<std::function<Ret>> &f) {
            return Boxed_Value(
                chaiscript::make_shared<dispatch::Proxy_Function_Base, dispatch::Assignable_Proxy_Function_Impl<Ret>>(std::ref(*f),f)
                );
          }
        };

      template<typename Ret>
        struct Handle_Return<const std::shared_ptr<std::function<Ret>> &>
        {
          static Boxed_Value handle(const std::shared_ptr<std::function<Ret>> &f) {
            return Boxed_Value(
                chaiscript::make_shared<dispatch::Proxy_Function_Base, dispatch::Assignable_Proxy_Function_Impl<Ret>>(std::ref(*f),f)
              );
          }
        };

      template<typename Ret>
        struct Handle_Return<std::shared_ptr<std::function<Ret>>>
        {
          static Boxed_Value handle(const std::shared_ptr<std::function<Ret>> &f) {
            return Boxed_Value(
                chaiscript::make_shared<dispatch::Proxy_Function_Base, dispatch::Assignable_Proxy_Function_Impl<Ret>>(std::ref(*f),f)
              );
          }
        };

      template<typename Ret>
        struct Handle_Return<std::function<Ret> &>
        {
          static Boxed_Value handle(std::function<Ret> &f) {
            return Boxed_Value(
                chaiscript::make_shared<dispatch::Proxy_Function_Base, dispatch::Assignable_Proxy_Function_Impl<Ret>>(std::ref(f),
                  std::shared_ptr<std::function<Ret>>())
              );
          }

          static Boxed_Value handle(const std::function<Ret> &f) {
            return Boxed_Value(
                chaiscript::make_shared<dispatch::Proxy_Function_Base, dispatch::Proxy_Function_Callable_Impl<Ret, std::function<Ret>>>(f)
              );
          }
        };

      template<typename Ret>
        struct Handle_Return<Ret *>
        {
          static Boxed_Value handle(Ret *p)
          {
            return Boxed_Value(p, true);
          }
        };

      template<typename Ret>
        struct Handle_Return<const Ret *>
        {
          static Boxed_Value handle(const Ret *p)
          {
            return Boxed_Value(p, true);
          }
        };

      template<typename Ret>
        struct Handle_Return<std::shared_ptr<Ret> &>
        {
          static Boxed_Value handle(const std::shared_ptr<Ret> &r)
          {
            return Boxed_Value(r, true);
          }
        };

      template<typename Ret>
        struct Handle_Return<std::shared_ptr<Ret> >
        {
          static Boxed_Value handle(const std::shared_ptr<Ret> &r)
          {
            return Boxed_Value(r, true);
          }
        };

      template<typename Ret>
        struct Handle_Return<const std::shared_ptr<Ret> &>
        {
          static Boxed_Value handle(const std::shared_ptr<Ret> &r)
          {
            return Boxed_Value(r, true);
          }
        };

      template<typename Ret>
        struct Handle_Return<const Ret &>
        {
          static Boxed_Value handle(const Ret &r)
          {
            return Boxed_Value(std::cref(r), true);
          }
        };


      /**
       * Used internally for handling a return value from a Proxy_Function call
       */
      template<typename Ret>
        struct Handle_Return<Ret &>
        {
          static Boxed_Value handle(Ret &r)
          {
            return Boxed_Value(std::ref(r));
          }

          static Boxed_Value handle(const Ret &r)
          {
            return Boxed_Value(std::cref(r));
          }
        };

      /**
       * Used internally for handling a return value from a Proxy_Function call
       */
      template<>
        struct Handle_Return<Boxed_Value>
        {
          static Boxed_Value handle(const Boxed_Value &r)
          {
            return r;
          }
        };

      /**
       * Used internally for handling a return value from a Proxy_Function call
       */
      template<>
        struct Handle_Return<const Boxed_Value>
        {
          static Boxed_Value handle(const Boxed_Value &r)
          {
            return r;
          }
        };

      /**
       * Used internally for handling a return value from a Proxy_Function call
       */
      template<>
        struct Handle_Return<Boxed_Value &>
        {
          static Boxed_Value handle(const Boxed_Value &r)
          {
            return r;
          }
        };

      /**
       * Used internally for handling a return value from a Proxy_Function call
       */
      template<>
        struct Handle_Return<const Boxed_Value &>
        {
          static Boxed_Value handle(const Boxed_Value &r)
          {
            return r;
          }
        };

      /**
       * Used internally for handling a return value from a Proxy_Function call
       */
      template<>
        struct Handle_Return<Boxed_Number>
        {
          static Boxed_Value handle(const Boxed_Number &r)
          {
            return r.bv;
          }
        };

      /**
       * Used internally for handling a return value from a Proxy_Function call
       */
      template<>
        struct Handle_Return<const Boxed_Number>
        {
          static Boxed_Value handle(const Boxed_Number &r)
          {
            return r.bv;
          }
        };


      /**
       * Used internally for handling a return value from a Proxy_Function call
       */
      template<>
        struct Handle_Return<void>
        {
          static Boxed_Value handle()
          {
            return Boxed_Value(Boxed_Value::Void_Type());
          }
        };
    }
  }
}

#endif
