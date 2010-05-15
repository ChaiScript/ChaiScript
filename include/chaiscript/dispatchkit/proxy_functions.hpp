// This file is distributed under the BSD License.
// See "license.txt" for details.
// Copyright 2009-2010, Jonathan Turner (jturner@minnow-lang.org)
// and Jason Turner (lefticus@gmail.com)
// http://www.chaiscript.com


#ifndef __proxy_functions_hpp__
#define __proxy_functions_hpp__


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
      virtual Boxed_Value operator()(const std::vector<Boxed_Value> &params) const = 0;

      std::vector<Type_Info> get_param_types() const { return m_types; }

      virtual bool operator==(const Proxy_Function_Base &) const = 0;
      virtual bool call_match(const std::vector<Boxed_Value> &vals) const = 0;

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
            const std::vector<Type_Info> &types = get_param_types();

            if (types.size() < 2)
            {
              return true;
            }

            const Type_Info &ti = types[1];


            if (ti.is_undef() || vals[0].get_type_info().is_undef()
                || ti.bare_equal(user_type<Boxed_Value>())
                || ti.bare_equal(user_type<Boxed_POD_Value>())
                || ti.bare_equal(vals[0].get_type_info()))
            {
              return true;
            } else {
              return false;
            }
          }
        } else {
          return false;
        }
      }

      virtual int get_arity() const = 0;

      virtual std::string annotation() const = 0;

    protected:
      Proxy_Function_Base(const std::vector<Type_Info> &t_types)
        : m_types(t_types)
      {
      }

      bool compare_types(const std::vector<Type_Info> &tis, const std::vector<Boxed_Value> &bvs) const
      {
        if (tis.size() - 1 != bvs.size())
        {
          return false;
        } else {
          const int size = bvs.size();
          for (int i = 0; i < size; ++i)
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

  typedef boost::shared_ptr<Proxy_Function_Base> Proxy_Function;
  typedef boost::shared_ptr<const Proxy_Function_Base> Const_Proxy_Function;

  /**
   * Exception thrown if a function's guard fails to execute
   */
  class guard_error : public std::runtime_error
  {
    public:
      guard_error() throw()
        : std::runtime_error("Guard evaluation failed")
      { }

      virtual ~guard_error() throw()
      { }
  };

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
          const std::string &t_description = "",
          const Proxy_Function &t_guard = Proxy_Function())
        : Proxy_Function_Base(build_param_type_list(t_arity)),
          m_f(t_f), m_arity(t_arity), m_description(t_description), m_guard(t_guard)
      {
      }

      virtual bool operator==(const Proxy_Function_Base &) const
      {
        return false;
      }

      virtual bool call_match(const std::vector<Boxed_Value> &vals) const
      {
          return (m_arity < 0 || vals.size() == size_t(m_arity))
            && test_guard(vals);
      }    

      virtual ~Dynamic_Proxy_Function() {}

      virtual Boxed_Value operator()(const std::vector<Boxed_Value> &params) const
      {
        if (m_arity < 0 || params.size() == size_t(m_arity))
        {

          if (test_guard(params))
          {
            return m_f(params);
          } else {
            throw guard_error();
          }

        } else {
          throw arity_error(params.size(), m_arity);
        } 
      }

      virtual int get_arity() const
      {
	return m_arity;
      }

      virtual std::string annotation() const
      {
        return m_description;
      }

    private:
      bool test_guard(const std::vector<Boxed_Value> &params) const
      {
        if (m_guard)
        {
          try {
            return boxed_cast<bool>((*m_guard)(params));
          } catch (const arity_error &) {
            return false;
          } catch (const bad_boxed_cast &) {
            return false;
          }
        } else {
          return true;
        }
      }

      static std::vector<Type_Info> build_param_type_list(int arity)
      {
        std::vector<Type_Info> types;

        types.push_back(detail::Get_Type_Info<Boxed_Value>::get());

        if (arity >= 0)
        {
          for (int i = 0; i < arity; ++i)
          {
            types.push_back(detail::Get_Type_Info<Boxed_Value>::get());
          }
        } else {
          types.push_back(detail::Get_Type_Info<std::vector<Boxed_Value> >::get());
        }

        return types;
      }

      boost::function<Boxed_Value (const std::vector<Boxed_Value> &)> m_f;
      int m_arity;
      std::string m_description;
      Proxy_Function m_guard;
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
        : Proxy_Function_Base(std::vector<Type_Info>()),
          m_f(t_f), m_args(t_args), m_arity(m_f->get_arity()<0?-1:(m_f->get_arity() - m_args.size()))
      {
      }

      virtual bool operator==(const Proxy_Function_Base &) const
      {
        return false;
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

      std::vector<Boxed_Value> build_param_list(const std::vector<Boxed_Value> &params) const
      {
        typedef std::vector<Boxed_Value>::const_iterator pitr;

        pitr parg = params.begin();
        pitr barg = m_args.begin();

        std::vector<Boxed_Value> args;

        while (true)
        {
          while (barg != m_args.end() 
                 && !(barg->get_type_info() == detail::Get_Type_Info<Placeholder_Object>::get()))
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
              && barg->get_type_info() == detail::Get_Type_Info<Placeholder_Object>::get())
          {
            ++barg;
          } 

          if (parg == params.end() && barg == m_args.end())
          {
            break;
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
        return "";
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
        : Proxy_Function_Base(build_param_type_list(static_cast<Func *>(0))),
          m_f(f), m_dummy_func(0)
      {
      }

      virtual ~Proxy_Function_Impl() {}

      virtual bool operator==(const Proxy_Function_Base &t_func) const
      {
        try {
          dynamic_cast<const Proxy_Function_Impl<Func> &>(t_func);
          return true; 
        } catch (const std::bad_cast &) {
          return false;
        }
      }
 
      virtual Boxed_Value operator()(const std::vector<Boxed_Value> &params) const
      {
        return Do_Call<typename boost::function<Func>::result_type>::go(m_f, params);
      }

      virtual int get_arity() const
      {
        return m_types.size() - 1;
      }


      virtual bool call_match(const std::vector<Boxed_Value> &vals) const
      {
        if (int(vals.size()) != get_arity()) 
        {
          return false;
        }

        return compare_types(m_types, vals) || compare_types_cast(m_dummy_func, vals);
      }

      virtual std::string annotation() const
      {
        return "";
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
        try {
          const Attribute_Access<T, Class> &aa 
            = dynamic_cast<const Attribute_Access<T, Class> &>(t_func);
          return m_attr == aa.m_attr;
        } catch (const std::bad_cast &) {
          return false;
        }
      }
 
      virtual Boxed_Value operator()(const std::vector<Boxed_Value> &params) const
      {
        if (params.size() == 1)
        {
          const Boxed_Value &bv = params[0];
          if (bv.is_const())
          {
            const Class *o = boxed_cast<const Class *>(bv);
            return Handle_Return<typename boost::add_reference<T>::type>::handle(o->*m_attr);
          } else {
            Class *o = boxed_cast<Class *>(bv);
            return Handle_Return<typename boost::add_reference<T>::type>::handle(o->*m_attr);
          }
        } else {
          throw arity_error(params.size(), 1);
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

  /**
   * Exception thrown in the case that a multi method dispatch fails
   * because no matching function was found
   * at runtime due to either an arity_error, a guard_error or a bad_boxed_cast
   * exception
   */
  struct dispatch_error : std::runtime_error
  {
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
        if (begin->second->filter(plist))
        {
          return (*begin->second)(plist);
        }
      } catch (const bad_boxed_cast &) {
        //parameter failed to cast, try again
      } catch (const arity_error &) {
        //invalid num params, try again
      } catch (const guard_error &) {
        //guard failed to allow the function to execute,
        //try again
      }
      ++begin;
    }

    throw dispatch_error(plist.empty()?false:plist[0].is_const());
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
    return dispatch(funcs.begin(), funcs.end(), plist);
  }
}


#endif
