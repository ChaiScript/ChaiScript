#ifndef __dispatchkit_hpp__
#define __dispatchkit_hpp__

#include <typeinfo>
#include <string>
#include <map>
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

namespace dispatchkit
{
  class Dispatch_Function : public Proxy_Function
  {
    public:
      Dispatch_Function(const std::vector<std::pair<std::string, boost::shared_ptr<Proxy_Function> > > &t_funcs)
        : m_funcs(t_funcs)
      {
      }

      virtual bool operator==(const Proxy_Function &f) const
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
        typedef std::vector<std::pair<std::string, boost::shared_ptr<Proxy_Function> > > function_vec;

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

    private:
      std::vector<std::pair<std::string, boost::shared_ptr<Proxy_Function> > > m_funcs;
  };  
  
  
  class Dispatch_Engine
  {
    public:
      typedef std::multimap<std::string, boost::shared_ptr<Proxy_Function> > Function_Map;
      typedef std::map<std::string, Type_Info> Type_Name_Map;
      typedef std::map<std::string, Boxed_Value> Scope;
      typedef std::deque<Scope> Stack;

      Dispatch_Engine()
        : m_place_holder(boost::shared_ptr<Placeholder_Object>(new Placeholder_Object()))
      {
        m_scopes.push_back(Scope());
      }

      bool register_function(const boost::shared_ptr<Proxy_Function> &f, const std::string &name)
      {
        return add_function(f, name);
      }

      template<typename Function>
        bool register_function(const Function &func, const std::string &name)
        {
          return add_function(boost::shared_ptr<Proxy_Function>(new Proxy_Function_Impl<Function>(func)), name);
        }

 
      template<typename Class>
        void set_object(const std::string &name, const Class &obj)
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

      template<typename Class>
        void add_object(const std::string &name, const Class &obj)
        {
          m_scopes.back()[name] = Boxed_Value(obj);
        }

      void new_scope()
      {
        m_scopes.push_back(Scope());
      }

      void pop_scope()
      {
        if (m_scopes.size() > 1)
        {
          m_scopes.pop_back();
        } else {
          throw std::range_error("Unable to pop global stack");
        }
      }

      Stack get_stack()
      {
        return m_scopes;
      }

      Stack set_stack(Stack s)
      {
        swap(s, m_scopes);
        return s;
      }


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

        std::vector<std::pair<std::string, Function_Map::mapped_type> > funcs = get_function_impl(name, false);

        if (funcs.empty())
        {
          throw std::range_error("Object not known: " + name);
        } else {
          return Boxed_Value(boost::shared_ptr<Proxy_Function>(new Dispatch_Function(funcs)));
        }
      }

      template<typename Type>
        void register_type(const std::string &name)
        {
          m_types.insert(std::make_pair(name, Get_Type_Info<Type>::get()));
        }

      Type_Info get_type(const std::string &name) const
      {
        Type_Name_Map::const_iterator itr = m_types.find(name);
        if (itr != m_types.end())
        {
          return itr->second;
        } else {
          throw std::range_error("Type Not Known");
        }
      }

      std::vector<Type_Name_Map::value_type> get_types() const
      {
        return std::vector<Type_Name_Map::value_type>(m_types.begin(), m_types.end());
      }

      std::vector<std::pair<std::string, Function_Map::mapped_type> > 
        get_function_impl(const std::string &t_name, bool include_objects) const
      {
        std::vector<std::pair<std::string, Function_Map::mapped_type> > funcs;

        if (include_objects)
        {
          try {
            funcs.insert(funcs.end(), 
                Function_Map::value_type(
                  t_name, 
                  boxed_cast<Function_Map::mapped_type>(get_object(t_name)))
                );
          } catch (const bad_boxed_cast &) {
          } catch (const std::range_error &) {
          }
        }

        std::pair<Function_Map::const_iterator, Function_Map::const_iterator> range
          = m_functions.equal_range(t_name);

        funcs.insert(funcs.end(), range.first, range.second);
        return funcs;
      }

      std::vector<std::pair<std::string, Function_Map::mapped_type> > 
        get_function(const std::string &t_name) const
      {
        return get_function_impl(t_name, true);
      }
 
      std::vector<Function_Map::value_type> get_functions() const
      {
        return std::vector<Function_Map::value_type>(m_functions.begin(), m_functions.end());
      }

    private:
      bool add_function(const boost::shared_ptr<Proxy_Function> &f, const std::string &t_name)
      {
        std::pair<Function_Map::const_iterator, Function_Map::const_iterator> range
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

      Function_Map m_functions;
      Type_Name_Map m_types;
      Boxed_Value m_place_holder;
  };

  void dump_object(Boxed_Value o)
  {
    std::cout << o.get_type_info().m_type_info->name() << std::endl;
  }

  void dump_type(const Type_Info &type)
  {
    std::cout << type.m_bare_type_info->name();
  }

  void dump_function(const Dispatch_Engine::Function_Map::value_type &f)
  {
    std::vector<Type_Info> params = f.second->get_param_types();

    dump_type(params.front());
    std::cout << " " << f.first << "(";

    for (std::vector<Type_Info>::const_iterator itr = params.begin() + 1;
         itr != params.end();
         ++itr)
    {
      dump_type(*itr);
      std::cout << ", ";
    }

    std::cout << ")" << std::endl;
  }

  void dump_system(const Dispatch_Engine &s)
  {
    std::cout << "Registered Types: " << std::endl;
    std::vector<Dispatch_Engine::Type_Name_Map::value_type> types = s.get_types();
    for (std::vector<Dispatch_Engine::Type_Name_Map::value_type>::const_iterator itr = types.begin();
         itr != types.end();
         ++itr)
    {
      std::cout << itr->first << ": ";
      dump_type(itr->second);
      std::cout << std::endl;
    }


    std::cout << std::endl;  std::vector<Dispatch_Engine::Function_Map::value_type> funcs = s.get_functions();

    std::cout << "Functions: " << std::endl;
    for (std::vector<Dispatch_Engine::Function_Map::value_type>::const_iterator itr = funcs.begin();
         itr != funcs.end();
         ++itr)
    {
      dump_function(*itr);
    }
    std::cout << std::endl;
  }


}

#endif

