// This file is distributed under the BSD License.
// See "license.txt" for details.
// Copyright 2009-2012, Jonathan Turner (jonathan@emptycrate.com)
// Copyright 2009-2016, Jason Turner (jason@emptycrate.com)
// http://www.chaiscript.com


#ifndef CHAISCRIPT_PROXY_FUNCTIONS_HPP_
#define CHAISCRIPT_PROXY_FUNCTIONS_HPP_


#include <cassert>
#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>
#include <iterator>

#include "../chaiscript_defines.hpp"
#include "boxed_cast.hpp"
#include "boxed_value.hpp"
#include "proxy_functions_detail.hpp"
#include "type_info.hpp"
#include "dynamic_object.hpp"

namespace chaiscript {
class Type_Conversions;
namespace exception {
class bad_boxed_cast;
struct arity_error;
}  // namespace exception
}  // namespace chaiscript

namespace chaiscript
{
  class Boxed_Number;
  struct AST_Node;

  typedef std::shared_ptr<AST_Node> AST_NodePtr;

  namespace dispatch
  {
    template<typename FunctionType>
      std::function<FunctionType> functor(std::shared_ptr<const Proxy_Function_Base> func, const Type_Conversions_State *t_conversions);

    class Param_Types
    {
      public:
        Param_Types()
          : m_has_types(false),
            m_doti(user_type<Dynamic_Object>())
        {}

        Param_Types(std::vector<std::pair<std::string, Type_Info>> t_types)
          : m_types(std::move(t_types)),
            m_has_types(false),
            m_doti(user_type<Dynamic_Object>())
        {
          update_has_types();
        }

        void push_front(std::string t_name, Type_Info t_ti)
        {
          m_types.emplace(m_types.begin(), std::move(t_name), std::move(t_ti));
          update_has_types();
        }

        bool operator==(const Param_Types &t_rhs) const
        {
          return m_types == t_rhs.m_types;
        }

        bool match(const std::vector<Boxed_Value> &vals, const Type_Conversions_State &t_conversions) const
        {
          if (!m_has_types) return true;
          if (vals.size() != m_types.size()) return false;

          for (size_t i = 0; i < vals.size(); ++i)
          {
            const auto &name = m_types[i].first;
            if (!name.empty()) {
              const auto &bv = vals[i];

              if (bv.get_type_info().bare_equal(m_doti))
              {
                try {
                  const Dynamic_Object &d = boxed_cast<const Dynamic_Object &>(bv, &t_conversions);
                  return name == "Dynamic_Object" || d.get_type_name() == name;
                } catch (const std::bad_cast &) {
                  return false;
                } 
              } else {
                const auto &ti = m_types[i].second;
                if (!ti.is_undef())
                {
                  if (!bv.get_type_info().bare_equal(ti)) {
                    return false;
                  }
                } else {
                  return false;
                }
              }
            }
          }

          return true;
        }

        const std::vector<std::pair<std::string, Type_Info>> &types() const
        {
          return m_types;
        }

      private:
        void update_has_types()
        {
          for (const auto &type : m_types)
          {
            if (!type.first.empty())
            {
              m_has_types = true;
              return;
            }
          }

          m_has_types = false;
        }

        std::vector<std::pair<std::string, Type_Info>> m_types;
        bool m_has_types;
        Type_Info m_doti;

    };

    /**
     * Pure virtual base class for all Proxy_Function implementations
     * Proxy_Functions are a type erasure of type safe C++
     * function calls. At runtime parameter types are expected to be
     * tested against passed in types.
     * Dispatch_Engine only knows how to work with Proxy_Function, no other
     * function classes.
     */
    class Proxy_Function_Base
    {
      public:
        virtual ~Proxy_Function_Base() {}

        Boxed_Value operator()(const std::vector<Boxed_Value> &params, const chaiscript::Type_Conversions_State &t_conversions) const
        {
          if (m_arity < 0 || size_t(m_arity) == params.size()) {
            return do_call(params, t_conversions);
          } else {
            throw exception::arity_error(static_cast<int>(params.size()), m_arity);
          }
        }

        /// Returns a vector containing all of the types of the parameters the function returns/takes
        /// if the function is variadic or takes no arguments (arity of 0 or -1), the returned
        /// value contains exactly 1 Type_Info object: the return type
        /// \returns the types of all parameters. 
        const std::vector<Type_Info> &get_param_types() const { return m_types; }

        virtual bool operator==(const Proxy_Function_Base &) const = 0;
        virtual bool call_match(const std::vector<Boxed_Value> &vals, const Type_Conversions_State &t_conversions) const = 0;

        virtual bool is_attribute_function() const { return false; }

        bool has_arithmetic_param() const 
        {
          return m_has_arithmetic_param;
        }

        virtual std::vector<std::shared_ptr<const Proxy_Function_Base> > get_contained_functions() const
        {
          return std::vector<std::shared_ptr<const Proxy_Function_Base> >();
        }

        //! Return true if the function is a possible match
        //! to the passed in values
        bool filter(const std::vector<Boxed_Value> &vals, const Type_Conversions_State &t_conversions) const
        {
          if (m_arity < 0)
          {
            return true;
          } else if (static_cast<size_t>(m_arity) == vals.size()) {
            if (m_arity == 0)
            {
              return true;
            } else if (m_arity > 1) {
              return compare_type_to_param(m_types[1], vals[0], t_conversions) && compare_type_to_param(m_types[2], vals[1], t_conversions);
            } else {
              return compare_type_to_param(m_types[1], vals[0], t_conversions);
            }
          } else {
            return false;
          }
        }

        /// \returns the number of arguments the function takes or -1 if it is variadic
        int get_arity() const
        {
          return m_arity;
        }

        virtual std::string annotation() const = 0;

        static bool compare_type_to_param(const Type_Info &ti, const Boxed_Value &bv, const Type_Conversions_State &t_conversions)
        {
          if (ti.is_undef() 
              || ti.bare_equal(user_type<Boxed_Value>())
              || (!bv.get_type_info().is_undef()
                && ( (ti.bare_equal(user_type<Boxed_Number>()) && bv.get_type_info().is_arithmetic())
                  || ti.bare_equal(bv.get_type_info())
                  || bv.get_type_info().bare_equal(user_type<std::shared_ptr<const Proxy_Function_Base> >())
                  || t_conversions->converts(ti, bv.get_type_info()) 
                  )
                )
             )
          {
            return true;
          } else {
            return false;
          }
        }

        virtual bool compare_first_type(const Boxed_Value &bv, const Type_Conversions_State &t_conversions) const
        {
          return compare_type_to_param(m_types[1], bv, t_conversions);
        }

      protected:
        virtual Boxed_Value do_call(const std::vector<Boxed_Value> &params, const Type_Conversions_State &t_conversions) const = 0;

        Proxy_Function_Base(std::vector<Type_Info> t_types, int t_arity)
          : m_types(std::move(t_types)), m_arity(t_arity), m_has_arithmetic_param(false)
        {
          for (size_t i = 1; i < m_types.size(); ++i)
          {
            if (m_types[i].is_arithmetic())
            {
              m_has_arithmetic_param = true;
              return;
            }
          }

        }


        static bool compare_types(const std::vector<Type_Info> &tis, const std::vector<Boxed_Value> &bvs, const Type_Conversions_State &t_conversions)
        {
          if (tis.size() - 1 != bvs.size())
          {
            return false;
          } else {
            const size_t size = bvs.size();
            for (size_t i = 0; i < size; ++i)
            {
              if (!compare_type_to_param(tis[i + 1], bvs[i], t_conversions)) { return false;  }
            }
          }
          return true;
        }

        std::vector<Type_Info> m_types;
        int m_arity;
        bool m_has_arithmetic_param;
    };
  }

  /// \brief Common typedef used for passing of any registered function in ChaiScript
  typedef std::shared_ptr<dispatch::Proxy_Function_Base> Proxy_Function;

  /// \brief Const version of Proxy_Function. Points to a const Proxy_Function. This is how most registered functions
  ///        are handled internally.
  typedef std::shared_ptr<const dispatch::Proxy_Function_Base> Const_Proxy_Function;

  namespace exception
  {
    /// \brief  Exception thrown if a function's guard fails
    class guard_error : public std::runtime_error
    {
      public:
        guard_error() CHAISCRIPT_NOEXCEPT
          : std::runtime_error("Guard evaluation failed")
        { }

        guard_error(const guard_error &) = default;

        virtual ~guard_error() CHAISCRIPT_NOEXCEPT
        { }
    };
  }

  namespace dispatch
  {
    /**
     * A Proxy_Function implementation that is not type safe, the called function
     * is expecting a vector<Boxed_Value> that it works with how it chooses.
     */
    class Dynamic_Proxy_Function : public Proxy_Function_Base
    {
      public:
        Dynamic_Proxy_Function(
            int t_arity=-1,
            AST_NodePtr t_parsenode = AST_NodePtr(),
            Param_Types t_param_types = Param_Types(),
            std::string t_description = "",
            Proxy_Function t_guard = Proxy_Function())
          : Proxy_Function_Base(build_param_type_list(t_param_types), t_arity),
            m_param_types(std::move(t_param_types)),
            m_guard(std::move(t_guard)), m_parsenode(std::move(t_parsenode)), m_description(std::move(t_description))
        {
        }

        virtual ~Dynamic_Proxy_Function() {}

        virtual bool operator==(const Proxy_Function_Base &rhs) const CHAISCRIPT_OVERRIDE
        {
          const Dynamic_Proxy_Function *prhs = dynamic_cast<const Dynamic_Proxy_Function *>(&rhs);

          return this == &rhs
            || (prhs
                && this->m_arity == prhs->m_arity
                && !this->m_guard && !prhs->m_guard
                && this->m_param_types == prhs->m_param_types);
        }

        virtual bool call_match(const std::vector<Boxed_Value> &vals, const Type_Conversions_State &t_conversions) const CHAISCRIPT_OVERRIDE
        {
          return (m_arity < 0 || (vals.size() == size_t(m_arity) && m_param_types.match(vals, t_conversions)))
            && test_guard(vals, t_conversions);
        }


        Proxy_Function get_guard() const
        {
          return m_guard;
        }

        AST_NodePtr get_parse_tree() const
        {
          return m_parsenode;
        }

        virtual std::string annotation() const CHAISCRIPT_OVERRIDE
        {
          return m_description;
        }


      protected:
        bool test_guard(const std::vector<Boxed_Value> &params, const Type_Conversions_State &t_conversions) const
        {
          if (m_guard)
          {
            try {
              return boxed_cast<bool>((*m_guard)(params, t_conversions));
            } catch (const exception::arity_error &) {
              return false;
            } catch (const exception::bad_boxed_cast &) {
              return false;
            }
          } else {
            return true;
          }
        }

      private:
        static std::vector<Type_Info> build_param_type_list(const Param_Types &t_types)
        {
          // For the return type
          std::vector<Type_Info> types{chaiscript::detail::Get_Type_Info<Boxed_Value>::get()};

          for (const auto &t : t_types.types())
          {
            if (t.second.is_undef()) {
              types.push_back(chaiscript::detail::Get_Type_Info<Boxed_Value>::get());
            } else {
              types.push_back(t.second);
            }
          }

          return types;
        }

        Param_Types m_param_types;
        Proxy_Function m_guard;
        AST_NodePtr m_parsenode;
        std::string m_description;
    };



    template<typename Callable>
    class Dynamic_Proxy_Function_Impl : public Dynamic_Proxy_Function
    {
      public:
        Dynamic_Proxy_Function_Impl(
            Callable t_f, 
            int t_arity=-1,
            AST_NodePtr t_parsenode = AST_NodePtr(),
            Param_Types t_param_types = Param_Types(),
            std::string t_description = "",
            Proxy_Function t_guard = Proxy_Function())
          : Dynamic_Proxy_Function(
                t_arity,
                std::move(t_parsenode),
                std::move(t_param_types),
                std::move(t_description),
                std::move(t_guard)
              ),
            m_f(std::move(t_f))
        {
        }

        virtual ~Dynamic_Proxy_Function_Impl() {}


      protected:
        virtual Boxed_Value do_call(const std::vector<Boxed_Value> &params, const Type_Conversions_State &t_conversions) const CHAISCRIPT_OVERRIDE
        {
          if (call_match(params, t_conversions) && test_guard(params, t_conversions))
          {
            return m_f(params);
          } else {
            throw exception::guard_error();
          }
        }

      private:

        Callable m_f;
    };

    template<typename Callable, typename ... Arg>
    Proxy_Function make_dynamic_proxy_function(Callable &&c, Arg&& ... a)
    {
      return chaiscript::make_shared<dispatch::Proxy_Function_Base, dispatch::Dynamic_Proxy_Function_Impl<Callable>>(
          std::forward<Callable>(c), std::forward<Arg>(a)...);
    }

    /// An object used by Bound_Function to represent "_" parameters
    /// of a binding. This allows for unbound parameters during bind.
    struct Placeholder_Object
    {
    };

    /// An implementation of Proxy_Function that takes a Proxy_Function
    /// and substitutes bound parameters into the parameter list
    /// at runtime, when call() is executed.
    /// it is used for bind(function, param1, _, param2) style calls
    class Bound_Function : public Proxy_Function_Base
    {
      public:
        Bound_Function(const Const_Proxy_Function &t_f, 
            const std::vector<Boxed_Value> &t_args)
          : Proxy_Function_Base(build_param_type_info(t_f, t_args), (t_f->get_arity()<0?-1:static_cast<int>(build_param_type_info(t_f, t_args).size())-1)),
            m_f(t_f), m_args(t_args)
        {
          assert(m_f->get_arity() < 0 || m_f->get_arity() == static_cast<int>(m_args.size()));
        }

        virtual bool operator==(const Proxy_Function_Base &t_f) const CHAISCRIPT_OVERRIDE
        {
          return &t_f == this;
        }

        virtual ~Bound_Function() {}

        virtual bool call_match(const std::vector<Boxed_Value> &vals, const Type_Conversions_State &t_conversions) const CHAISCRIPT_OVERRIDE
        {
          return m_f->call_match(build_param_list(vals), t_conversions);
        }

        virtual std::vector<Const_Proxy_Function> get_contained_functions() const CHAISCRIPT_OVERRIDE
        {
          return std::vector<Const_Proxy_Function>{m_f};
        }


        std::vector<Boxed_Value> build_param_list(const std::vector<Boxed_Value> &params) const
        {
          auto parg = params.begin();
          auto barg = m_args.begin();

          std::vector<Boxed_Value> args;

          while (!(parg == params.end() && barg == m_args.end()))
          {
            while (barg != m_args.end() 
                && !(barg->get_type_info() == chaiscript::detail::Get_Type_Info<Placeholder_Object>::get()))
            {
              args.push_back(*barg);
              ++barg;
            }

            if (parg != params.end())
            {
              args.push_back(*parg);
              ++parg;
            }

            if (barg != m_args.end() 
                && barg->get_type_info() == chaiscript::detail::Get_Type_Info<Placeholder_Object>::get())
            {
              ++barg;
            } 
          }
          return args;
        }

        virtual std::string annotation() const CHAISCRIPT_OVERRIDE
        {
          return "Bound: " + m_f->annotation();
        }

      protected:
        static std::vector<Type_Info> build_param_type_info(const Const_Proxy_Function &t_f, 
            const std::vector<Boxed_Value> &t_args)
        {
          assert(t_f->get_arity() < 0 || t_f->get_arity() == static_cast<int>(t_args.size()));

          if (t_f->get_arity() < 0) { return std::vector<Type_Info>(); }

          std::vector<Type_Info> types = t_f->get_param_types();
          assert(types.size() == t_args.size() + 1);

#ifdef CHAISCRIPT_MSVC_12
#pragma warning(push)
#pragma warning(disable : 6011)
#endif
          // this analysis warning is invalid in MSVC12 and doesn't exist in MSVC14
          std::vector<Type_Info> retval{types[0]};
#ifdef CHAISCRIPT_MSVC_12
#pragma warning(pop)
#endif

          for (size_t i = 0; i < types.size() - 1; ++i)
          {
            if (t_args[i].get_type_info() == chaiscript::detail::Get_Type_Info<Placeholder_Object>::get())
            {
              retval.push_back(types[i+1]);
            }
          }

          return retval;
        }

        virtual Boxed_Value do_call(const std::vector<Boxed_Value> &params, const Type_Conversions_State &t_conversions) const CHAISCRIPT_OVERRIDE
        {
          return (*m_f)(build_param_list(params), t_conversions);
        }

      private:
        Const_Proxy_Function m_f;
        std::vector<Boxed_Value> m_args;
    };

    class Proxy_Function_Impl_Base : public Proxy_Function_Base
    {
      public:
        Proxy_Function_Impl_Base(const std::vector<Type_Info> &t_types)
          : Proxy_Function_Base(t_types, static_cast<int>(t_types.size()) - 1)
        {
        }

        virtual ~Proxy_Function_Impl_Base() {}

        virtual std::string annotation() const CHAISCRIPT_OVERRIDE
        {
          return "";
        }

        virtual bool call_match(const std::vector<Boxed_Value> &vals, const Type_Conversions_State &t_conversions) const CHAISCRIPT_OVERRIDE
        {
          return static_cast<int>(vals.size()) == get_arity() && (compare_types(m_types, vals, t_conversions) && compare_types_with_cast(vals, t_conversions));
        }

        virtual bool compare_types_with_cast(const std::vector<Boxed_Value> &vals, const Type_Conversions_State &t_conversions) const = 0;
    };



    /// For any callable object
    template<typename Func, typename Callable>
      class Proxy_Function_Callable_Impl : public Proxy_Function_Impl_Base
    {
      public:
        Proxy_Function_Callable_Impl(Callable f)
          : Proxy_Function_Impl_Base(detail::build_param_type_list(static_cast<Func *>(nullptr))),
            m_f(std::move(f))
        {
        }

        virtual ~Proxy_Function_Callable_Impl() {}

        virtual bool compare_types_with_cast(const std::vector<Boxed_Value> &vals, const Type_Conversions_State &t_conversions) const CHAISCRIPT_OVERRIDE
        {
          return detail::compare_types_cast(static_cast<Func *>(nullptr), vals, t_conversions);
        }

        virtual bool operator==(const Proxy_Function_Base &t_func) const CHAISCRIPT_OVERRIDE
        {
          return dynamic_cast<const Proxy_Function_Callable_Impl<Func, Callable> *>(&t_func) != nullptr;
        }


      protected:
        virtual Boxed_Value do_call(const std::vector<Boxed_Value> &params, const Type_Conversions_State &t_conversions) const CHAISCRIPT_OVERRIDE
        {
          typedef typename detail::Function_Signature<Func>::Return_Type Return_Type;
          return detail::Do_Call<Return_Type>::template go<Func>(m_f, params, t_conversions);
        }

      private:
        Callable m_f;
    };


    class Assignable_Proxy_Function : public Proxy_Function_Impl_Base
    {
      public:
        Assignable_Proxy_Function(const std::vector<Type_Info> &t_types)
          : Proxy_Function_Impl_Base(t_types)
        {
        }

        virtual ~Assignable_Proxy_Function() {}


        virtual void assign(const std::shared_ptr<const Proxy_Function_Base> &t_rhs)  = 0;


    };

    template<typename Func>
      class Assignable_Proxy_Function_Impl : public Assignable_Proxy_Function
    {
      public:
        Assignable_Proxy_Function_Impl(std::reference_wrapper<std::function<Func>> t_f, std::shared_ptr<std::function<Func>> t_ptr)
          : Assignable_Proxy_Function(detail::build_param_type_list(static_cast<Func *>(nullptr))),
            m_f(std::move(t_f)), m_shared_ptr_holder(std::move(t_ptr))
        {
          assert(!m_shared_ptr_holder || m_shared_ptr_holder.get() == &m_f.get());
        }

        virtual ~Assignable_Proxy_Function_Impl() {}

        virtual bool compare_types_with_cast(const std::vector<Boxed_Value> &vals, const Type_Conversions_State &t_conversions) const CHAISCRIPT_OVERRIDE
        {
          return detail::compare_types_cast(static_cast<Func *>(nullptr), vals, t_conversions);
        }

        virtual bool operator==(const Proxy_Function_Base &t_func) const CHAISCRIPT_OVERRIDE
        {
          return dynamic_cast<const Assignable_Proxy_Function_Impl<Func> *>(&t_func) != nullptr;
        }

        std::function<Func> internal_function() const
        {
          return m_f.get();
        }

        virtual void assign(const std::shared_ptr<const Proxy_Function_Base> &t_rhs) CHAISCRIPT_OVERRIDE {
          m_f.get() = dispatch::functor<Func>(t_rhs, nullptr);
        }

      protected:
        virtual Boxed_Value do_call(const std::vector<Boxed_Value> &params, const Type_Conversions_State &t_conversions) const CHAISCRIPT_OVERRIDE
        {
          return detail::Do_Call<typename std::function<Func>::result_type>::template go<Func>(m_f.get(), params, t_conversions);
        }


      private:
        std::reference_wrapper<std::function<Func>> m_f;
        std::shared_ptr<std::function<Func>> m_shared_ptr_holder;
    };
    /// Attribute getter Proxy_Function implementation
    template<typename T, typename Class>
      class Attribute_Access : public Proxy_Function_Base
    {
      public:
        Attribute_Access(T Class::* t_attr)
          : Proxy_Function_Base(param_types(), 1),
            m_attr(t_attr)
        {
        }

        virtual ~Attribute_Access() {}

        virtual bool is_attribute_function() const CHAISCRIPT_OVERRIDE { return true; } 

        virtual bool operator==(const Proxy_Function_Base &t_func) const CHAISCRIPT_OVERRIDE
        {
          const Attribute_Access<T, Class> * aa 
            = dynamic_cast<const Attribute_Access<T, Class> *>(&t_func);

          if (aa) {
            return m_attr == aa->m_attr;
          } else {
            return false;
          }
        }

        virtual bool call_match(const std::vector<Boxed_Value> &vals, const Type_Conversions_State &) const CHAISCRIPT_OVERRIDE
        {
          if (vals.size() != 1)
          {
            return false;
          }

          return vals[0].get_type_info().bare_equal(user_type<Class>());
        }

        virtual std::string annotation() const CHAISCRIPT_OVERRIDE
        {
          return "";
        }

      protected:
        virtual Boxed_Value do_call(const std::vector<Boxed_Value> &params, const Type_Conversions_State &t_conversions) const CHAISCRIPT_OVERRIDE
        {
          const Boxed_Value &bv = params[0];
          if (bv.is_const())
          {
            const Class *o = boxed_cast<const Class *>(bv, &t_conversions);
            return detail::Handle_Return<const typename std::add_lvalue_reference<T>::type>::handle(o->*m_attr);
          } else {
            Class *o = boxed_cast<Class *>(bv, &t_conversions);
            return detail::Handle_Return<typename std::add_lvalue_reference<T>::type>::handle(o->*m_attr);
          }
        }

      private:
        static std::vector<Type_Info> param_types()
        {
          return {user_type<T>(), user_type<Class>()};
        }

        T Class::* m_attr;
    };
  }

  namespace exception
  {
     /// \brief Exception thrown in the case that a method dispatch fails
     ///        because no matching function was found
     /// 
     /// May be thrown due to an arity_error, a guard_error or a bad_boxed_cast
     /// exception
    class dispatch_error : public std::runtime_error
    {
      public:
        dispatch_error(std::vector<Boxed_Value> t_parameters, 
            std::vector<Const_Proxy_Function> t_functions)
          : std::runtime_error("Error with function dispatch"), parameters(std::move(t_parameters)), functions(std::move(t_functions))
        {
        }

        dispatch_error(std::vector<Boxed_Value> t_parameters, 
            std::vector<Const_Proxy_Function> t_functions,
            const std::string &t_desc)
          : std::runtime_error(t_desc), parameters(std::move(t_parameters)), functions(std::move(t_functions))
        {
        }


        dispatch_error(const dispatch_error &) = default;
        virtual ~dispatch_error() CHAISCRIPT_NOEXCEPT {}

        std::vector<Boxed_Value> parameters;
        std::vector<Const_Proxy_Function> functions;
    };
  } 

  namespace dispatch
  {
    namespace detail 
    {
      template<typename FuncType>
        bool types_match_except_for_arithmetic(const FuncType &t_func, const std::vector<Boxed_Value> &plist,
            const Type_Conversions_State &t_conversions)
        {
          const std::vector<Type_Info> &types = t_func->get_param_types();

          if (t_func->get_arity() == -1) return false;

          assert(plist.size() == types.size() - 1);

          return std::mismatch(plist.begin(), plist.end(),
                               types.begin()+1,
                               [&](const Boxed_Value &bv, const Type_Info &ti) {
                                 return Proxy_Function_Base::compare_type_to_param(ti, bv, t_conversions)
                                       || (bv.get_type_info().is_arithmetic() && ti.is_arithmetic());
                               }
              ) == std::make_pair(plist.end(), types.end());
        }

      template<typename InItr, typename Funcs>
        Boxed_Value dispatch_with_conversions(InItr begin, const InItr &end, const std::vector<Boxed_Value> &plist, 
            const Type_Conversions_State &t_conversions, const Funcs &t_funcs)
        {
          InItr matching_func(end);

          while (begin != end)
          {
            if (types_match_except_for_arithmetic(begin->second, plist, t_conversions))
            {
              if (matching_func == end)
              {
                matching_func = begin;
              } else {
                // handle const members vs non-const member, which is not really ambiguous
                const auto &mat_fun_param_types = matching_func->second->get_param_types();
                const auto &next_fun_param_types = begin->second->get_param_types();

                if (plist[0].is_const() && !mat_fun_param_types[1].is_const() && next_fun_param_types[1].is_const()) {
                  matching_func = begin; // keep the new one, the const/non-const matchup is correct
                } else if (!plist[0].is_const() && !mat_fun_param_types[1].is_const() && next_fun_param_types[1].is_const()) {
                  // keep the old one, it has a better const/non-const matchup
                } else {
                  // ambiguous function call
                  throw exception::dispatch_error(plist, std::vector<Const_Proxy_Function>(t_funcs.begin(), t_funcs.end()));
                }
              }
            }

            ++begin;
          }

          if (matching_func == end)
          {
            // no appropriate function to attempt arithmetic type conversion on
            throw exception::dispatch_error(plist, std::vector<Const_Proxy_Function>(t_funcs.begin(), t_funcs.end()));
          }


          std::vector<Boxed_Value> newplist;
          newplist.reserve(plist.size());

          const std::vector<Type_Info> &tis = matching_func->second->get_param_types();
          std::transform(tis.begin() + 1, tis.end(),
                         plist.begin(),
                         std::back_inserter(newplist),
                         [](const Type_Info &ti, const Boxed_Value &param) -> Boxed_Value {
                           if (ti.is_arithmetic() && param.get_type_info().is_arithmetic()) {
                             return Boxed_Number(param).get_as(ti).bv;
                           } else {
                             return param;
                           }
                         }
                       );



          try {
            return (*(matching_func->second))(newplist, t_conversions);
          } catch (const exception::bad_boxed_cast &) {
            //parameter failed to cast
          } catch (const exception::arity_error &) {
            //invalid num params
          } catch (const exception::guard_error &) {
            //guard failed to allow the function to execute
          }

          throw exception::dispatch_error(plist, std::vector<Const_Proxy_Function>(t_funcs.begin(), t_funcs.end()));

        }
    }

    /**
     * Take a vector of functions and a vector of parameters. Attempt to execute
     * each function against the set of parameters, in order, until a matching
     * function is found or throw dispatch_error if no matching function is found
     */
    template<typename Funcs>
      Boxed_Value dispatch(const Funcs &funcs,
          const std::vector<Boxed_Value> &plist, const Type_Conversions_State &t_conversions)
      {
        std::vector<std::pair<size_t, const Proxy_Function_Base *>> ordered_funcs;
        ordered_funcs.reserve(funcs.size());

        for (const auto &func : funcs)
        {
          const auto arity = func->get_arity();

          if (arity == -1)
          {
            ordered_funcs.emplace_back(plist.size(), func.get());
          } else if (arity == static_cast<int>(plist.size())) {
            size_t numdiffs = 0;
            for (size_t i = 0; i < plist.size(); ++i)
            {
              if (!func->get_param_types()[i+1].bare_equal(plist[i].get_type_info()))
              {
                ++numdiffs;
              }
            }
            ordered_funcs.emplace_back(numdiffs, func.get());
          }
        }


        for (size_t i = 0; i <= plist.size(); ++i)
        {
          for (const auto &func : ordered_funcs )
          {
            try {
              if (func.first == i && func.second->filter(plist, t_conversions))
              {
                return (*(func.second))(plist, t_conversions);
              }
            } catch (const exception::bad_boxed_cast &) {
              //parameter failed to cast, try again
            } catch (const exception::arity_error &) {
              //invalid num params, try again
            } catch (const exception::guard_error &) {
              //guard failed to allow the function to execute,
              //try again
            }
          }
        }

        return detail::dispatch_with_conversions(ordered_funcs.cbegin(), ordered_funcs.cend(), plist, t_conversions, funcs);
      }
  }
}


#endif
