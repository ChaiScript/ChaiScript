// This file is distributed under the BSD License.
// See "license.txt" for details.
// Copyright 2009-2012, Jonathan Turner (jonathan@emptycrate.com)
// Copyright 2009-2014, Jason Turner (jason@emptycrate.com)
// http://www.chaiscript.com


#ifndef CHAISCRIPT_PROXY_FUNCTIONS_HPP_
#define CHAISCRIPT_PROXY_FUNCTIONS_HPP_


#include <algorithm>
#include <cassert>
#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

#include "../chaiscript_defines.hpp"
#include "boxed_cast.hpp"
#include "boxed_cast_helper.hpp"
#include "boxed_value.hpp"
#include "proxy_functions_detail.hpp"
#include "type_info.hpp"

namespace chaiscript {
class Dynamic_Cast_Conversions;
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

        Boxed_Value operator()(const std::vector<Boxed_Value> &params, const chaiscript::Dynamic_Cast_Conversions &t_conversions) const
        {
          Boxed_Value bv = do_call(params, t_conversions);
          return bv;
        }

        /// Returns a vector containing all of the types of the parameters the function returns/takes
        /// if the function is variadic or takes no arguments (arity of 0 or -1), the returned
        /// value containes exactly 1 Type_Info object: the return type
        /// \returns the types of all parameters. 
        const std::vector<Type_Info> &get_param_types() const { return m_types; }

        virtual bool operator==(const Proxy_Function_Base &) const = 0;
        virtual bool call_match(const std::vector<Boxed_Value> &vals, const Dynamic_Cast_Conversions &t_conversions) const = 0;

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
        bool filter(const std::vector<Boxed_Value> &vals, const Dynamic_Cast_Conversions &t_conversions) const
        {
          int arity = get_arity();

          if (arity < 0)
          {
            return true;
          } else if (size_t(arity) == vals.size()) {
            if (arity == 0)
            {
              return true;
            } else {
              return compare_first_type(vals[0], t_conversions);
            }
          } else {
            return false;
          }
        }

        /// \returns the number of arguments the function takes or -1 if it is variadic
        virtual int get_arity() const = 0;

        virtual std::string annotation() const = 0;

        static bool compare_type_to_param(const Type_Info &ti, const Boxed_Value &bv, const Dynamic_Cast_Conversions &t_conversions)
        {
          if (ti.is_undef() 
              || ti.bare_equal(user_type<Boxed_Value>())
              || (!bv.get_type_info().is_undef()
                && (ti.bare_equal(user_type<Boxed_Number>())
                  || ti.bare_equal(bv.get_type_info())
                  || bv.get_type_info().bare_equal(user_type<std::shared_ptr<const Proxy_Function_Base> >())
                  || t_conversions.dynamic_cast_converts(ti, bv.get_type_info()) 
                  )
                )
             )
          {
            return true;
          } else {
            return false;
          }
        }
      protected:
        virtual Boxed_Value do_call(const std::vector<Boxed_Value> &params, const Dynamic_Cast_Conversions &t_conversions) const = 0;

        Proxy_Function_Base(std::vector<Type_Info> t_types)
          : m_types(std::move(t_types)), m_has_arithmetic_param(false)
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

        virtual bool compare_first_type(const Boxed_Value &bv, const Dynamic_Cast_Conversions &t_conversions) const
        {
          const std::vector<Type_Info> &types = get_param_types();

          if (types.size() < 2)
          {
            return true;
          }

          const Type_Info &ti = types[1];
          return compare_type_to_param(ti, bv, t_conversions);

        }

        static bool compare_types(const std::vector<Type_Info> &tis, const std::vector<Boxed_Value> &bvs)
        {
          if (tis.size() - 1 != bvs.size())
          {
            return false;
          } else {
            size_t size = bvs.size();
            for (size_t i = 0; i < size; ++i)
            {
              if (!(tis[i+1].bare_equal(bvs[i].get_type_info()) && tis[i+1].is_const() >= bvs[i].get_type_info().is_const() ))
              {
                return false;
              }
            }
          }
          return true;
        }

        std::vector<Type_Info> m_types;
        bool m_has_arithmetic_param;

    };
  }

  /// \brief Common typedef used for passing of any registered function in ChaiScript
  typedef std::shared_ptr<dispatch::Proxy_Function_Base> Proxy_Function;

  /// \brief Const version of Proxy_Function chaiscript. Points to a const Proxy_Function. This is how most registered functions
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
            std::function<Boxed_Value (const std::vector<Boxed_Value> &)> t_f, 
            int t_arity=-1,
            AST_NodePtr t_parsenode = AST_NodePtr(),
            std::string t_description = "",
            Proxy_Function t_guard = Proxy_Function())
          : Proxy_Function_Base(build_param_type_list(t_arity)),
            m_f(std::move(t_f)), m_arity(t_arity), m_description(std::move(t_description)), m_guard(std::move(t_guard)), m_parsenode(std::move(t_parsenode))
        {
        }

        virtual ~Dynamic_Proxy_Function() {}

        virtual bool operator==(const Proxy_Function_Base &rhs) const CHAISCRIPT_OVERRIDE
        {
          const Dynamic_Proxy_Function *prhs = dynamic_cast<const Dynamic_Proxy_Function *>(&rhs);

          return this == &rhs
            || (prhs
                && this->m_arity == prhs->m_arity
                && !this->m_guard && !prhs->m_guard);
        }

        virtual bool call_match(const std::vector<Boxed_Value> &vals, const Dynamic_Cast_Conversions &t_conversions) const CHAISCRIPT_OVERRIDE
        {
          return (m_arity < 0 || vals.size() == size_t(m_arity))
            && test_guard(vals, t_conversions);
        }    

        virtual int get_arity() const CHAISCRIPT_OVERRIDE
        {
          return m_arity;
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
        virtual Boxed_Value do_call(const std::vector<Boxed_Value> &params, const Dynamic_Cast_Conversions &t_conversions) const CHAISCRIPT_OVERRIDE
        {
          if (m_arity < 0 || params.size() == size_t(m_arity))
          {

            if (test_guard(params, t_conversions))
            {
              return m_f(params);
            } else {
              throw exception::guard_error();
            }

          } else {
            throw exception::arity_error(static_cast<int>(params.size()), m_arity);
          } 
        }

      private:
        bool test_guard(const std::vector<Boxed_Value> &params, const Dynamic_Cast_Conversions &t_conversions) const
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

        static std::vector<Type_Info> build_param_type_list(int arity)
        {
          std::vector<Type_Info> types;

          // For the return type
          types.push_back(chaiscript::detail::Get_Type_Info<Boxed_Value>::get());

          if (arity > 0)
          {
            for (int i = 0; i < arity; ++i)
            {
              types.push_back(chaiscript::detail::Get_Type_Info<Boxed_Value>::get());
            }
          }

          return types;
        }

        std::function<Boxed_Value (const std::vector<Boxed_Value> &)> m_f;
        int m_arity;
        std::string m_description;
        Proxy_Function m_guard;
        AST_NodePtr m_parsenode;
    };

    /**
     * An object used by Bound_Function to represent "_" parameters
     * of a binding. This allows for unbound parameters during bind.
     */
    struct Placeholder_Object
    {
    };

    /**
     * An implementation of Proxy_Function that takes a Proxy_Function
     * and substitutes bound parameters into the parameter list
     * at runtime, when call() is executed.
     * it is used for bind(function, param1, _, param2) style calls
     */
    class Bound_Function : public Proxy_Function_Base
    {
      public:
        Bound_Function(const Const_Proxy_Function &t_f, 
            const std::vector<Boxed_Value> &t_args)
          : Proxy_Function_Base(build_param_type_info(t_f, t_args)),
            m_f(t_f), m_args(t_args), m_arity(t_f->get_arity()<0?-1:static_cast<int>(get_param_types().size())-1)
        {
          assert(m_f->get_arity() < 0 || m_f->get_arity() == static_cast<int>(m_args.size()));
        }

        virtual bool operator==(const Proxy_Function_Base &t_f) const CHAISCRIPT_OVERRIDE
        {
          return &t_f == this;
        }

        virtual ~Bound_Function() {}

        virtual bool call_match(const std::vector<Boxed_Value> &vals, const Dynamic_Cast_Conversions &t_conversions) const CHAISCRIPT_OVERRIDE
        {
          return m_f->call_match(build_param_list(vals), t_conversions);
        }

        virtual std::vector<Const_Proxy_Function> get_contained_functions() const CHAISCRIPT_OVERRIDE
        {
          std::vector<Const_Proxy_Function> fs;
          fs.push_back(m_f);
          return fs;
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

        virtual int get_arity() const CHAISCRIPT_OVERRIDE
        {
          return m_arity;
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

          std::vector<Type_Info> retval;
          retval.push_back(types[0]);
          for (size_t i = 0; i < types.size()-1; ++i)
          {
            if (t_args[i].get_type_info() == chaiscript::detail::Get_Type_Info<Placeholder_Object>::get())
            {
              retval.push_back(types[i+1]);
            }
          }

          return retval;
        }

        virtual Boxed_Value do_call(const std::vector<Boxed_Value> &params, const Dynamic_Cast_Conversions &t_conversions) const CHAISCRIPT_OVERRIDE
        {
          return (*m_f)(build_param_list(params), t_conversions);
        }

      private:
        Const_Proxy_Function m_f;
        std::vector<Boxed_Value> m_args;
        int m_arity;
    };

    class Proxy_Function_Impl_Base : public Proxy_Function_Base
    {
      public:
        Proxy_Function_Impl_Base(std::vector<Type_Info> t_types)
          : Proxy_Function_Base(std::move(t_types))
        {
        }

        virtual ~Proxy_Function_Impl_Base() {}

        virtual int get_arity() const CHAISCRIPT_OVERRIDE
        {
          return static_cast<int>(m_types.size()) - 1;
        }

        virtual std::string annotation() const CHAISCRIPT_OVERRIDE
        {
          return "";
        }

        virtual bool call_match(const std::vector<Boxed_Value> &vals, const Dynamic_Cast_Conversions &t_conversions) const CHAISCRIPT_OVERRIDE
        {
          if (int(vals.size()) != get_arity()) 
          {
            return false;
          }

          return compare_types(m_types, vals) || compare_types_with_cast(vals, t_conversions);
        }

        virtual bool compare_types_with_cast(const std::vector<Boxed_Value> &vals, const Dynamic_Cast_Conversions &t_conversions) const = 0;
    };

    /**
     * The standard typesafe function call implementation of Proxy_Function
     * It takes a std::function<> object and performs runtime 
     * type checking of Boxed_Value parameters, in a type safe manner
     */
    template<typename Func>
      class Proxy_Function_Impl : public Proxy_Function_Impl_Base
    {
      public:
        Proxy_Function_Impl(std::function<Func> f)
          : Proxy_Function_Impl_Base(detail::build_param_type_list(static_cast<Func *>(nullptr))),
            m_f(std::move(f)), m_dummy_func(nullptr)
        {
        }

        virtual ~Proxy_Function_Impl() {}

        virtual bool compare_types_with_cast(const std::vector<Boxed_Value> &vals, const Dynamic_Cast_Conversions &t_conversions) const CHAISCRIPT_OVERRIDE
        {
          return detail::compare_types_cast(m_dummy_func, vals, t_conversions);
        }

        virtual bool operator==(const Proxy_Function_Base &t_func) const CHAISCRIPT_OVERRIDE
        {
          return dynamic_cast<const Proxy_Function_Impl<Func> *>(&t_func) != nullptr;
        }

        std::function<Func> internal_function() const
        {
          return m_f;
        }

      protected:
        virtual Boxed_Value do_call(const std::vector<Boxed_Value> &params, const Dynamic_Cast_Conversions &t_conversions) const
        {
          return detail::Do_Call<typename std::function<Func>::result_type>::go(m_f, params, t_conversions);
        }

      private:
        std::function<Func> m_f;
        Func *m_dummy_func;
    };

    /**
     * Attribute getter Proxy_Function implementation
     */
    template<typename T, typename Class>
      class Attribute_Access : public Proxy_Function_Base
    {
      public:
        Attribute_Access(T Class::* t_attr)
          : Proxy_Function_Base(param_types()),
          m_attr(t_attr)
        {
        }

        virtual ~Attribute_Access() {}

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


        virtual int get_arity() const CHAISCRIPT_OVERRIDE
        {
          return 1;
        }

        virtual bool call_match(const std::vector<Boxed_Value> &vals, const Dynamic_Cast_Conversions &) const CHAISCRIPT_OVERRIDE
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
        virtual Boxed_Value do_call(const std::vector<Boxed_Value> &params, const Dynamic_Cast_Conversions &t_conversions) const CHAISCRIPT_OVERRIDE
        {
          if (params.size() == 1)
          {
            const Boxed_Value &bv = params[0];
            if (bv.is_const())
            {
              const Class *o = boxed_cast<const Class *>(bv, &t_conversions);
              return detail::Handle_Return<typename std::add_lvalue_reference<T>::type>::handle(o->*m_attr);
            } else {
              Class *o = boxed_cast<Class *>(bv, &t_conversions);
              return detail::Handle_Return<typename std::add_lvalue_reference<T>::type>::handle(o->*m_attr);
            }
          } else {
            throw exception::arity_error(static_cast<int>(params.size()), 1);
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
            const Dynamic_Cast_Conversions &t_conversions)
        {
          if (t_func->get_arity() != static_cast<int>(plist.size()))
          {
            return false;
          }

          const std::vector<Type_Info> &types = t_func->get_param_types();

          assert(plist.size() == types.size() - 1);

          for (size_t i = 0; i < plist.size(); ++i)
          {
            if (Proxy_Function_Base::compare_type_to_param(types[i+1], plist[i], t_conversions)
                || (types[i+1].is_arithmetic() && plist[i].get_type_info().is_arithmetic()))
            {
              // types continue to match
            } else {
              return false;
            }
          }

          // all types match
          return true;
        }

      template<typename InItr>
        Boxed_Value dispatch_with_conversions(InItr begin, const InItr &end, const std::vector<Boxed_Value> &plist, 
            const Dynamic_Cast_Conversions &t_conversions)
        {
          InItr orig(begin);

          InItr matching_func(end);

          while (begin != end)
          {
            if (types_match_except_for_arithmetic(*begin, plist, t_conversions))
            {
              if (matching_func == end)
              {
                matching_func = begin;
              } else {
                // More than one function matches, not attempting
                throw exception::dispatch_error(plist, std::vector<Const_Proxy_Function>(orig, end));
              }
            }

            ++begin;
          }

          if (matching_func == end)
          {
            // no appropriate function to attempt arithmetic type conversion on
            throw exception::dispatch_error(plist, std::vector<Const_Proxy_Function>(orig, end));
          }


          std::vector<Boxed_Value> newplist;
          const std::vector<Type_Info> &tis = (*matching_func)->get_param_types();

          for (size_t i = 0; i < plist.size(); ++i)
          {
            if (tis[i+1].is_arithmetic()
                && plist[i].get_type_info().is_arithmetic()) {
              newplist.push_back(Boxed_Number(plist[i]).get_as(tis[i+1]).bv);
            } else {
              newplist.push_back(plist[i]);
            }
          }

          try {
            return (*(*matching_func))(newplist, t_conversions);
          } catch (const exception::bad_boxed_cast &) {
            //parameter failed to cast
          } catch (const exception::arity_error &) {
            //invalid num params
          } catch (const exception::guard_error &) {
            //guard failed to allow the function to execute
          }

          throw exception::dispatch_error(plist, std::vector<Const_Proxy_Function>(orig, end));

        }
    }

    /**
     * Take a vector of functions and a vector of parameters. Attempt to execute
     * each function against the set of parameters, in order, until a matching
     * function is found or throw dispatch_error if no matching function is found
     */
    template<typename InItr>
      Boxed_Value dispatch(InItr begin, const InItr &end,
          const std::vector<Boxed_Value> &plist, const Dynamic_Cast_Conversions &t_conversions)
      {
        InItr orig(begin);
        while (begin != end)
        {
          try {
            if ((*begin)->filter(plist, t_conversions))
            {
              return (*(*begin))(plist, t_conversions);
            }
          } catch (const exception::bad_boxed_cast &) {
            //parameter failed to cast, try again
          } catch (const exception::arity_error &) {
            //invalid num params, try again
          } catch (const exception::guard_error &) {
            //guard failed to allow the function to execute,
            //try again
          }
          ++begin;
        }

        return detail::dispatch_with_conversions(orig, end, plist, t_conversions);
      }

    /**
     * Take a vector of functions and a vector of parameters. Attempt to execute
     * each function against the set of parameters, in order, until a matching
     * function is found or throw dispatch_error if no matching function is found
     */
    template<typename Funcs>
      Boxed_Value dispatch(const Funcs &funcs,
          const std::vector<Boxed_Value> &plist, const Dynamic_Cast_Conversions &t_conversions)
      {
        return dispatch::dispatch(funcs.begin(), funcs.end(), plist, t_conversions);
      }
  }
}


#endif
