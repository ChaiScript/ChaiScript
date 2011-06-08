// This file is distributed under the BSD License.
// See "license.txt" for details.
// Copyright 2009-2011, Jonathan Turner (jonathan@emptycrate.com)
// and Jason Turner (jason@emptycrate.com)
// http://www.chaiscript.com


#ifndef CHAISCRIPT_PROXY_FUNCTIONS_HPP_
#define CHAISCRIPT_PROXY_FUNCTIONS_HPP_


#include "boxed_value.hpp"
#include "type_info.hpp"
#include <string>
#include <boost/function.hpp>
#include <boost/type_traits/add_reference.hpp>
#include <stdexcept>
#include <vector>
#include "proxy_functions_detail.hpp"

namespace chaiscript
{
  class Boxed_Number;
  struct AST_Node;

  typedef boost::shared_ptr<struct AST_Node> AST_NodePtr;


  namespace dispatch
  {
    /**
     * Helper for building a list of parameters for calling a Proxy_Function
     * it does automatic conversion to Boxed_Value types via operator<<
     *
     * example usage:
     * Boxed_Value retval = dispatch(dispatchengine.get_function("+"), 
     *                               chaiscript::Param_List_Builder() << 5 << 6);
     */
    struct Param_List_Builder
    {
      Param_List_Builder &operator<<(const Boxed_Value &so)
      {
        objects.push_back(so);
        return *this;
      }

      template<typename T>
        Param_List_Builder &operator<<(T t)
        {
          objects.push_back(Boxed_Value(t));
          return *this;
        }

      operator const std::vector<Boxed_Value> &() const
      {
        return objects;
      }

      std::vector<Boxed_Value> objects;
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
        Boxed_Value operator()(const std::vector<Boxed_Value> &params) const
        {
          Boxed_Value bv = do_call(params);
          bv.add_dependencies(params.begin(), params.end());
          return bv;
        }

        /// Returns a vector containing all of the types of the parameters the function returns/takes
        /// if the function is variadic or takes no arguments (arity of 0 or -1), the returned
        /// value containes exactly 1 Type_Info object: the return type
        /// \returns the types of all parameters. 
        std::vector<Type_Info> get_param_types() const { return m_types; }

        virtual bool operator==(const Proxy_Function_Base &) const = 0;
        virtual bool call_match(const std::vector<Boxed_Value> &vals) const = 0;

        virtual std::vector<boost::shared_ptr<const Proxy_Function_Base> > get_contained_functions() const
        {
          return std::vector<boost::shared_ptr<const Proxy_Function_Base> >();
        }


        //! Return true if the function is a possible match
        //! to the passed in values
        bool filter(const std::vector<Boxed_Value> &vals) const
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
              return compare_first_type(vals[0]);
            }
          } else {
            return false;
          }
        }

        /// \returns the number of arguments the function takes or -1 if it is variadic
        virtual int get_arity() const = 0;

        virtual std::string annotation() const = 0;

      protected:
        virtual Boxed_Value do_call(const std::vector<Boxed_Value> &params) const = 0;

        Proxy_Function_Base(const std::vector<Type_Info> &t_types)
          : m_types(t_types)
        {
        }

        virtual bool compare_first_type(const Boxed_Value &bv) const
        {
          const std::vector<Type_Info> &types = get_param_types();

          if (types.size() < 2)
          {
            return true;
          }
          const Type_Info &ti = types[1];
          if (ti.is_undef() 
              || ti.bare_equal(user_type<Boxed_Value>())
              || (!bv.get_type_info().is_undef()
                && (ti.bare_equal(user_type<Boxed_Number>())
                  || ti.bare_equal(bv.get_type_info())
                  || chaiscript::detail::dynamic_cast_converts(ti, bv.get_type_info()) 
                  || bv.get_type_info().bare_equal(user_type<boost::shared_ptr<const Proxy_Function_Base> >())
                  )
                )
             )
          {
            return true;
          } else {
            return false;
          }
        }

        bool compare_types(const std::vector<Type_Info> &tis, const std::vector<Boxed_Value> &bvs) const
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
    };
  }

  /// \brief Common typedef used for passing of any registered function in ChaiScript
  typedef boost::shared_ptr<dispatch::Proxy_Function_Base> Proxy_Function;

  /// \brief Const version of Proxy_Function chaiscript. Points to a const Proxy_Function. This is how most registered functions
  ///        are handled internally.
  typedef boost::shared_ptr<const dispatch::Proxy_Function_Base> Const_Proxy_Function;

  namespace exception
  {
    /// \brief  Exception thrown if a function's guard fails
    class guard_error : public std::runtime_error
    {
      public:
        guard_error() throw()
          : std::runtime_error("Guard evaluation failed")
        { }

        virtual ~guard_error() throw()
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
            const boost::function<Boxed_Value (const std::vector<Boxed_Value> &)> &t_f, 
            int t_arity=-1,
            const AST_NodePtr &t_parsenode = AST_NodePtr(),
            const std::string &t_description = "",
            const Proxy_Function &t_guard = Proxy_Function())
          : Proxy_Function_Base(build_param_type_list(t_arity)),
            m_f(t_f), m_arity(t_arity), m_description(t_description), m_guard(t_guard), m_parsenode(t_parsenode)
        {
        }

        virtual bool operator==(const Proxy_Function_Base &rhs) const
        {
          return this == &rhs;
        }

        virtual bool call_match(const std::vector<Boxed_Value> &vals) const
        {
          return (m_arity < 0 || vals.size() == size_t(m_arity))
            && test_guard(vals);
        }    

        virtual ~Dynamic_Proxy_Function() {}


        virtual int get_arity() const
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

        virtual std::string annotation() const
        {
          return m_description;
        }

      protected:
        virtual Boxed_Value do_call(const std::vector<Boxed_Value> &params) const
        {
          if (m_arity < 0 || params.size() == size_t(m_arity))
          {

            if (test_guard(params))
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
        bool test_guard(const std::vector<Boxed_Value> &params) const
        {
          if (m_guard)
          {
            try {
              return boxed_cast<bool>((*m_guard)(params));
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

          if (arity >= 0)
          {
            for (int i = 0; i < arity; ++i)
            {
              types.push_back(chaiscript::detail::Get_Type_Info<Boxed_Value>::get());
            }
          }

          return types;
        }

        boost::function<Boxed_Value (const std::vector<Boxed_Value> &)> m_f;
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

        virtual bool operator==(const Proxy_Function_Base &t_f) const
        {
          return &t_f == this;
        }

        virtual ~Bound_Function() {}

        virtual bool call_match(const std::vector<Boxed_Value> &vals) const
        {
          return m_f->call_match(build_param_list(vals));
        }

        virtual Boxed_Value operator()(const std::vector<Boxed_Value> &params) const
        {
          return (*m_f)(build_param_list(params));
        }

        virtual std::vector<Const_Proxy_Function> get_contained_functions() const
        {
          std::vector<Const_Proxy_Function> fs;
          fs.push_back(m_f);
          return fs;
        }


        std::vector<Boxed_Value> build_param_list(const std::vector<Boxed_Value> &params) const
        {
          typedef std::vector<Boxed_Value>::const_iterator pitr;

          pitr parg = params.begin();
          pitr barg = m_args.begin();

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

        virtual int get_arity() const
        {
          return m_arity;
        }

        virtual std::string annotation() const
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

        virtual Boxed_Value do_call(const std::vector<Boxed_Value> &params) const
        {
          return (*m_f)(build_param_list(params));
        }

      private:
        Const_Proxy_Function m_f;
        std::vector<Boxed_Value> m_args;
        int m_arity;
    };

    /**
     * The standard typesafe function call implementation of Proxy_Function
     * It takes a boost::function<> object and performs runtime 
     * type checking of Boxed_Value parameters, in a type safe manner
     */
    template<typename Func>
      class Proxy_Function_Impl : public Proxy_Function_Base
    {
      public:
        Proxy_Function_Impl(const boost::function<Func> &f)
          : Proxy_Function_Base(detail::build_param_type_list(static_cast<Func *>(0))),
          m_f(f), m_dummy_func(0)
      {
      }

        virtual ~Proxy_Function_Impl() {}

        virtual bool operator==(const Proxy_Function_Base &t_func) const
        {
          const Proxy_Function_Impl *pimpl = dynamic_cast<const Proxy_Function_Impl<Func> *>(&t_func);
          return pimpl != 0;
        }


        virtual int get_arity() const
        {
          return static_cast<int>(m_types.size()) - 1;
        }


        virtual bool call_match(const std::vector<Boxed_Value> &vals) const
        {
          if (int(vals.size()) != get_arity()) 
          {
            return false;
          }

          return compare_types(m_types, vals) || detail::compare_types_cast(m_dummy_func, vals);
        }

        virtual std::string annotation() const
        {
          return "";
        }

        boost::function<Func> internal_function() const
        {
          return m_f;
        }

      protected:
        virtual Boxed_Value do_call(const std::vector<Boxed_Value> &params) const
        {
          return detail::Do_Call<typename boost::function<Func>::result_type>::go(m_f, params);
        }

      private:
        boost::function<Func> m_f;
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

        virtual bool operator==(const Proxy_Function_Base &t_func) const
        {
          const Attribute_Access<T, Class> * aa 
            = dynamic_cast<const Attribute_Access<T, Class> *>(&t_func);
          if (aa) {
            return m_attr == aa->m_attr;
          } else {
            return false;
          }
        }


        virtual int get_arity() const
        {
          return 1;
        }

        virtual bool call_match(const std::vector<Boxed_Value> &vals) const
        {
          if (vals.size() != 1)
          {
            return false;
          }

          return vals[0].get_type_info().bare_equal(user_type<Class>());
        }

        virtual std::string annotation() const
        {
          return "";
        }

      protected:
        virtual Boxed_Value do_call(const std::vector<Boxed_Value> &params) const
        {
          if (params.size() == 1)
          {
            const Boxed_Value &bv = params[0];
            if (bv.is_const())
            {
              const Class *o = boxed_cast<const Class *>(bv);
              return detail::Handle_Return<typename boost::add_reference<T>::type>::handle(o->*m_attr);
            } else {
              Class *o = boxed_cast<Class *>(bv);
              return detail::Handle_Return<typename boost::add_reference<T>::type>::handle(o->*m_attr);
            }
          } else {
            throw exception::arity_error(static_cast<int>(params.size()), 1);
          }       
        }

      private:
        static std::vector<Type_Info> param_types()
        {
          std::vector<Type_Info> v;
          v.push_back(user_type<T>());
          v.push_back(user_type<Class>());
          return v;
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
        dispatch_error() throw()
          : std::runtime_error("No matching function to dispatch to")
        {
        }

        dispatch_error(bool is_const) throw()
          : std::runtime_error(std::string("No matching function to dispatch to") + (is_const?", parameter is const":""))
          {
          }

        virtual ~dispatch_error() throw() {}
    };
  } 

  namespace dispatch
  {

    /**
     * Take a vector of functions and a vector of parameters. Attempt to execute
     * each function against the set of parameters, in order, until a matching
     * function is found or throw dispatch_error if no matching function is found
     */
    template<typename InItr>
      Boxed_Value dispatch(InItr begin, InItr end,
          const std::vector<Boxed_Value> &plist)
      {
        while (begin != end)
        {
          try {
            if ((*begin)->filter(plist))
            {
              return (*(*begin))(plist);
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

        throw exception::dispatch_error(plist.empty()?false:plist[0].is_const());
      }

    /**
     * Take a vector of functions and a vector of parameters. Attempt to execute
     * each function against the set of parameters, in order, until a matching
     * function is found or throw dispatch_error if no matching function is found
     */
    template<typename Funcs>
      Boxed_Value dispatch(const Funcs &funcs,
          const std::vector<Boxed_Value> &plist)
      {
        return dispatch::dispatch(funcs.begin(), funcs.end(), plist);
      }
  }
}


#endif
