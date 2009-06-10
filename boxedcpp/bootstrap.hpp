#ifndef __bootstrap_hpp
#define __bootstrap_hpp__

#include "boxedcpp.hpp"
#include "register_function.hpp"

template<typename Ret, typename P1, typename P2>
Ret add(P1 p1, P2 p2)
{
  return p1 + p2;
}

Boxed_Value pod_add(Boxed_POD_Value l, Boxed_POD_Value r)
{
  return l + r;
}


template<typename Ret, typename P1, typename P2>
Ret subtract(P1 p1, P2 p2)
{
  return p1 - p2;
}

Boxed_Value pod_subtract(Boxed_POD_Value l, Boxed_POD_Value r)
{
  return l - r;
}


template<typename Ret, typename P1, typename P2>
Ret divide(P1 p1, P2 p2)
{
  return p1 / p2;
}

Boxed_Value pod_divide(Boxed_POD_Value l, Boxed_POD_Value r)
{
  return l / r;
}


template<typename Ret, typename P1, typename P2>
Ret multiply(P1 p1, P2 p2)
{
  return p1 * p2;
}

Boxed_Value pod_multiply(Boxed_POD_Value l, Boxed_POD_Value r)
{
  return l * r;
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
P1 &assign(P1 &p1, const P2 &p2)
{
  return (p1 = p2);
}

template<typename P1>
P1 &assign_pod(P1 &p1, Boxed_POD_Value v)
{
  if (v.m_isfloat)
  {
    return (p1 = v.d);
  } else {
    return (p1 = v.i);
  }
}


template<typename P1, typename P2>
bool equals(P1 p1, P2 p2)
{
  return p1 == p2;
}

bool pod_equals(Boxed_POD_Value l, Boxed_POD_Value r)
{
  return l == r;
}

template<typename P1, typename P2>
bool not_equals(P1 p1, P2 p2)
{
  return p1 != p2;
}

bool pod_not_equals(Boxed_POD_Value l, Boxed_POD_Value r)
{
  return l != r;
}


template<typename P1, typename P2>
bool less_than(P1 p1, P2 p2)
{
  return p1 < p2;
}

bool pod_less_than(Boxed_POD_Value l, Boxed_POD_Value r)
{
  return l < r;
}
  
template<typename P1, typename P2>
bool greater_than(P1 p1, P2 p2)
{
  return p1 > p2;
}

bool pod_greater_than(Boxed_POD_Value l, Boxed_POD_Value r)
{
  return l > r;
}
 
template<typename P1, typename P2>
bool less_than_equals(P1 p1, P2 p2)
{
  return p1 <= p2;
}

bool pod_less_than_equals(Boxed_POD_Value l, Boxed_POD_Value r)
{
  return l <= r;
}

template<typename P1, typename P2>
bool greater_than_equals(P1 p1, P2 p2)
{
  return p1 >= p2;
}

bool pod_greater_than_equals(Boxed_POD_Value l, Boxed_POD_Value r)
{
  return l >= r;
}

template<typename P1, typename P2>
P1 &timesequal(P1 &p1, const P2 &p2)
{
  return (p1 *= p2);
}

template<typename P1>
P1 &timesequal_pod(P1 &p1, Boxed_POD_Value r)
{
  if (r.m_isfloat)
  {
    return (p1 *= r.d);
  } else {
    return (p1 *= r.i);
  } 
}


template<typename P1, typename P2>
P1 &dividesequal(P1 &p1, const P2 &p2)
{
  return (p1 /= p2);
}

template<typename P1>
P1 &dividesequal_pod(P1 &p1, Boxed_POD_Value r)
{
  if (r.m_isfloat)
  {
    return (p1 /= r.d);
  } else {
    return (p1 /= r.i);
  } 
}


template<typename P1, typename P2>
P1 &addsequal(P1 &p1, const P2 &p2)
{
  return (p1 += p2);
}

template<typename P1>
P1 &addsequal_pod(P1 &p1, Boxed_POD_Value r)
{
  if (r.m_isfloat)
  {
    return (p1 += r.d);
  } else {
    return (p1 += r.i);
  } 
}

template<typename P1, typename P2>
P1 &subtractsequal(P1 &p1, const P2 &p2)
{
  return (p1 -= p2);
}

template<typename P1>
P1 &subtractsequal_pod(P1 &p1, Boxed_POD_Value r)
{
  if (r.m_isfloat)
  {
    return (p1 -= r.d);
  } else {
    return (p1 -= r.i);
  } 
}

template<typename P1>
P1 &prefixincrement(P1 &p1)
{
  return (++p1);
}

template<typename P1>
P1 &prefixdecrement(P1 &p1)
{
  return (--p1);
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

template<typename T, typename U>
void add_oper_assign_overload(BoxedCPP_System &s)
{
  register_function(s, &assign<T,U>, "=");
}


template<typename T>
void add_oper_assign(BoxedCPP_System &s)
{
  register_function(s, &assign<T,T>, "=");
}


template<typename T>
void add_oper_assign_pod(BoxedCPP_System &s)
{
  register_function(s, &assign_pod<T>, "=");
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

void add_opers_comparison_pod(BoxedCPP_System &s)
{
  register_function(s, &pod_equals, "==");
  register_function(s, &pod_not_equals, "!=");
  register_function(s, &pod_less_than, "<");
  register_function(s, &pod_greater_than, ">");
  register_function(s, &pod_less_than_equals, "<=");
  register_function(s, &pod_greater_than_equals, ">=");
}

void add_opers_arithmetic_pod(BoxedCPP_System &s)
{
  register_function(s, &pod_add, "+");
  register_function(s, &pod_subtract, "-");
  register_function(s, &pod_divide, "/");
  register_function(s, &pod_multiply, "*");
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
  register_function(s, &dividesequal<T, R>, "/=");
  register_function(s, &subtractsequal<T, R>, "-=");
  register_function(s, &addsequal<T, R>, "+=");
  register_function(s, &prefixincrement<T>, "++");
  register_function(s, &prefixdecrement<T>, "--");
}

template<typename T>
void add_opers_arithmetic_modify_pod(BoxedCPP_System &s)
{
  register_function(s, &timesequal_pod<T>, "*=");
  register_function(s, &dividesequal_pod<T>, "/=");
  register_function(s, &subtractsequal_pod<T>, "-=");
  register_function(s, &addsequal_pod<T>, "+=");
}

template<typename T>
void add_basic_constructors(BoxedCPP_System &s, const std::string &type)
{
  s.register_function(build_constructor<T>(), type);
  s.register_function(build_constructor<T, const T &>(), type);
  s.register_function(build_constructor<T, const T &>(), "clone");
}

template<typename T, typename U>
void add_constructor_overload(BoxedCPP_System &s, const std::string &type)
{
  s.register_function(build_constructor<T, const U &>(), type);
}

template<typename T>
void add_opers_arithmetic(BoxedCPP_System &s)
{
  add_opers_arithmetic_overload<T, T, T>(s);

}

class bad_boxed_value_cast : public std::bad_cast
{
  public:
    bad_boxed_value_cast(const std::string &val) throw()
      : m_val(val)
    {
    }

    virtual ~bad_boxed_value_cast() throw()
    {
    }

    virtual const char * what() const throw()
    {
      return m_val.c_str();
    }

  private:
    std::string m_val;
};

Boxed_Value unknown_assign(Boxed_Value lhs, Boxed_Value rhs)
{
  if (lhs.is_unknown())
  {
    return (lhs.assign(rhs));
  } else {
    throw bad_boxed_value_cast("boxed_value has a set type alread");
  }
}

//Built in to_string operator
template<typename Input>
std::string to_string(Input i)
{
  return boost::lexical_cast<std::string>(i);
}

std::string to_string(bool b)
{
  if (b)
  {
    return "true";
  } else {
    return "false";
  }
}

template<typename T>
void bootstrap_pod_type(BoxedCPP_System &s, const std::string &name)
{
  s.register_type<T>(name);
  add_basic_constructors<T>(s, name);
  add_oper_assign<T>(s);
  add_oper_assign_pod<T>(s);
  add_opers_arithmetic<T>(s);
  add_opers_arithmetic_modify_pod<T>(s);
  register_function(s, &to_string<T>, "to_string");
}

void bootstrap(BoxedCPP_System &s)
{
  s.register_type<void>("void");

  s.register_type<std::string>("string");

  add_basic_constructors<bool>(s, "bool");
  add_basic_constructors<std::string>(s, "string");
  add_oper_assign<std::string>(s);

  register_function(s, &to_string<const std::string &>, "to_string");
  register_function(s, &to_string<bool>, "to_string");
  register_function(s, &unknown_assign, "=");

  bootstrap_pod_type<double>(s, "double");
  bootstrap_pod_type<int>(s, "int");
  bootstrap_pod_type<size_t>(s, "size_t");
  bootstrap_pod_type<char>(s, "char");
  bootstrap_pod_type<int64_t>(s, "int64_t");



  add_opers_comparison_pod(s);
  add_opers_arithmetic_pod(s);

  add_oper_add<std::string>(s);

  register_function(s, &bool_and<bool, bool>, "&&");
  register_function(s, &bool_or<bool, bool>, "||");

}

#endif
