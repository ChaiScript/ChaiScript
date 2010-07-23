// This file is distributed under the BSD License.
// See "license.txt" for details.
// Copyright 2009-2010, Jonathan Turner (jturner@minnow-lang.org)
// and Jason Turner (lefticus@gmail.com)
// http://www.chaiscript.com

#ifndef __dispatchkit_hpp__
#define __dispatchkit_hpp__

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

namespace chaiscript
{
  class Module
  {
    public:
      Module &add(const Type_Info &ti, const std::string &name)
      {
        m_typeinfos.push_back(std::make_pair(ti, name));
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
        }

    private:
      std::vector<std::pair<Type_Info, std::string> > m_typeinfos;
      std::vector<std::pair<Proxy_Function, std::string> > m_funcs;
      std::vector<std::string> m_evals;

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
        void apply_eval(InItr begin, InItr end, T &t) const
        {
          while (begin != end)
          {
            t.eval(*begin);
            ++begin;
          }
        }


  };

  typedef boost::shared_ptr<Module> ModulePtr;

  /**
   * A Proxy_Function implementation that is able to take
   * a vector of Proxy_Functions and perform a dispatch on them. It is 
   * used specifically in the case of dealing with Function object variables
   */
  class Dispatch_Function : public Proxy_Function_Base
  {
    public:
      Dispatch_Function(const std::vector<std::pair<std::string, Proxy_Function > > &t_funcs)
        : Proxy_Function_Base(std::vector<Type_Info>()),
          m_funcs(t_funcs)
      {
      }

      virtual bool operator==(const Proxy_Function_Base &) const
      {
        return false;
      }

      virtual ~Dispatch_Function() {}

      virtual Boxed_Value operator()(const std::vector<Boxed_Value> &params) const
      {
        return dispatch(m_funcs.begin(), m_funcs.end(), params);
      }

      virtual int get_arity() const
      {
        return -1;
      }

      virtual bool call_match(const std::vector<Boxed_Value> &vals) const
      {
        typedef std::vector<std::pair<std::string, Proxy_Function > > function_vec;

        function_vec::const_iterator begin = m_funcs.begin();
        function_vec::const_iterator end = m_funcs.end();

        while (begin != end)
        {
          if (begin->second->call_match(vals))
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
        return "";
      }

    private:
      std::vector<std::pair<std::string, Proxy_Function > > m_funcs;
  };  


  /**
   * Exception thrown in the case that a multi method dispatch fails
   * because no matching function was found
   * at runtime due to either an arity_error, a guard_error or a bad_boxed_cast
   * exception
   */
  struct reserved_word_error : std::runtime_error
  {
    reserved_word_error(const std::string &word) throw()
      : std::runtime_error("Reserved word not allowed in object name: " + word)
    {
    }

    virtual ~reserved_word_error() throw() {}
  };

  /**
   * Exception thrown in the case that a non-const object was added as a shared object
   */
  struct global_non_const : std::runtime_error
  {
    global_non_const() throw()
      : std::runtime_error("a global object must be const")
    {
    }

    virtual ~global_non_const() throw() {}
  };


  /**
   * Main class for the dispatchkit. Handles management
   * of the object stack, functions and registered types.
   */
  class Dispatch_Engine
  {
    public:
      typedef std::map<std::string, chaiscript::Type_Info> Type_Name_Map;
      typedef std::map<std::string, Boxed_Value> Scope;
      typedef boost::tuples::tuple<std::map<std::string, Boxed_Value>, std::deque<Scope>, bool> StackData;
      typedef boost::shared_ptr<StackData> Stack;

      struct State
      {
        std::multimap<std::string, Proxy_Function> m_functions;
        std::map<std::string, Boxed_Value> m_global_objects;
        Type_Name_Map m_types;
        std::set<std::string> m_reserved_words;
      };

      Dispatch_Engine()
        : m_place_holder(boost::shared_ptr<Placeholder_Object>(new Placeholder_Object()))
      {
        StackData &stack = get_stack_data();
        stack.get<1>().push_back(Scope());
      }

      ~Dispatch_Engine()
      {
        Boxed_Value::clear_cache();
      }

      /**
       * Add a new named Proxy_Function to the system
       */
      bool add(const Proxy_Function &f, const std::string &name)
      {
        validate_object_name(name);

        StackData &stack = get_stack_data();
        stack.get<0>().erase(name);
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

        for (int i = stack.get<1>().size()-1; i >= 0; --i)
        {
          std::map<std::string, Boxed_Value>::const_iterator itr = (stack.get<1>())[i].find(name);
          if (itr != (stack.get<1>())[i].end())
          {
            stack.get<0>().erase(name);
            (stack.get<1>())[i][name] = obj;
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
        stack.get<0>().erase(name);
        stack.get<1>().back()[name] = obj;
      }

      /**
       * Adds a new global shared object, between all the threads
       */
      void add_global_const(const Boxed_Value &obj, const std::string &name)
      {
        StackData &stack = get_stack_data();
        validate_object_name(name);
        if (!obj.is_const())
        {
          throw global_non_const();
        }

        stack.get<0>().erase(name);
        
#ifndef CHAISCRIPT_NO_THREADS
        boost::unique_lock<boost::shared_mutex> l(m_global_object_mutex);
#endif

        m_state.m_global_objects[name] = obj;
      }

      /**
       * Adds a new scope to the stack
       */
      void new_scope()
      {
        StackData &stack = get_stack_data();
        stack.get<1>().push_back(Scope());
      }

      /**
       * Pops the current scope from the stack
       */
      void pop_scope()
      {
        StackData &stack = get_stack_data();
        if (stack.get<1>().size() > 1)
        {
          Scope &scope(stack.get<1>().back());
          for (Scope::const_iterator itr = scope.begin();
               itr != scope.end();
               ++itr)
          {
            stack.get<0>().erase(itr->first);
          }
          stack.get<1>().pop_back();
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
        s->get<1>().push_back(Scope());
        s->get<2>() = false;
        return s;
      }

      Stack get_stack() const
      {
        return m_stack_holder->stack;
      }

      void sync_cache()
      {
#ifndef CHAISCRIPT_NO_THREADS
        boost::shared_lock<boost::shared_mutex> l(m_mutex);
#endif
        sync_cache_no_lock();
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
        std::map<std::string, Boxed_Value> &cache = stack.get<0>();

        // Is it in the cache?
        std::map<std::string, Boxed_Value>::const_iterator itr = cache.find(name);
        if (itr != cache.end())
        {
          return itr->second;
        }

        // Is it in the stack?
        for (int i = stack.get<1>().size()-1; i >= 0; --i)
        {
          std::map<std::string, Boxed_Value>::const_iterator stackitr = (stack.get<1>())[i].find(name);
          if (stackitr != (stack.get<1>())[i].end())
          {
            cache[name] = stackitr->second;
            return stackitr->second;
          }
        }

        // Is the value we are looking for a global?
        {
#ifndef CHAISCRIPT_NO_THREADS
          boost::shared_lock<boost::shared_mutex> l(m_global_object_mutex);
#endif

          itr = m_state.m_global_objects.find(name);
          if (itr != m_state.m_global_objects.end())
          {
            cache[name] = itr->second;
            return itr->second;
          }
        }

        // If all that failed, then check to see if it's a function
        std::vector<std::pair<std::string, std::multimap<std::string, Proxy_Function >::mapped_type> > funcs = get_function(name);

        if (funcs.empty())
        {
          throw std::range_error("Object not known: " + name);
        } else {
          Boxed_Value f(Const_Proxy_Function(new Dispatch_Function(funcs)));
          cache[name] = f;
          return f;
        }
      }

      /**
       * Registers a new named type
       */
      void add(const Type_Info &ti, const std::string &name)
      {
        add_global_const(const_var(ti), name + "_type");
       
#ifndef CHAISCRIPT_NO_THREADS
        boost::unique_lock<boost::shared_mutex> l(m_mutex);
#endif

        m_state.m_types.insert(std::make_pair(name, ti));
      }

      /**
       * Returns the type info for a named type
       */
      Type_Info get_type(const std::string &name) const
      {
#ifndef CHAISCRIPT_NO_THREADS
        boost::shared_lock<boost::shared_mutex> l(m_mutex);
#endif

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
#ifndef CHAISCRIPT_NO_THREADS
        boost::shared_lock<boost::shared_mutex> l(m_mutex);
#endif

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
#ifndef CHAISCRIPT_NO_THREADS
        boost::shared_lock<boost::shared_mutex> l(m_mutex);
#endif

        return std::vector<std::pair<std::string, Type_Info> >(m_state.m_types.begin(), m_state.m_types.end());
      }

      /**
       * Return a function by name
       */
      std::vector<std::pair<std::string, std::multimap<std::string, Proxy_Function >::mapped_type> > 
        get_function(const std::string &t_name) const
      {
        std::pair<std::multimap<std::string, Proxy_Function >::const_iterator, std::multimap<std::string, Proxy_Function >::const_iterator> range
          = get_function_cache().equal_range(t_name);

        return  std::vector<std::pair<std::string, std::multimap<std::string, Proxy_Function >::mapped_type> >(range.first, range.second);
      }

      /**
       * Return true if a function exists
       */
      bool function_exists(const std::string &name) const
      {
        std::multimap<std::string, Proxy_Function> &functions = get_function_cache();
        return functions.find(name) != functions.end();
      }

      /**
       * Get a vector of all registered functions
       */
      std::vector<std::pair<std::string, Proxy_Function > > get_functions() const
      {
        std::multimap<std::string, Proxy_Function> &functions = get_function_cache();
        return std::vector<std::pair<std::string, Proxy_Function > >(functions.begin(), functions.end());
      }

      void add_reserved_word(const std::string &name)
      {
#ifndef CHAISCRIPT_NO_THREADS
        boost::unique_lock<boost::shared_mutex> l(m_mutex);
#endif

        m_state.m_reserved_words.insert(name);
      }

      Boxed_Value call_function(const std::string &t_name, const std::vector<Boxed_Value> &params) const
      {
        std::multimap<std::string, Proxy_Function> &functions = get_function_cache();
        std::pair<std::multimap<std::string, Proxy_Function >::const_iterator, std::multimap<std::string, Proxy_Function >::const_iterator> range
          = functions.equal_range(t_name);

        return dispatch(range.first, range.second, params);
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
      bool is_type(Boxed_Value r, const std::string &user_typename) const
      {
        try {
          if (get_type(user_typename).bare_equal(r.get_type_info()))
          {
            return true;
          }
        } catch (const std::range_error &) {
        }

        try {
          const Dynamic_Object &d = boxed_cast<const Dynamic_Object &>(r);
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
#ifndef CHAISCRIPT_NO_THREADS
        boost::unique_lock<boost::shared_mutex> l(m_mutex);
        boost::unique_lock<boost::shared_mutex> l2(m_global_object_mutex);
#endif

        return m_state;
      }

      void set_state(const State &t_state)
      {
#ifndef CHAISCRIPT_NO_THREADS
        boost::unique_lock<boost::shared_mutex> l(m_mutex);
        boost::unique_lock<boost::shared_mutex> l2(m_global_object_mutex);
#endif

        m_state = t_state;
        sync_cache_no_lock();
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

      std::multimap<std::string, Proxy_Function> &get_function_cache() const
      {
        return m_stack_holder->function_cache;
      }

      void sync_cache_no_lock()
      {
        m_stack_holder->stack->get<0>().clear();

        get_function_cache() = m_state.m_functions;
      }


      /**
       * Throw a reserved_word exception if the name is not allowed
       */
      void validate_object_name(const std::string &name) const
      {
#ifndef CHAISCRIPT_NO_THREADS
        boost::shared_lock<boost::shared_mutex> l(m_mutex);
#endif

        if (m_state.m_reserved_words.find(name) != m_state.m_reserved_words.end())
        {
          throw reserved_word_error(name);
        }
      }

      /**
       * Implementation detail for adding a function. Returns
       * true if the function was added, false if a function with the
       * same signature and name already exists.
       */
      bool add_function(const Proxy_Function &f, const std::string &t_name)
      {
#ifndef CHAISCRIPT_NO_THREADS
        boost::unique_lock<boost::shared_mutex> l(m_mutex);
#endif

        std::pair<std::multimap<std::string, Proxy_Function >::const_iterator, std::multimap<std::string, Proxy_Function >::const_iterator> range
          = m_state.m_functions.equal_range(t_name);

        while (range.first != range.second)
        {
          if ((*f) == *(range.first->second))
          {
            return false;
          }
          ++range.first;
        }

        m_state.m_functions.insert(std::make_pair(t_name, f));
        get_function_cache().insert(std::make_pair(t_name, f));

        return true;
      }

#ifndef CHAISCRIPT_NO_THREADS
      mutable boost::shared_mutex m_mutex;
      mutable boost::shared_mutex m_global_object_mutex;
#endif

      struct Stack_Holder
      {
        Stack_Holder()
          : stack(new StackData())
        {
          stack->get<2>() = true;
        }

        Stack stack;

        std::multimap<std::string, Proxy_Function> function_cache;
      };  

      chaiscript::threading::Thread_Storage<Stack_Holder> m_stack_holder;


      State m_state;

      Boxed_Value m_place_holder;
  };

}

#endif

