#ifndef __bootstrap_hpp
#define __bootstrap_hpp__

#include "boxedcpp.hpp"

template<typename Ret, typename P1, typename P2>
Ret add(P1 p1, P2 p2)
{
  return p1 + p2;
}

template<typename Ret, typename P1, typename P2>
Ret subtract(P1 p1, P2 p2)
{
  return p1 - p2;
}

template<typename Ret, typename P1, typename P2>
Ret divide(P1 p1, P2 p2)
{
  return p1 / p2;
}


template<typename Ret, typename P1, typename P2>
Ret multiply(P1 p1, P2 p2)
{
  return p1 * p2;
}

template<typename P1, typename P2>
bool bool_and(P1 p1, P2 p2)
{
  return p1 && p2;
}

template<typename P1, typename P2>
bool bool_or(P1 p1, P2 p2)
{
  return p1 || p2;
}

template<typename P1, typename P2>
bool equals(P1 p1, P2 p2)
{
  return p1 == p2;
}

template<typename P1, typename P2>
bool not_equals(P1 p1, P2 p2)
{
  return p1 != p2;
}

template<typename P1, typename P2>
bool less_than(P1 p1, P2 p2)
{
  return p1 < p2;
}

template<typename P1, typename P2>
bool greater_than(P1 p1, P2 p2)
{
  return p1 > p2;
}

template<typename P1, typename P2>
bool less_than_equals(P1 p1, P2 p2)
{
  return p1 <= p2;
}

template<typename P1, typename P2>
bool greater_than_equals(P1 p1, P2 p2)
{
  return p1 >= p2;
}

template<typename P1, typename P2>
P1 &timesequal(P1 &p1, P2 p2)
{
  return (p1 *= p2);
}

template<typename Input>
std::string to_string(Input i)
{
  return boost::lexical_cast<std::string>(i);
}

void bootstrap(BoxedCPP_System &s)
{
  s.register_type<void>("void");
  s.register_type<double>("double");
  s.register_type<int>("int");
  s.register_type<char>("char");
  s.register_type<bool>("bool");
  s.register_type<std::string>("string");

  s.register_function(boost::function<std::string (const std::string &, const std::string&)>(&add<std::string, const std::string &, const std::string &>), "+");

  s.register_function(boost::function<int (int, int)>(&add<int, int, int>), "+");
  s.register_function(boost::function<double (int, double)>(&add<double, int, double>), "+");
  s.register_function(boost::function<double (double, int)>(&add<double, double, int>), "+");
  s.register_function(boost::function<double (double, double)>(&add<double, double, double>), "+");

  s.register_function(boost::function<int (int, int)>(&subtract<int, int, int>), "-");
  s.register_function(boost::function<double (int, double)>(&subtract<double, int, double>), "-");
  s.register_function(boost::function<double (double, int)>(&subtract<double, double, int>), "-");
  s.register_function(boost::function<double (double, double)>(&subtract<double, double, double>), "-");

  s.register_function(boost::function<int (int, int)>(&divide<int, int, int>), "/");
  s.register_function(boost::function<double (int, double)>(&divide<double, int, double>), "/");
  s.register_function(boost::function<double (double, int)>(&divide<double, double, int>), "/");
  s.register_function(boost::function<double (double, double)>(&divide<double, double, double>), "/");

  s.register_function(boost::function<bool (bool, bool)>(&bool_and<bool, bool>), "&&");
  s.register_function(boost::function<bool (bool, bool)>(&bool_or<bool, bool>), "||");

  s.register_function(boost::function<bool (const std::string &, const std::string &)>(&equals<const std::string &, const std::string &>), "==");
  s.register_function(boost::function<bool (int, int)>(&equals<int, int>), "==");
  s.register_function(boost::function<bool (int, double)>(&equals<int, double>), "==");
  s.register_function(boost::function<bool (double, int)>(&equals<double, int>), "==");
  s.register_function(boost::function<bool (double, double)>(&equals<double, double>), "==");

  s.register_function(boost::function<bool (const std::string &, const std::string &)>(&not_equals<const std::string &, const std::string &>), "!=");
  s.register_function(boost::function<bool (int, int)>(&not_equals<int, int>), "!=");
  s.register_function(boost::function<bool (int, double)>(&not_equals<int, double>), "!=");
  s.register_function(boost::function<bool (double, int)>(&not_equals<double, int>), "!=");
  s.register_function(boost::function<bool (double, double)>(&not_equals<double, double>), "!=");

  s.register_function(boost::function<bool (int, int)>(&less_than<int, int>), "<");
  s.register_function(boost::function<bool (int, double)>(&less_than<int, double>), "<");
  s.register_function(boost::function<bool (double, int)>(&less_than<double, int>), "<");
  s.register_function(boost::function<bool (double, double)>(&less_than<double, double>), "<");

  s.register_function(boost::function<bool (int, int)>(&greater_than<int, int>), ">");
  s.register_function(boost::function<bool (int, double)>(&greater_than<int, double>), ">");
  s.register_function(boost::function<bool (double, int)>(&greater_than<double, int>), ">");
  s.register_function(boost::function<bool (double, double)>(&greater_than<double, double>), ">");

  s.register_function(boost::function<bool (int, int)>(&less_than_equals<int, int>), "<=");
  s.register_function(boost::function<bool (int, double)>(&less_than_equals<int, double>), "<=");
  s.register_function(boost::function<bool (double, int)>(&less_than_equals<double, int>), "<=");
  s.register_function(boost::function<bool (double, double)>(&less_than_equals<double, double>), "<=");

  s.register_function(boost::function<bool (int, int)>(&greater_than_equals<int, int>), ">=");
  s.register_function(boost::function<bool (int, double)>(&greater_than_equals<int, double>), ">=");
  s.register_function(boost::function<bool (double, int)>(&greater_than_equals<double, int>), ">=");
  s.register_function(boost::function<bool (double, double)>(&greater_than_equals<double, double>), ">=");

  s.register_function(boost::function<int (int, int)>(&multiply<int, int, int>), "*");
  s.register_function(boost::function<double (int, double)>(&multiply<double, int, double>), "*");
  s.register_function(boost::function<double (double, int)>(&multiply<double, double, int>), "*");
  s.register_function(boost::function<double (double, double)>(&multiply<double, double, double>), "*");

  s.register_function(boost::function<std::string (int)>(&to_string<int>), "to_string");
  s.register_function(boost::function<std::string (const std::string &)>(&to_string<const std::string &>), "to_string");
  s.register_function(boost::function<std::string (char)>(&to_string<char>), "to_string");
  s.register_function(boost::function<std::string (double)>(&to_string<double>), "to_string");

  s.register_function(boost::function<int &(int&, int)>(&timesequal<int, int>), "*=");
  s.register_function(boost::function<double &(double&, int)>(&timesequal<double, int>), "*=");
  s.register_function(boost::function<double &(double&, double)>(&timesequal<double, double>), "*=");
  s.register_function(boost::function<int &(int&, double)>(&timesequal<int, double>), "*=");

}

#endif
