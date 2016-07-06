// This file is distributed under the BSD License.
// See "license.txt" for details.
// Copyright 2009-2012, Jonathan Turner (jonathan@emptycrate.com)
// Copyright 2009-2016, Jason Turner (jason@emptycrate.com)
// http://www.chaiscript.com

#ifndef CHAISCRIPT_REGISTER_FUNCTION_HPP_
#define CHAISCRIPT_REGISTER_FUNCTION_HPP_

#include <type_traits>

#include "bind_first.hpp"
#include "proxy_functions.hpp"
#include "handle_return.hpp"
#include "function_call.hpp"

namespace chaiscript
{

  /// \brief Creates a new Proxy_Function object from a free function, member function or data member
  /// \param[in] t Function / member to expose
  ///
  /// \b Example:
  /// \code
  /// int myfunction(const std::string &);
  /// class MyClass
  /// {
  ///   public:
  ///     void memberfunction();
  ///     int memberdata;
  /// };
  /// 
  /// chaiscript::ChaiScript chai;
  /// chai.add(fun(&myfunction), "myfunction");
  /// chai.add(fun(&MyClass::memberfunction), "memberfunction");
  /// chai.add(fun(&MyClass::memberdata), "memberdata");
  /// \endcode
  /// 
  /// \sa \ref adding_functions
  //
  //
  template<typename ... Param, size_t ... I>
    Proxy_Function assignable_fun(
        std::reference_wrapper<std::function<void (Param...)>> t_func,
        std::shared_ptr<std::function<void (Param...)>> t_ptr,
        std::index_sequence<I...>)
    {
      return [t_func, t_ptr](){
        class Func final : public dispatch::Assignable_Proxy_Function
        {
          public:
            Func(std::reference_wrapper<std::function<void (Param...)>> t_f, std::shared_ptr<std::function<void (Param...)>> t_p)
              : Assignable_Proxy_Function({user_type<void>(), user_type<Param>()...}),
                m_f(std::move(t_f)), m_shared_ptr_holder(std::move(t_p))
              {
                assert(!m_shared_ptr_holder || m_shared_ptr_holder.get() == &m_f.get());
              }

            bool compare_types_with_cast(const std::vector<Boxed_Value> &params, const Type_Conversions_State &t_conversions) const override
            {
              return compare_types_with_cast_impl<Param...>(params, t_conversions);
            }

            void assign(const std::shared_ptr<const Proxy_Function_Base> &t_rhs) override {
              m_f.get() = dispatch::functor<void (Param...)>(t_rhs, nullptr);
            }

          protected:
            Boxed_Value do_call(const std::vector<Boxed_Value> &params, const Type_Conversions_State &t_conversions) const override
            {
              m_f(boxed_cast<Param>(params.at(I), &t_conversions)...);
              return dispatch::detail::Handle_Return<void>::handle();
            }

          private:
            std::reference_wrapper<std::function<void (Param...)>> m_f;
            std::shared_ptr<std::function<void (Param...)>> m_shared_ptr_holder;
        };

        return chaiscript::make_shared<dispatch::Proxy_Function_Base, Func>(t_func, t_ptr);
      }();
    }

  template<typename Ret, typename ... Param, size_t ... I>
    Proxy_Function assignable_fun(
        std::reference_wrapper<std::function<Ret (Param...)>> t_func,
        std::shared_ptr<std::function<Ret (Param...)>> t_ptr,
        std::index_sequence<I...>)
    {
      return [t_func, t_ptr](){
        class Func final : public dispatch::Assignable_Proxy_Function
        {
          public:
            Func(std::reference_wrapper<std::function<Ret (Param...)>> t_f, std::shared_ptr<std::function<Ret (Param...)>> t_p)
              : Assignable_Proxy_Function({user_type<Ret>(), user_type<Param>()...}),
                m_f(std::move(t_f)), m_shared_ptr_holder(std::move(t_p))
              {
                assert(!m_shared_ptr_holder || m_shared_ptr_holder.get() == &m_f.get());
              }

            bool compare_types_with_cast(const std::vector<Boxed_Value> &params, const Type_Conversions_State &t_conversions) const override
            {
              return compare_types_with_cast_impl<Param...>(params, t_conversions);
            }

            void assign(const std::shared_ptr<const Proxy_Function_Base> &t_rhs) override {
              m_f.get() = dispatch::functor<Ret (Param...)>(t_rhs, nullptr);
            }

          protected:
            Boxed_Value do_call(const std::vector<Boxed_Value> &params, const Type_Conversions_State &t_conversions) const override
            {
              return dispatch::detail::Handle_Return<Ret>::handle(m_f(boxed_cast<Param>(params.at(I), &t_conversions)...));
            }

          private:
            std::reference_wrapper<std::function<Ret (Param...)>> m_f;
            std::shared_ptr<std::function<Ret (Param...)>> m_shared_ptr_holder;
        };

        return chaiscript::make_shared<dispatch::Proxy_Function_Base, Func>(t_func, t_ptr);
      }();
    }

  template<typename Ret, typename ... Param>
    Proxy_Function assignable_fun(
        std::reference_wrapper<std::function<Ret (Param...)>> t_func,
        std::shared_ptr<std::function<Ret (Param...)>> t_ptr
        )
    {
      return assignable_fun(std::move(t_func), std::move(t_ptr), std::make_index_sequence<sizeof...(Param)>());
    }


  template<typename T, typename ... Param, size_t ... I>
    Proxy_Function fun(const T &t_func, void (*)(Param...), std::index_sequence<I...>)
    {
      return [t_func](){
        class Func final : public dispatch::Proxy_Function_Impl_Base
        {
          public:
            Func(const T &func)
              : dispatch::Proxy_Function_Impl_Base({user_type<void>(), user_type<Param>()...}),
                m_f(func)
            {
            }

            bool compare_types_with_cast(const std::vector<Boxed_Value> &params, const Type_Conversions_State &t_conversions) const override
            {
              return compare_types_with_cast_impl<Param...>(params, t_conversions);
            }

          protected:
            Boxed_Value do_call(const std::vector<Boxed_Value> &params, const Type_Conversions_State &t_conversions) const override
            {
              m_f(boxed_cast<Param>(params[I], &t_conversions)...);
              return dispatch::detail::Handle_Return<void>::handle();
            }

          private:
            T m_f;
        };

        return chaiscript::make_shared<dispatch::Proxy_Function_Base, Func>(t_func);
      }();
    }

  template<typename Ret, typename T, typename ... Param, size_t ... I>
    Proxy_Function fun(const T &t_func, Ret (*)(Param...), std::index_sequence<I...>)
    {
      return [t_func](){
        class Func final : public dispatch::Proxy_Function_Impl_Base
        {
          public:
            Func(const T &func)
              : dispatch::Proxy_Function_Impl_Base({user_type<Ret>(), user_type<Param>()...}),
                m_f(func)
            {
            }

            bool compare_types_with_cast(const std::vector<Boxed_Value> &params, const Type_Conversions_State &t_conversions) const override
            {
              return compare_types_with_cast_impl<Param...>(params, t_conversions);
            }

          protected:
            Boxed_Value do_call(const std::vector<Boxed_Value> &params, const Type_Conversions_State &t_conversions) const override
            {
              return dispatch::detail::Handle_Return<Ret>::handle(m_f(boxed_cast<Param>(params.at(I), &t_conversions)...));
            }

          private:
            T m_f;
        };

        return chaiscript::make_shared<dispatch::Proxy_Function_Base, Func>(t_func);
      }();
    }

  template<typename T>
    Proxy_Function fun(const T &t)
    {
      typedef typename dispatch::detail::Callable_Traits<T>::Signature Signature;
      Signature *f = nullptr;
      return fun(t, f, std::make_index_sequence<dispatch::detail::Arity<Signature>::arity>());
    }

  template<typename ... Param, size_t ... I>
    Proxy_Function fun(void (*t_func)(Param...), std::index_sequence<I...>)
    {
      return [t_func](){
        class Func final : public dispatch::Proxy_Function_Impl_Base
        {
          public:
            Func(decltype(t_func) func)
              : dispatch::Proxy_Function_Impl_Base({user_type<void>(), user_type<Param>()...}),
                m_f(func)
            {
            }

            bool compare_types_with_cast(const std::vector<Boxed_Value> &params, const Type_Conversions_State &t_conversions) const override
            {
              return compare_types_with_cast_impl<Param...>(params, t_conversions);
            }

          protected:
            Boxed_Value do_call(const std::vector<Boxed_Value> &params, const Type_Conversions_State &t_conversions) const override
            {
              (*m_f)(boxed_cast<Param>(params[I], &t_conversions)...);
              return dispatch::detail::Handle_Return<void>::handle();
            }

          private:
            decltype(t_func) m_f;
        };

        return chaiscript::make_shared<dispatch::Proxy_Function_Base, Func>(t_func);
      }();
    }

  template<typename Ret, typename ... Param, size_t ... I>
    Proxy_Function fun(Ret (*t_func)(Param...), std::index_sequence<I...>)
    {
      return [t_func](){
        class Func final : public dispatch::Proxy_Function_Impl_Base
        {
          public:
            Func(decltype(t_func) func)
              : dispatch::Proxy_Function_Impl_Base({user_type<Ret>(), user_type<Param>()...}),
                m_f(func)
            {
            }

            bool compare_types_with_cast(const std::vector<Boxed_Value> &params, const Type_Conversions_State &t_conversions) const override
            {
              return compare_types_with_cast_impl<Param...>(params, t_conversions);
            }

          protected:
            Boxed_Value do_call(const std::vector<Boxed_Value> &params, const Type_Conversions_State &t_conversions) const override
            {
              return dispatch::detail::Handle_Return<Ret>::handle((*m_f)(boxed_cast<Param>(params[I], &t_conversions)...));
            }

          private:
            decltype(t_func) m_f;
        };

        return chaiscript::make_shared<dispatch::Proxy_Function_Base, Func>(t_func);
      }();
    }



  template<typename Class, typename ... Param, size_t ... I>
    Proxy_Function fun(void (Class::*t_func)(Param...) const, std::index_sequence<I...>)
    {
      return [t_func](){
        class Func final : public dispatch::Proxy_Function_Impl_Base
        {
          public:
            Func(decltype(t_func) func)
              : dispatch::Proxy_Function_Impl_Base({user_type<void>(), user_type<const Class &>(), user_type<Param>()...}),
                m_f(func)
            {
            }

            bool compare_types_with_cast(const std::vector<Boxed_Value> &params, const Type_Conversions_State &t_conversions) const override
            {
              return compare_types_with_cast_impl<const Class &, Param...>(params, t_conversions);
            }

          protected:
            Boxed_Value do_call(const std::vector<Boxed_Value> &params, const Type_Conversions_State &t_conversions) const override
            {
              const Class &o = static_cast<const Class &>(boxed_cast<const Class &>(params[0], &t_conversions));
              (o.*m_f)(boxed_cast<Param>(params[I+1], &t_conversions)...);
              return dispatch::detail::Handle_Return<void>::handle();
            }

          private:
            decltype(t_func) m_f;
        };

        return chaiscript::make_shared<dispatch::Proxy_Function_Base, Func>(t_func);
      }();
    }

  template<typename Ret, typename Class, typename ... Param, size_t ... I>
    Proxy_Function fun(Ret (Class::*t_func)(Param...) const, std::index_sequence<I...>)
    {
      return [t_func](){
        class Func final : public dispatch::Proxy_Function_Impl_Base
        {
          public:
            Func(decltype(t_func) func)
              : dispatch::Proxy_Function_Impl_Base({user_type<Ret>(), user_type<const Class &>(), user_type<Param>()...}),
                m_f(func)
            {
            }

            bool compare_types_with_cast(const std::vector<Boxed_Value> &params, const Type_Conversions_State &t_conversions) const override
            {
              return compare_types_with_cast_impl<const Class &, Param...>(params, t_conversions);
            }

          protected:
            Boxed_Value do_call(const std::vector<Boxed_Value> &params, const Type_Conversions_State &t_conversions) const override
            {
              const Class &o = static_cast<const Class &>(boxed_cast<const Class &>(params[0], &t_conversions));
              return dispatch::detail::Handle_Return<Ret>::handle((o.*m_f)(boxed_cast<Param>(params[I+1], &t_conversions)...));
            }

          private:
            decltype(t_func) m_f;
        };

        return chaiscript::make_shared<dispatch::Proxy_Function_Base, Func>(t_func);
      }();
    }


  template<typename Class, typename ... Param, size_t ... I>
    Proxy_Function fun(void (Class::*t_func)(Param...), std::index_sequence<I...>)
    {
      return [t_func](){
        class Func final : public dispatch::Proxy_Function_Impl_Base
        {
          public:
            Func(decltype(t_func) func)
              : dispatch::Proxy_Function_Impl_Base({user_type<void>(), user_type<Class &>(), user_type<Param>()...}),
                m_f(func)
            {
            }

            bool compare_types_with_cast(const std::vector<Boxed_Value> &params, const Type_Conversions_State &t_conversions) const override
            {
              return compare_types_with_cast_impl<Class &, Param...>(params, t_conversions);
            }

          protected:
            Boxed_Value do_call(const std::vector<Boxed_Value> &params, const Type_Conversions_State &t_conversions) const override
            {
              Class &o = static_cast<Class &>(boxed_cast<Class &>(params[0], &t_conversions));
              (o.*m_f)(boxed_cast<Param>(params[I+1], &t_conversions)...);
              return dispatch::detail::Handle_Return<void>::handle();
            }

          private:
            decltype(t_func) m_f;
        };

        return chaiscript::make_shared<dispatch::Proxy_Function_Base, Func>(t_func);
      }();
    }

  template<typename Ret, typename Class, typename ... Param, size_t ... I>
    Proxy_Function fun(Ret (Class::*t_func)(Param...), std::index_sequence<I...>)
    {
      return [t_func](){
        class Func final : public dispatch::Proxy_Function_Impl_Base
        {
          public:
            Func(decltype(t_func) func)
              : dispatch::Proxy_Function_Impl_Base({user_type<Ret>(), user_type<Class &>(), user_type<Param>()...}),
                m_f(func)
            {
            }

            bool compare_types_with_cast(const std::vector<Boxed_Value> &params, const Type_Conversions_State &t_conversions) const override
            {
              return compare_types_with_cast_impl<Class &, Param...>(params, t_conversions);
            }

          protected:
            Boxed_Value do_call(const std::vector<Boxed_Value> &params, const Type_Conversions_State &t_conversions) const override
            {
              Class &o = static_cast<Class &>(boxed_cast<Class &>(params[0], &t_conversions));
              return dispatch::detail::Handle_Return<Ret>::handle((o.*m_f)(boxed_cast<Param>(params[I+1], &t_conversions)...));
            }

          private:
            decltype(t_func) m_f;
        };

        return chaiscript::make_shared<dispatch::Proxy_Function_Base, Func>(t_func);
      }();

    }

  template<typename Ret, typename Class, typename ... Param>
    Proxy_Function fun(Ret (Class::*t_func)(Param...) const)
    {
      return fun(t_func, std::make_index_sequence<sizeof...(Param)>());
    }

  template<typename Ret, typename ... Param>
    Proxy_Function fun(Ret (*func)(Param...))
    {
      return fun(func, std::make_index_sequence<sizeof...(Param)>());
    }


  template<typename Ret, typename Class, typename ... Param>
    Proxy_Function fun(Ret (Class::*t_func)(Param...))
    {
      return fun(t_func, std::make_index_sequence<sizeof...(Param)>());
    }

  template<typename T, typename Class /*, typename = typename std::enable_if<std::is_member_object_pointer<T>::value>::type*/>
    Proxy_Function fun(T Class::* m /*, typename std::enable_if<std::is_member_object_pointer<T>::value>::type* = 0*/ )
    {
      return Proxy_Function(chaiscript::make_shared<dispatch::Proxy_Function_Base, dispatch::Attribute_Access<T, Class>>(m));
    }



  
  /// \brief Creates a new Proxy_Function object from a free function, member function or data member and binds the first parameter of it
  /// \param[in] t Function / member to expose
  /// \param[in] q Value to bind to first parameter
  ///
  /// \b Example:
  /// \code
  /// struct MyClass
  /// {
  ///   void memberfunction(int);
  /// };
  /// 
  /// MyClass obj;
  /// chaiscript::ChaiScript chai;
  /// // Add function taking only one argument, an int, and permanently bound to "obj"
  /// chai.add(fun(&MyClass::memberfunction, std::ref(obj)), "memberfunction"); 
  /// \endcode
  /// 
  /// \sa \ref adding_functions
  template<typename T, typename Q>
    Proxy_Function fun(T &&t, const Q &q)
    {
      return fun(detail::bind_first(std::forward<T>(t), q));
    }

  /// \brief Creates a new Proxy_Function object from a free function or member function and binds the first and second parameters of it
  /// \param[in] t Function / member to expose
  /// \param[in] q Value to bind to first parameter
  /// \param[in] r Value to bind to second parameter
  ///
  /// \b Example:
  /// \code
  /// struct MyClass
  /// {
  ///   void memberfunction(int);
  /// };
  /// 
  /// MyClass obj;
  /// chaiscript::ChaiScript chai;
  /// // Add function taking only no arguments, and permanently bound to "obj" and "1"
  /// // memberfunction() will be equivalent to obj.memberfunction(1)
  /// chai.add(fun(&MyClass::memberfunction, std::ref(obj), 1), "memberfunction"); 
  /// \endcode
  /// 
  /// \sa \ref adding_functions
  template<typename T, typename Q, typename R>
    Proxy_Function fun(T &&t, Q &&q, R &&r)
    {
      return fun(detail::bind_first(detail::bind_first(std::forward<T>(t), std::forward<Q>(q)), std::forward<R>(r)));
    }

}


#endif

