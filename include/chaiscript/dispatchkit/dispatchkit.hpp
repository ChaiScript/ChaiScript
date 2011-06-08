// This file is distributed under the BSD License.
// See "license.txt" for details.
// Copyright 2009-2011, Jonathan Turner (jonathan@emptycrate.com)
// and Jason Turner (jason@emptycrate.com)
// http://www.chaiscript.com

#ifndef CHAISCRIPT_DISPATCHKIT_HPP_
#define CHAISCRIPT_DISPATCHKIT_HPP_

#include <typeinfo>
#include <string>
#include <map>
#include <set>
#include <boost/shared_ptr.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/tuple/tuple.hpp>
#include <stdexcept>
#include <vector>
#include <iostream>
#include <deque>

#include "boxed_value.hpp"
#include "type_info.hpp"
#include "proxy_functions.hpp"
#include "proxy_constructors.hpp"
#include "dynamic_object.hpp"
#include "../chaiscript_threading.hpp"


/// \namespace chaiscript::dispatch
/// \brief Classes and functions specific to the runtime dispatch side of ChaiScript. Some items may be of use to the end user.

namespace chaiscript
{
  /// \brief Holds a collection of ChaiScript settings which can be applied to the ChaiScript runtime.
  ///        Used to implement loadable module support.
  class Module
  {
    public:
      Module &add(const Type_Info &ti, const std::string &name)
      {
        m_typeinfos.push_back(std::make_pair(ti, name));
        return *this;
      }

      Module &add(const Dynamic_Cast_Conversion &d)
      {
        m_conversions.push_back(d);
        return *this;
      }

      Module &add(const Proxy_Function &f, const std::string &name)
      {
        m_funcs.push_back(std::make_pair(f, name));
        return *this;
      }


      //Add a bit of chaiscript to eval during module implementation
      Module &eval(const std::string &str)
      {
        m_evals.push_back(str);
        return *this;
      }

      Module &add(const boost::shared_ptr<Module> &m)
      {
        m->apply(*this, *this);
        return *m;
      }

      template<typename Eval, typename Engine>
        void apply(Eval &t_eval, Engine &t_engine) const
        {
          apply(m_typeinfos.begin(), m_typeinfos.end(), t_engine);
          apply(m_funcs.begin(), m_funcs.end(), t_engine);
          apply_eval(m_evals.begin(), m_evals.end(), t_eval);
          apply_single(m_conversions.begin(), m_conversions.end(), t_engine);
        }

    private:
      std::vector<std::pair<Type_Info, std::string> > m_typeinfos;
      std::vector<std::pair<Proxy_Function, std::string> > m_funcs;
      std::vector<std::string> m_evals;
      std::vector<Dynamic_Cast_Conversion> m_conversions;

      template<typename T, typename InItr>
        void apply(InItr begin, InItr end, T &t) const
        {
          while (begin != end)
          {
            t.add(begin->first, begin->second);
            ++begin;
          }
        }

      template<typename T, typename InItr>
        void apply_single(InItr begin, InItr end, T &t) const
        {
          while (begin != end)
          {
            t.add(*begin);
            ++begin;
          }
        }

      template<typename T, typename InItr>
        void apply_eval(InItr begin, InItr end, T &t) const
        {
          while (begin != end)
          {
            t.eval(*begin);
            ++begin;
          }
        }
  };

  /// Convenience typedef for Module objects to be added to the ChaiScript runtime
  typedef boost::shared_ptr<Module> ModulePtr;

  namespace detail
  {
    /**
     * A Proxy_Function implementation that is able to take
     * a vector of Proxy_Functions and perform a dispatch on them. It is 
     * used specifically in the case of dealing with Function object variables
     */
    class Dispatch_Function : public dispatch::Proxy_Function_Base
    {
      public:
        Dispatch_Function(const std::vector<Proxy_Function> &t_funcs)
          : Proxy_Function_Base(build_type_infos(t_funcs)),
          m_funcs(t_funcs)
      {
      }

        virtual bool operator==(const dispatch::Proxy_Function_Base &rhs) const
        {
          try {
            const Dispatch_Function &dispatchfun = dynamic_cast<const Dispatch_Function &>(rhs);
            return m_funcs == dispatchfun.m_funcs;
          } catch (const std::bad_cast &) {
            return false;
          }
        }

        virtual ~Dispatch_Function() {}

        virtual std::vector<Const_Proxy_Function> get_contained_functions() const
        {
          return std::vector<Const_Proxy_Function>(m_funcs.begin(), m_funcs.end());
        }


        virtual int get_arity() const
        {
          typedef std::vector<Proxy_Function> function_vec;

          function_vec::const_iterator begin = m_funcs.begin();
          const function_vec::const_iterator end = m_funcs.end();

          if (begin != end)
          {
            int arity = (*begin)->get_arity();

            ++begin;

            while (begin != end)
            {
              if (arity != (*begin)->get_arity())
              {
                // The arities in the list do not match, so it's unspecified
                return -1;
              }

              ++begin;
            }

            return arity;
          }

          return -1; // unknown arity
        }

        virtual bool call_match(const std::vector<Boxed_Value> &vals) const
        {
          typedef std::vector<Proxy_Function> function_vec;

          function_vec::const_iterator begin = m_funcs.begin();
          function_vec::const_iterator end = m_funcs.end();

          while (begin != end)
          {
            if ((*begin)->call_match(vals))
            {
              return true;
            } else {
              ++begin;
            }
          }

          return false;
        }

        virtual std::string annotation() const
        {
          return "Multiple method dispatch function wrapper.";
        }

      protected:
        virtual Boxed_Value do_call(const std::vector<Boxed_Value> &params) const
        {
          return dispatch::dispatch(m_funcs.begin(), m_funcs.end(), params);
        }

      private:
        std::vector<Proxy_Function> m_funcs;

        static std::vector<Type_Info> build_type_infos(const std::vector<Proxy_Function> &t_funcs)
        {
          typedef std::vector<Proxy_Function> function_vec;

          function_vec::const_iterator begin = t_funcs.begin();
          const function_vec::const_iterator end = t_funcs.end();

          if (begin != end)
          {
            std::vector<Type_Info> type_infos = (*begin)->get_param_types();

            ++begin;

            bool sizemismatch = false;

            while (begin != end)
            {
              std::vector<Type_Info> param_types = (*begin)->get_param_types();

              if (param_types.size() != type_infos.size())
              {
                sizemismatch = true;
              }

              for (size_t i = 0; i < type_infos.size() && i < param_types.size(); ++i)
              {
                if (!(type_infos[i] == param_types[i]))
                {
                  type_infos[i] = detail::Get_Type_Info<Boxed_Value>::get();
                }
              }

              ++begin;
            }

            assert(type_infos.size() > 0 && " type_info vector size is < 0, this is only possible if something else is broken");

            if (sizemismatch)
            {
              type_infos.resize(1);
            }

            return type_infos;
          }

          return std::vector<Type_Info>();
        }
    };  
  }

  namespace exception
  {
    /**
     * Exception thrown in the case that a multi method dispatch fails
     * because no matching function was found
     * at runtime due to either an arity_error, a guard_error or a bad_boxed_cast
     * exception
     */
    class reserved_word_error : public std::runtime_error
    {
      public:
        reserved_word_error(const std::string &t_word) throw()
          : std::runtime_error("Reserved word not allowed in object name: " + t_word), m_word(t_word)
        {
        }

        virtual ~reserved_word_error() throw() {}

        std::string word() const
        {
          return m_word;
        }

      private:
        std::string m_word;

    };

    /**
     * Exception thrown in the case that a non-const object was added as a shared object
     */
    class global_non_const : public std::runtime_error
    {
      public:
        global_non_const() throw()
          : std::runtime_error("a global object must be const")
        {
        }

        virtual ~global_non_const() throw() {}
    };
  }

  namespace detail
  {
    /**
     * Main class for the dispatchkit. Handles management
     * of the object stack, functions and registered types.
     */
    class Dispatch_Engine
    {
      public:
        typedef std::map<std::string, chaiscript::Type_Info> Type_Name_Map;
        typedef std::map<std::string, Boxed_Value> Scope;
        typedef std::deque<Scope> StackData;
        typedef boost::shared_ptr<StackData> Stack;

        struct State
        {
          std::map<std::string, std::vector<Proxy_Function> > m_functions;
          std::map<std::string, Boxed_Value> m_global_objects;
          Type_Name_Map m_types;
          std::set<std::string> m_reserved_words;
        };

        Dispatch_Engine()
          : m_place_holder(boost::shared_ptr<dispatch::Placeholder_Object>(new dispatch::Placeholder_Object()))
        {
        }

        ~Dispatch_Engine()
        {
          detail::Dynamic_Conversions::get().cleanup(m_conversions.begin(), m_conversions.end());
        }

        /**
         * Add a new conversion for upcasting to a base class
         */
        void add(const Dynamic_Cast_Conversion &d)
        {
          m_conversions.push_back(d);
          return detail::Dynamic_Conversions::get().add_conversion(d);
        }

        /**
         * Add a new named Proxy_Function to the system
         */
        bool add(const Proxy_Function &f, const std::string &name)
        {
          validate_object_name(name);
          return add_function(f, name);
        }

        /**
         * Set the value of an object, by name. If the object
         * is not available in the current scope it is created
         */
        void add(const Boxed_Value &obj, const std::string &name)
        {
          validate_object_name(name);
          StackData &stack = get_stack_data();

          for (int i = static_cast<int>(stack.size())-1; i >= 0; --i)
          {
            std::map<std::string, Boxed_Value>::const_iterator itr = stack[i].find(name);
            if (itr != stack[i].end())
            {
              stack[i][name] = obj;
              return;
            }
          }

          add_object(name, obj);
        }

        /**
         * Adds a named object to the current scope
         */
        void add_object(const std::string &name, const Boxed_Value &obj)
        {
          StackData &stack = get_stack_data();
          validate_object_name(name);
          stack.back()[name] = obj;
        }

        /**
         * Adds a new global shared object, between all the threads
         */
        void add_global_const(const Boxed_Value &obj, const std::string &name)
        {
          validate_object_name(name);
          if (!obj.is_const())
          {
            throw exception::global_non_const();
          }

          chaiscript::detail::threading::unique_lock<chaiscript::detail::threading::shared_mutex> l(m_global_object_mutex);

          m_state.m_global_objects[name] = obj;
        }

        /**
         * Adds a new scope to the stack
         */
        void new_scope()
        {
          StackData &stack = get_stack_data();
          stack.push_back(Scope());
        }

        /**
         * Pops the current scope from the stack
         */
        void pop_scope()
        {
          StackData &stack = get_stack_data();
          if (stack.size() > 1)
          {
            stack.pop_back();
          } else {
            throw std::range_error("Unable to pop global stack");
          }
        }

        /**
         * Swaps out the stack with a new stack
         * \returns the old stack
         * \param[in] s The new stack
         */
        Stack set_stack(const Stack &s)
        {
          Stack old = m_stack_holder->stack;
          m_stack_holder->stack = s;
          return old;
        }

        Stack new_stack() const
        {
          Stack s(new Stack::element_type());
          s->push_back(Scope());
          return s;
        }

        Stack get_stack() const
        {
          return m_stack_holder->stack;
        }

        /**
         * Searches the current stack for an object of the given name
         * includes a special overload for the _ place holder object to
         * ensure that it is always in scope.
         */
        Boxed_Value get_object(const std::string &name) const
        {
          // Is it a placeholder object?
          if (name == "_")
          {
            return m_place_holder;
          }

          StackData &stack = get_stack_data();

          // Is it in the stack?
          for (int i = static_cast<int>(stack.size())-1; i >= 0; --i)
          {
            std::map<std::string, Boxed_Value>::const_iterator stackitr = stack[i].find(name);
            if (stackitr != stack[i].end())
            {
              return stackitr->second;
            }
          }

          // Is the value we are looking for a global?
          {
            chaiscript::detail::threading::shared_lock<chaiscript::detail::threading::shared_mutex> l(m_global_object_mutex);

            std::map<std::string, Boxed_Value>::const_iterator itr = m_state.m_global_objects.find(name);
            if (itr != m_state.m_global_objects.end())
            {
              return itr->second;
            }
          }

          // If all that failed, then check to see if it's a function
          std::vector<Proxy_Function> funcs = get_function(name);

          if (funcs.empty())
          {
            throw std::range_error("Object not known: " + name);
          } else {
            if (funcs.size() == 1)
            {
              // Return the first item if there is only one,
              // no reason to take the cast of the extra level of dispatch
              return const_var(*funcs.begin());
            } else {
              return Boxed_Value(Const_Proxy_Function(new Dispatch_Function(funcs)));
            }
          }
        }

        /**
         * Registers a new named type
         */
        void add(const Type_Info &ti, const std::string &name)
        {
          add_global_const(const_var(ti), name + "_type");

          chaiscript::detail::threading::unique_lock<chaiscript::detail::threading::shared_mutex> l(m_mutex);

          m_state.m_types.insert(std::make_pair(name, ti));
        }

        /**
         * Returns the type info for a named type
         */
        Type_Info get_type(const std::string &name) const
        {
          chaiscript::detail::threading::shared_lock<chaiscript::detail::threading::shared_mutex> l(m_mutex);

          Type_Name_Map::const_iterator itr = m_state.m_types.find(name);

          if (itr != m_state.m_types.end())
          {
            return itr->second;
          }

          throw std::range_error("Type Not Known");
        }

        /**
         * Returns the registered name of a known type_info object
         * compares the "bare_type_info" for the broadest possible
         * match
         */
        std::string get_type_name(const Type_Info &ti) const
        {
          chaiscript::detail::threading::shared_lock<chaiscript::detail::threading::shared_mutex> l(m_mutex);

          for (Type_Name_Map::const_iterator itr = m_state.m_types.begin();
              itr != m_state.m_types.end();
              ++itr)
          {
            if (itr->second.bare_equal(ti))
            {
              return itr->first;
            }
          }

          return ti.bare_name();
        }

        /**
         * Return all registered types
         */
        std::vector<std::pair<std::string, Type_Info> > get_types() const
        {
          chaiscript::detail::threading::shared_lock<chaiscript::detail::threading::shared_mutex> l(m_mutex);

          return std::vector<std::pair<std::string, Type_Info> >(m_state.m_types.begin(), m_state.m_types.end());
        }

        /**
         * Return a function by name
         */
        std::vector< Proxy_Function >
          get_function(const std::string &t_name) const
          {
            chaiscript::detail::threading::shared_lock<chaiscript::detail::threading::shared_mutex> l(m_mutex);

            const std::map<std::string, std::vector<Proxy_Function> > &funs = get_functions_int();

            std::map<std::string, std::vector<Proxy_Function> >::const_iterator itr 
              = funs.find(t_name);

            if (itr != funs.end())
            {
              return itr->second;
            } else {
              return std::vector<Proxy_Function>();
            }

          }

        /**
         * Return true if a function exists
         */
        bool function_exists(const std::string &name) const
        {
          chaiscript::detail::threading::shared_lock<chaiscript::detail::threading::shared_mutex> l(m_mutex);

          const std::map<std::string, std::vector<Proxy_Function> > &functions = get_functions_int();
          return functions.find(name) != functions.end();
        }

        /**
         * Get a vector of all registered functions
         */
        std::vector<std::pair<std::string, Proxy_Function > > get_functions() const
        {
          chaiscript::detail::threading::shared_lock<chaiscript::detail::threading::shared_mutex> l(m_mutex);

          std::vector<std::pair<std::string, Proxy_Function> > rets;

          const std::map<std::string, std::vector<Proxy_Function> > &functions = get_functions_int();

          for (std::map<std::string, std::vector<Proxy_Function> >::const_iterator itr = functions.begin();
              itr != functions.end();
              ++itr)
          {
            for (std::vector<Proxy_Function>::const_iterator itr2 = itr->second.begin();
                itr2 != itr->second.end();
                ++itr2)
            {
              rets.push_back(std::make_pair(itr->first, *itr2));
            }
          }

          return rets;
        }

        void add_reserved_word(const std::string &name)
        {
          chaiscript::detail::threading::unique_lock<chaiscript::detail::threading::shared_mutex> l(m_mutex);

          m_state.m_reserved_words.insert(name);
        }

        Boxed_Value call_function(const std::string &t_name, const std::vector<Boxed_Value> &params) const
        {
          std::vector<Proxy_Function> functions = get_function(t_name);

          return dispatch::dispatch(functions.begin(), functions.end(), params);
        }

        Boxed_Value call_function(const std::string &t_name) const
        {
          return call_function(t_name, std::vector<Boxed_Value>());
        }

        Boxed_Value call_function(const std::string &t_name, const Boxed_Value &p1) const
        {
          std::vector<Boxed_Value> params;
          params.push_back(p1);
          return call_function(t_name, params);
        }

        Boxed_Value call_function(const std::string &t_name, const Boxed_Value &p1, const Boxed_Value &p2) const
        {
          std::vector<Boxed_Value> params;
          params.push_back(p1);
          params.push_back(p2);
          return call_function(t_name, params);
        }

        /**
         * Dump object info to stdout
         */
        void dump_object(Boxed_Value o) const
        {
          Type_Info ti = o.get_type_info();
          std::cout << (ti.is_const()?"const ":"") << get_type_name(ti) << std::endl;
        }

        /**
         * Dump type info to stdout
         */
        void dump_type(const Type_Info &type) const
        {
          std::cout << (type.is_const()?"const ":"") << get_type_name(type);
        }

        /**
         * Dump function to stdout
         */
        void dump_function(const std::pair<const std::string, Proxy_Function > &f) const
        {
          std::vector<Type_Info> params = f.second->get_param_types();
          std::string annotation = f.second->annotation();

          if (annotation.size() > 0) {
            std::cout << annotation;
          }
          dump_type(params.front());
          std::cout << " " << f.first << "(";

          for (std::vector<Type_Info>::const_iterator itr = params.begin() + 1;
              itr != params.end();
              )
          {
            dump_type(*itr);
            ++itr;

            if (itr != params.end())
            {
              std::cout << ", ";
            }
          }

          std::cout << ") " << std::endl;
        }

        /**
         * Dump all system info to stdout
         */
        void dump_system() const
        {
          std::cout << "Registered Types: " << std::endl;
          std::vector<std::pair<std::string, Type_Info> > types = get_types();
          for (std::vector<std::pair<std::string, Type_Info> >::const_iterator itr = types.begin();
              itr != types.end();
              ++itr)
          {
            std::cout << itr->first << ": ";
            std::cout << itr->second.bare_name();
            std::cout << std::endl;
          }

          std::cout << std::endl;  
          std::vector<std::pair<std::string, Proxy_Function > > funcs = get_functions();

          std::cout << "Functions: " << std::endl;
          for (std::vector<std::pair<std::string, Proxy_Function > >::const_iterator itr = funcs.begin();
              itr != funcs.end();
              ++itr)
          {
            dump_function(*itr);
          }
          std::cout << std::endl;
        }

        /**
         * return true if the Boxed_Value matches the registered type by name
         */
        bool is_type(const Boxed_Value &r, const std::string &user_typename) const
        {
          try {
            if (get_type(user_typename).bare_equal(r.get_type_info()))
            {
              return true;
            }
          } catch (const std::range_error &) {
          }

          try {
            const dispatch::Dynamic_Object &d = boxed_cast<const dispatch::Dynamic_Object &>(r);
            return d.get_type_name() == user_typename;
          } catch (const std::bad_cast &) {
          }

          return false;
        }

        std::string type_name(Boxed_Value obj) const
        {
          return get_type_name(obj.get_type_info());
        }

        State get_state()
        {
          chaiscript::detail::threading::shared_lock<chaiscript::detail::threading::shared_mutex> l(m_mutex);
          chaiscript::detail::threading::shared_lock<chaiscript::detail::threading::shared_mutex> l2(m_global_object_mutex);

          return m_state;
        }

        void set_state(const State &t_state)
        {
          chaiscript::detail::threading::unique_lock<chaiscript::detail::threading::shared_mutex> l(m_mutex);
          chaiscript::detail::threading::unique_lock<chaiscript::detail::threading::shared_mutex> l2(m_global_object_mutex);

          m_state = t_state;
        }


      private:
        /**
         * Returns the current stack
         * make const/non const versions
         */
        StackData &get_stack_data() const
        {
          return *(m_stack_holder->stack);
        }

        const std::map<std::string, std::vector<Proxy_Function> > &get_functions_int() const
        {
          return m_state.m_functions;
        }

        std::map<std::string, std::vector<Proxy_Function> > &get_functions_int() 
        {
          return m_state.m_functions;
        }

        static bool function_less_than(const Proxy_Function &lhs, const Proxy_Function &rhs)
        {
          const std::vector<Type_Info> lhsparamtypes = lhs->get_param_types();
          const std::vector<Type_Info> rhsparamtypes = rhs->get_param_types();

          const size_t lhssize = lhsparamtypes.size();
          const size_t rhssize = rhsparamtypes.size();

          const Type_Info boxed_type = user_type<Boxed_Value>();
          const Type_Info boxed_pod_type = user_type<Boxed_Number>();

          boost::shared_ptr<const dispatch::Dynamic_Proxy_Function> dynamic_lhs(boost::dynamic_pointer_cast<const dispatch::Dynamic_Proxy_Function>(lhs));
          boost::shared_ptr<const dispatch::Dynamic_Proxy_Function> dynamic_rhs(boost::dynamic_pointer_cast<const dispatch::Dynamic_Proxy_Function>(rhs));

          if (dynamic_lhs && dynamic_rhs)
          {
            if (dynamic_lhs->get_guard())
            {
              if (dynamic_rhs->get_guard())
              {
                return false;
              } else {
                return true;
              }
            } else {
              return false;
            }
          }

          if (dynamic_lhs && !dynamic_rhs)
          {
            return false;
          }

          if (!dynamic_lhs && dynamic_rhs)
          {
            return true;
          }



          for (size_t i = 1; i < lhssize && i < rhssize; ++i)
          {
            const Type_Info lt = lhsparamtypes[i];
            const Type_Info rt = rhsparamtypes[i];

            if (lt.bare_equal(rt) && lt.is_const() == rt.is_const())
            {
              continue; // The first two types are essentially the same, next iteration
            }

            // const is after non-const for the same type
            if (lt.bare_equal(rt) && lt.is_const() && !rt.is_const())
            {
              return false;
            }

            if (lt.bare_equal(rt) && !lt.is_const())
            {
              return true;
            }

            // boxed_values are sorted last
            if (lt.bare_equal(boxed_type))
            {
              return false;
            }

            if (rt.bare_equal(boxed_type))
            {
              if (lt.bare_equal(boxed_pod_type))
              {
                return true;
              }
              return true;
            }

            if (lt.bare_equal(boxed_pod_type))
            {
              return false;
            }

            if (rt.bare_equal(boxed_pod_type))
            {
              return true;
            }

            // otherwise, we want to sort by typeid
            return lt < rt;
          }

          return false;
        }


        /**
         * Throw a reserved_word exception if the name is not allowed
         */
        void validate_object_name(const std::string &name) const
        {
          chaiscript::detail::threading::shared_lock<chaiscript::detail::threading::shared_mutex> l(m_mutex);

          if (m_state.m_reserved_words.find(name) != m_state.m_reserved_words.end())
          {
            throw exception::reserved_word_error(name);
          }
        }

        /**
         * Implementation detail for adding a function. Returns
         * true if the function was added, false if a function with the
         * same signature and name already exists.
         */
        bool add_function(const Proxy_Function &t_f, const std::string &t_name)
        {
          chaiscript::detail::threading::unique_lock<chaiscript::detail::threading::shared_mutex> l(m_mutex);

          std::map<std::string, std::vector<Proxy_Function> > &funcs = get_functions_int();

          std::map<std::string, std::vector<Proxy_Function> >::iterator itr
            = funcs.find(t_name);

          if (itr != funcs.end())
          {
            std::vector<Proxy_Function> &vec = itr->second;
            for (std::vector<Proxy_Function>::const_iterator itr2 = vec.begin();
                itr2 != vec.end();
                ++itr2)
            {
              if ((*t_f) == *(*itr2))
              {
                return false;
              }
            }

            vec.push_back(t_f);
            std::stable_sort(vec.begin(), vec.end(), &function_less_than);
          } else {
            std::vector<Proxy_Function> vec;
            vec.push_back(t_f);
            funcs.insert(std::make_pair(t_name, vec));
          }

          return true;
        }

        mutable chaiscript::detail::threading::shared_mutex m_mutex;
        mutable chaiscript::detail::threading::shared_mutex m_global_object_mutex;

        struct Stack_Holder
        {
          Stack_Holder()
            : stack(new StackData())
          {
            stack->push_back(Scope());
          }

          Stack stack;
        };  

        std::vector<Dynamic_Cast_Conversion> m_conversions;
        chaiscript::detail::threading::Thread_Storage<Stack_Holder> m_stack_holder;


        State m_state;

        Boxed_Value m_place_holder;
    };
  }
}

#endif

