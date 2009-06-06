#ifndef __bootstrap_hpp
#define __bootstrap_hpp__

#include "boxedcpp.hpp"
#include "register_function.hpp"

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

//Add canonical forms of operators
template<typename T>
void add_oper_equals(BoxedCPP_System &s)
{
  register_function(s, &equals<const T&, const T&>, "=");
}

template<typename T>
void add_oper_add(BoxedCPP_System &s)
{
  register_function(s, &add<T, const T&, const T&>, "+");
}

template<typename T>
void add_oper_subtract(BoxedCPP_System &s)
{
  register_function(s, &subtract<T, const T&, const T&>, "-");
}

template<typename T>
void add_oper_divide(BoxedCPP_System &s)
{
  register_function(s, &divide<T, const T&, const T&>, "-");
}

template<typename T>
void add_oper_multiply(BoxedCPP_System &s)
{
  register_function(s, &multiply<T, const T&, const T&>, "*");
}

template<typename T>
void add_oper_not_equals(BoxedCPP_System &s)
{
  register_function(s, &not_equals<const T&, const T&>, "!=");
}

template<typename T>
void add_oper_less_than(BoxedCPP_System &s)
{
  register_function(s, &less_than<const T&, const T&>, "<");
}

template<typename T>
void add_oper_greater_than(BoxedCPP_System &s)
{
  register_function(s, &greater_than<const T&, const T&>, ">");
}

template<typename T>
void add_oper_less_than_equals(BoxedCPP_System &s)
{
  register_function(s, &less_than_equals<const T&, const T&>, "<=");
}

template<typename T>
void add_oper_greater_than_equals(BoxedCPP_System &s)
{
  register_function(s, &greater_than_equals<const T&, const T&>, ">=");
}


template<typename T, typename R>
void add_opers_comparison_overload(BoxedCPP_System &s)
{
  register_function(s, &equals<const T&, const R&>, "==");
  register_function(s, &not_equals<const T&, const R&>, "!=");
  register_function(s, &less_than<const T&, const R&>, "<");
  register_function(s, &greater_than<const T&, const R&>, ">");
  register_function(s, &less_than_equals<const T&, const R&>, "<=");
  register_function(s, &greater_than_equals<const T&, const R&>, ">=");
}

template<typename T>
void add_opers_comparison(BoxedCPP_System &s)
{
  add_opers_comparison_overload<T, T>(s);
}

template<typename Ret, typename T, typename R>
void add_opers_arithmetic_overload(BoxedCPP_System &s)
{
  register_function(s, &add<Ret, T, R>, "+");
  register_function(s, &subtract<Ret, T, R>, "-");
  register_function(s, &divide<Ret, T, R>, "/");
  register_function(s, &multiply<Ret, T, R>, "*");
  register_function(s, &timesequal<T, R>, "*=");
}

template<typename T>
void add_opers_arithmetic(BoxedCPP_System &s)
{
  add_opers_arithmetic_overload<T, T, T>(s);
}

//Built in to_string operator
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

  add_opers_comparison<int>(s);
  add_opers_comparison<double>(s);
  add_opers_comparison<char>(s);
  add_opers_comparison<std::string>(s);

  add_opers_comparison_overload<int, double>(s);
  add_opers_comparison_overload<double, int>(s);

  add_opers_arithmetic<int>(s);
  add_opers_arithmetic<double>(s);

  add_opers_arithmetic_overload<double, int, double>(s);
  add_opers_arithmetic_overload<double, double, int>(s);

  add_oper_add<std::string>(s);

  register_function(s, &bool_and<bool, bool>, "&&");
  register_function(s, &bool_or<bool, bool>, "||");

  register_function(s, &to_string<int>, "to_string");
  register_function(s, &to_string<const std::string &>, "to_string");
  register_function(s, &to_string<char>, "to_string");
  register_function(s, &to_string<double>, "to_string");
}

#endif
