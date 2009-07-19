// This file is distributed under the BSD License.
// See "license.txt" for details.
// Copyright 2009, Jonathan Turner (jturner@minnow-lang.org)
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
#include <stdexcept>
#include <vector>
#include <iostream>
#include <deque>

#include "boxed_value.hpp"
#include "type_info.hpp"
#include "proxy_functions.hpp"
#include "proxy_constructors.hpp"

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

      template<typename T>
        void apply(T &t) const
        {
          apply(m_typeinfos.begin(), m_typeinfos.end(), t);
          apply(m_funcs.begin(), m_funcs.end(), t);
        }

    private:
      std::vector<std::pair<Type_Info, std::string> > m_typeinfos;
      std::vector<std::pair<Proxy_Function, std::string> > m_funcs;

      template<typename T, typename InItr>
        void apply(InItr begin, InItr end, T &t) const
        {
          while (begin != end)
          {
            t.add(begin->first, begin->second);
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
        : m_funcs(t_funcs)
      {
      }

      virtual bool operator==(const Proxy_Function_Base &) const
      {
        return false;
      }

      virtual ~Dispatch_Function() {}

      virtual Boxed_Value operator()(const std::vector<Boxed_Value> &params)
      {
        return dispatch(m_funcs, params);
      }

      virtual std::vector<Type_Info> get_param_types() const
      {
        return std::vector<Type_Info>();
      }

      virtual bool types_match(const std::vector<Boxed_Value> &types) const
      {
        typedef std::vector<std::pair<std::string, Proxy_Function > > function_vec;

        function_vec::const_iterator begin = m_funcs.begin();
        function_vec::const_iterator end = m_funcs.end();

        while (begin != end)
        {
          if (begin->second->types_match(types))
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
   * Main class for the dispatchkit. Handles management
   * of the object stack, functions and registered types.
   */
  class Dispatch_Engine
  {
    public:
      typedef std::map<std::string, chaiscript::Type_Info> Type_Name_Map;
      typedef std::map<std::string, Boxed_Value> Scope;
      typedef std::deque<Scope> Stack;

      Dispatch_Engine()
        : m_place_holder(boost::shared_ptr<Placeholder_Object>(new Placeholder_Object()))
      {
        m_scopes.push_back(Scope());
      }

      /**
       * Add a new named Proxy_Function to the system
       */
      bool add(const Proxy_Function &f, const std::string &name)
      {
        return add_function(f, name);
      }

      /**
       * Add a module's worth of registrations to the system
       */
      void add(const ModulePtr &m)
      {
        m->apply(*this);
      }

      /**
       * Set the value of an object, by name. If the object
       * is not available in the current scope it is created
       */
      void add(const Boxed_Value &obj, const std::string &name)
      {
        for (int i = m_scopes.size()-1; i >= 0; --i)
        {
          std::map<std::string, Boxed_Value>::const_iterator itr = m_scopes[i].find(name);
          if (itr != m_scopes[i].end())
          {
            m_scopes[i][name] = Boxed_Value(obj);
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
        m_scopes.back()[name] = Boxed_Value(obj);
      }

      /**
       * Adds a new scope to the stack
       */
      void new_scope()
      {
        m_scopes.push_back(Scope());
      }

      /**
       * Pops the current scope from the stack
       */
      void pop_scope()
      {
        if (m_scopes.size() > 1)
        {
          m_scopes.pop_back();
        } else {
          throw std::range_error("Unable to pop global stack");
        }
      }

      /**
       * Returns the current stack
       */
      Stack get_stack()
      {
        return m_scopes;
      }

      /**
       * Swaps out the stack with a new stack
       * \returns the old stack
       * \param[in] s The new stack
       */
      Stack set_stack(Stack s)
      {
        std::swap(s, m_scopes);
        return s;
      }

      /**
       * Searches the current stack for an object of the given name
       * includes a special overload for the _ place holder object to
       * ensure that it is always in scope.
       */
      Boxed_Value get_object(const std::string &name) const
      {
        if (name == "_")
        {
          return m_place_holder;
        }

        for (int i = m_scopes.size()-1; i >= 0; --i)
        {
          std::map<std::string, Boxed_Value>::const_iterator itr = m_scopes[i].find(name);
          if (itr != m_scopes[i].end())
          {
            return itr->second;
          }
        }

        std::vector<std::pair<std::string, std::multimap<std::string, Proxy_Function >::mapped_type> > funcs = get_function_impl(name, false);

        if (funcs.empty())
        {
          throw std::range_error("Object not known: " + name);
        } else {
          return Boxed_Value(Proxy_Function(new Dispatch_Function(funcs)));
        }
      }

      /**
       * Registers a new named type
       */
      void add(const Type_Info &ti, const std::string &name)
      {
        m_types.insert(std::make_pair(name, ti));
      }

      /**
       * Returns the type info for a named type
       */
      Type_Info get_type(const std::string &name) const
      {
        Type_Name_Map::const_iterator itr = m_types.find(name);

        if (itr != m_types.end())
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
        for (Type_Name_Map::const_iterator itr = m_types.begin();
             itr != m_types.end();
             ++itr)
        {
          if (itr->second.m_bare_type_info == ti.m_bare_type_info)
          {
            return itr->first;
          }
        }

        return ti.m_bare_type_info->name();
      }

      /**
       * Return all registered types
       */
      std::vector<std::pair<std::string, Type_Info> > get_types() const
      {
        return std::vector<std::pair<std::string, Type_Info> >(m_types.begin(), m_types.end());
      }

      /**
       * Return a function by name
       */
      std::vector<std::pair<std::string, std::multimap<std::string, Proxy_Function >::mapped_type> > 
        get_function(const std::string &t_name) const
      {
        return get_function_impl(t_name, true);
      }
 
      /**
       * Get a vector of all registered functions
       */
      std::vector<std::pair<std::string, Proxy_Function > > get_functions() const
      {
        return std::vector<std::pair<std::string, Proxy_Function > >(m_functions.begin(), m_functions.end());
      }

    private:
      /**
       * Implementation detail for searching for a function by name.
       * Looks for all registered global functions and optionally for an object
       * in scope with the same name
       */
      std::vector<std::pair<std::string, std::multimap<std::string, Proxy_Function >::mapped_type> > 
        get_function_impl(const std::string &t_name, bool include_objects) const
      {
        std::vector<std::pair<std::string, std::multimap<std::string, Proxy_Function >::mapped_type> > funcs;

        if (include_objects)
        {
          try {
            funcs.insert(funcs.end(), 
              std::make_pair(
                  t_name, 
                  boxed_cast<std::multimap<std::string, Proxy_Function >::mapped_type>(get_object(t_name)))
                );
          } catch (const bad_boxed_cast &) {
          } catch (const std::range_error &) {
          }
        }

        std::pair<std::multimap<std::string, Proxy_Function >::const_iterator, std::multimap<std::string, Proxy_Function >::const_iterator> range
          = m_functions.equal_range(t_name);

        funcs.insert(funcs.end(), range.first, range.second);
        return funcs;
      }

      /**
       * Implementation detail for adding a function. Returns
       * true if the function was added, false if a function with the
       * same signature and name already exists.
       */
      bool add_function(const Proxy_Function &f, const std::string &t_name)
      {
        std::pair<std::multimap<std::string, Proxy_Function >::const_iterator, std::multimap<std::string, Proxy_Function >::const_iterator> range
          = m_functions.equal_range(t_name);

        while (range.first != range.second)
        {
          if ((*f) == *(range.first->second))
          {
            return false;
          }
          ++range.first;
        }

        m_functions.insert(std::make_pair(t_name, f));
        return true;
      }

      std::deque<Scope> m_scopes;

      std::multimap<std::string, Proxy_Function > m_functions;
      Type_Name_Map m_types;
      Boxed_Value m_place_holder;
  };

  /**
   * Dump object info to stdout
   */
  void dump_object(Boxed_Value o, const Dispatch_Engine &e)
  {
    std::cout << e.get_type_name(o.get_type_info()) << std::endl;
  }

  /**
   * Dump type info to stdout
   */
  void dump_type(const Type_Info &type, const Dispatch_Engine &e)
  {
    std::cout << e.get_type_name(type);
  }

  /**
   * Dump function to stdout
   */
  void dump_function(const std::pair<const std::string, Proxy_Function > &f, const Dispatch_Engine &e)
  {
    std::vector<Type_Info> params = f.second->get_param_types();
    std::string annotation = f.second->annotation();

    if (annotation.size() > 0) {
        std::cout << annotation;
    }
    dump_type(params.front(), e);
    std::cout << " " << f.first << "(";

    for (std::vector<Type_Info>::const_iterator itr = params.begin() + 1;
         itr != params.end();
         )
    {
      dump_type(*itr, e);
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
  void dump_system(const Dispatch_Engine &s)
  {
    std::cout << "Registered Types: " << std::endl;
    std::vector<std::pair<std::string, Type_Info> > types = s.get_types();
    for (std::vector<std::pair<std::string, Type_Info> >::const_iterator itr = types.begin();
         itr != types.end();
         ++itr)
    {
      std::cout << itr->first << ": ";
      std::cout << itr->second.m_bare_type_info->name();
      std::cout << std::endl;
    }

    std::cout << std::endl;  
    std::vector<std::pair<std::string, Proxy_Function > > funcs = s.get_functions();

    std::cout << "Functions: " << std::endl;
    for (std::vector<std::pair<std::string, Proxy_Function > >::const_iterator itr = funcs.begin();
         itr != funcs.end();
         ++itr)
    {
      dump_function(*itr, s);
    }
    std::cout << std::endl;
  }

  /**
   * return true if the Boxed_Value matches the registered type by name
   */
  static bool is_type(const Dispatch_Engine &e, const std::string &user_typename, Boxed_Value r)
  {
    try {
      return e.get_type(user_typename) == r.get_type_info();
    } catch (const std::range_error &) {
      return false;
    }
  }

}

#endif

