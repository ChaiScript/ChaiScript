#ifndef __boxedcpp_system_hpp__
#define __boxedcpp_system_hpp__

#include <typeinfo>
#include <string>
#include <map>
#include <boost/shared_ptr.hpp>
#include <stdexcept>
#include <vector>

#include "boxed_value.hpp"
#include "type_info.hpp"
#include "proxy_functions.hpp"
#include "proxy_constructors.hpp"

class BoxedCPP_System
{
  public:
    typedef std::multimap<std::string, boost::shared_ptr<Proxy_Function> > Function_Map;
    typedef std::map<std::string, Type_Info> Type_Name_Map;

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
        m_types.insert(std::make_pair(name, Get_Type_Info<Type>()()));
      }

    std::vector<Type_Name_Map::value_type> get_types() const
    {
      return std::vector<Type_Name_Map::value_type>(m_types.begin(), m_types.end());
    }

    std::vector<Function_Map::value_type> get_function(const std::string &t_name) const
    {
      std::pair<Function_Map::const_iterator, Function_Map::const_iterator> range
        = m_functions.equal_range(t_name);

      return std::vector<Function_Map::value_type>(range.first, range.second);
    }

    std::vector<Function_Map::value_type> get_functions() const
    {
      return std::vector<Function_Map::value_type>(m_functions.begin(), m_functions.end());
    }

  private:
    std::map<std::string, Boxed_Value > m_objects;

    Function_Map m_functions;
    Type_Name_Map m_types;
};

void dump_type(const Type_Info &type)
{
  std::cout << type.m_bare_type_info->name();
}

void dump_function(const BoxedCPP_System::Function_Map::value_type &f)
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

void dump_system(const BoxedCPP_System &s)
{
  std::cout << "Registered Types: " << std::endl;
  std::vector<BoxedCPP_System::Type_Name_Map::value_type> types = s.get_types();
  for (std::vector<BoxedCPP_System::Type_Name_Map::value_type>::const_iterator itr = types.begin();
       itr != types.end();
       ++itr)
  {
    std::cout << itr->first << ": ";
    dump_type(itr->second);
    std::cout << std::endl;
  } 


  std::cout << std::endl;  std::vector<BoxedCPP_System::Function_Map::value_type> funcs = s.get_functions();

  std::cout << "Functions: " << std::endl;
  for (std::vector<BoxedCPP_System::Function_Map::value_type>::const_iterator itr = funcs.begin();
       itr != funcs.end();
       ++itr)
  {
    dump_function(*itr);
  } 
  std::cout << std::endl;
}

template<typename Ret, typename P1, typename P2>
Ret add(const P1 &p1, const P2 &p2)
{
  return p1 + p2;
}

template<typename Ret, typename P1, typename P2>
Ret multiply(const P1 &p1, const P2 &p2)
{
  return p1 * p2;
}


void bootstrap(BoxedCPP_System &s)
{
  s.register_type<void>("void");
  s.register_type<double>("double");
  s.register_type<int>("int");
  s.register_type<std::string>("string");

  s.register_function(boost::function<std::string (std::string, std::string)>(&add<std::string, std::string, std::string>), "+");

  s.register_function(boost::function<int (int, int)>(&add<int, int, int>), "+");
  s.register_function(boost::function<double (int, double)>(&add<double, int, double>), "+");
  s.register_function(boost::function<double (double, int)>(&add<double, double, int>), "+");
  s.register_function(boost::function<double (double, double)>(&add<double, double, double>), "+");

  s.register_function(boost::function<int (int, int)>(&multiply<int, int, int>), "*");
  s.register_function(boost::function<double (int, double)>(&multiply<double, int, double>), "*");
  s.register_function(boost::function<double (double, int)>(&multiply<double, double, int>), "*");
  s.register_function(boost::function<double (double, double)>(&multiply<double, double, double>), "*");
}

#endif 
