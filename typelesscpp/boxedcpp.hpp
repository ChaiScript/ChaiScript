#ifndef __scripting_system_hpp__
#define __scripting_system_hpp__

#include <typeinfo>
#include <string>
#include <map>
#include <boost/shared_ptr.hpp>
#include <stdexcept>
#include <vector>

#include "boxed_value.hpp"
#include "proxy_functions.hpp"
#include "proxy_constructors.hpp"

class Scripting_System
{
  public:
    template<typename Function>
      void register_function(const Function &func, const std::string &name)
      {
        m_functions.insert(std::make_pair(name, boost::shared_ptr<Proxy_Function>(new Proxy_Function_Impl<Function>(func))));
      }

    template<typename Class>
      void add_object(const std::string &name, const Class &obj)
      {
        m_objects.insert(std::make_pair(name, Boxed_Value(obj)));
      }

    Boxed_Value get_object(const std::string &name) const
    {
      std::map<std::string, Boxed_Value>::const_iterator itr = m_objects.find(name);
      if (itr != m_objects.end())
      {
        return itr->second;
      } else {
        throw std::range_error("Object not known: " + name);
      }
    }

    template<typename Type>
      void register_type(const std::string &name)
      {
        m_types.insert(std::make_pair(name, &typeid(Type)));
      }

    boost::shared_ptr<Proxy_Function> get_function(const std::string &t_name)
    {
      std::map<std::string, boost::shared_ptr<Proxy_Function> >::const_iterator itr = m_functions.find(t_name);
      if (itr != m_functions.end())
      {
        return itr->second;
      } else {
        throw std::range_error("Function not known: " + t_name);
      }
    }

  private:
    std::map<std::string, Boxed_Value > m_objects;
    std::map<std::string, const std::type_info *> m_types;
    std::map<std::string, boost::shared_ptr<Proxy_Function> > m_functions;
};

#endif 
