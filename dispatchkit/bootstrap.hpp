#ifndef __bootstrap_hpp
#define __bootstrap_hpp__

#include "dispatchkit.hpp"
#include "register_function.hpp"

namespace dispatchkit
{
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

  template<typename P1>
  P1 &prefixnegate(P1 &p1)
  {
    return (p1);
  }

  template<typename P1>
  P1 &prefixnot(P1 &p1)
  {
    return (p1);
  }


  //Add canonical forms of operators
  template<typename T>
  void add_oper_equals(Dispatch_Engine &s)
  {
    register_function(s, &equals<const T&, const T&>, "=");
  }

  template<typename T>
  void add_oper_add(Dispatch_Engine &s)
  {
    register_function(s, &add<T, const T&, const T&>, "+");
  }

  template<typename T>
  void add_oper_add_equals(Dispatch_Engine &s)
  {
    register_function(s, &addsequal<T, T>, "+=");
  }

  template<typename T>
  void add_oper_subtract(Dispatch_Engine &s)
  {
    register_function(s, &subtract<T, const T&, const T&>, "-");
  }

  template<typename T>
  void add_oper_divide(Dispatch_Engine &s)
  {
    register_function(s, &divide<T, const T&, const T&>, "-");
  }

  template<typename T>
  void add_oper_multiply(Dispatch_Engine &s)
  {
    register_function(s, &multiply<T, const T&, const T&>, "*");
  }

  template<typename T>
  void add_oper_not_equals(Dispatch_Engine &s)
  {
    register_function(s, &not_equals<const T&, const T&>, "!=");
  }

  template<typename T, typename U>
  void add_oper_assign_overload(Dispatch_Engine &s)
  {
    register_function(s, &assign<T,U>, "=");
  }


  template<typename T>
  void add_oper_assign(Dispatch_Engine &s)
  {
    register_function(s, &assign<T,T>, "=");
  }


  template<typename T>
  void add_oper_assign_pod(Dispatch_Engine &s)
  {
    register_function(s, &assign_pod<T>, "=");
  }


  template<typename T>
  void add_oper_less_than(Dispatch_Engine &s)
  {
    register_function(s, &less_than<const T&, const T&>, "<");
  }

  template<typename T>
  void add_oper_greater_than(Dispatch_Engine &s)
  {
    register_function(s, &greater_than<const T&, const T&>, ">");
  }

  template<typename T>
  void add_oper_less_than_equals(Dispatch_Engine &s)
  {
    register_function(s, &less_than_equals<const T&, const T&>, "<=");
  }

  template<typename T>
  void add_oper_greater_than_equals(Dispatch_Engine &s)
  {
    register_function(s, &greater_than_equals<const T&, const T&>, ">=");
  }


  template<typename T, typename R>
  void add_opers_comparison_overload(Dispatch_Engine &s)
  {
    register_function(s, &equals<const T&, const R&>, "==");
    register_function(s, &not_equals<const T&, const R&>, "!=");
    register_function(s, &less_than<const T&, const R&>, "<");
    register_function(s, &greater_than<const T&, const R&>, ">");
    register_function(s, &less_than_equals<const T&, const R&>, "<=");
    register_function(s, &greater_than_equals<const T&, const R&>, ">=");
  }

  template<typename T>
  void add_opers_comparison(Dispatch_Engine &s)
  {
    add_opers_comparison_overload<T, T>(s);
  }

  template<typename Ret, typename T, typename R>
  void add_opers_arithmetic_overload(Dispatch_Engine &s)
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
    register_function(s, &prefixnegate<T>, "-");
    register_function(s, &prefixnot<T>, "!");
  }

  template<typename T>
  void add_opers_arithmetic_modify_pod(Dispatch_Engine &s)
  {
    register_function(s, &timesequal_pod<T>, "*=");
    register_function(s, &dividesequal_pod<T>, "/=");
    register_function(s, &subtractsequal_pod<T>, "-=");
    register_function(s, &addsequal_pod<T>, "+=");
  }

  template<typename T>
    void add_copy_constructor(Dispatch_Engine &s, const std::string &type)
    {
      s.register_function(build_constructor<T, const T &>(), type);
      s.register_function(build_constructor<T, const T &>(), "clone");
    }

  template<typename T>
  void add_basic_constructors(Dispatch_Engine &s, const std::string &type)
  {
    s.register_function(build_constructor<T>(), type);
    add_copy_constructor<T>(s, type);
  }

  template<typename T, typename U>
  void add_constructor_overload(Dispatch_Engine &s, const std::string &type)
  {
    s.register_function(build_constructor<T, const U &>(), type);
  }

  template<typename T>
  void add_opers_arithmetic(Dispatch_Engine &s)
  {
    add_opers_arithmetic_overload<T, T, T>(s);

  }



  //Built in to_string operator
  template<typename Input>
  std::string to_string(Input i)
  {
    return boost::lexical_cast<std::string>(i);
  }

  template<> std::string to_string(bool b)
  {
    if (b)
    {
      return "true";
    } else {
      return "false";
    }
  }

  template<typename Input>
  Input parse_string(const std::string &i)
  {
    return boost::lexical_cast<Input>(i);
  }



  template<typename T>
  void bootstrap_pod_type(Dispatch_Engine &s, const std::string &name)
  {
    s.register_type<T>(name);
    add_basic_constructors<T>(s, name);
    add_oper_assign<T>(s);
    add_oper_assign_pod<T>(s);
    add_opers_arithmetic<T>(s);
    add_opers_arithmetic_modify_pod<T>(s);
    register_function(s, &to_string<T>, "to_string");
    register_function(s, &parse_string<T>, "to_" + name);
  }

  template<typename Type>
    boost::shared_ptr<Type> shared_ptr_clone(boost::shared_ptr<Type> f)
    {
      return f;
    }

  template<typename Type>
    Boxed_Value ptr_assign(Boxed_Value lhs, boost::shared_ptr<Type> rhs)
    {
      lhs.assign(Boxed_Value(rhs));

      return lhs;
    }


  struct Bootstrap
  {
    static Boxed_Value unknown_assign(Boxed_Value lhs, Boxed_Value rhs)
    {
      if (lhs.is_unknown())
      {
        return (lhs.assign(rhs));
      } else {
        throw bad_boxed_cast("boxed_value has a set type already");
      }
    }

    static void print(const std::string &s)
    {
      std::cout << s;
    }

    static void println(const std::string &s)
    {
      std::cout << s << std::endl;
    }

    static void add_opers_comparison_pod(Dispatch_Engine &s)
    {
      register_function(s, &equals<Boxed_POD_Value, Boxed_POD_Value>, "==");
      register_function(s, &not_equals<Boxed_POD_Value, Boxed_POD_Value>, "!=");
      register_function(s, &less_than<Boxed_POD_Value, Boxed_POD_Value>, "<");
      register_function(s, &greater_than<Boxed_POD_Value, Boxed_POD_Value>, ">");
      register_function(s, &less_than_equals<Boxed_POD_Value, Boxed_POD_Value>, "<=");
      register_function(s, &greater_than_equals<Boxed_POD_Value, Boxed_POD_Value>, ">=");
    }

    static void add_opers_arithmetic_pod(Dispatch_Engine &s)
    {
      register_function(s, &add<Boxed_Value, Boxed_POD_Value, Boxed_POD_Value>, "+");
      register_function(s, &subtract<Boxed_Value, Boxed_POD_Value, Boxed_POD_Value>, "-");
      register_function(s, &divide<Boxed_Value, Boxed_POD_Value, Boxed_POD_Value>, "/");
      register_function(s, &multiply<Boxed_Value, Boxed_POD_Value, Boxed_POD_Value>, "*");
    }

    static bool type_match(Boxed_Value l, Boxed_Value r)
    {
      return l.get_type_info() == r.get_type_info();
    }

    static Boxed_Value bind_function(const std::vector<Boxed_Value> &params)
    {
      if (params.size() < 2)
      {
        throw arity_error(params.size(), 2);
      }

      boost::shared_ptr<Proxy_Function> f = boxed_cast<boost::shared_ptr<Proxy_Function> >(params[0]);

      return Boxed_Value(boost::shared_ptr<Proxy_Function>(new Bound_Function(f,
            std::vector<Boxed_Value>(params.begin() + 1, params.end()))));
    }

    static Boxed_Value call_exists(const std::vector<Boxed_Value> &params)
    {
      if (params.size() < 1)
      {
        throw arity_error(params.size(), 1);
      }

      boost::shared_ptr<Proxy_Function> f = boxed_cast<boost::shared_ptr<Proxy_Function> >(params[0]);

      return Boxed_Value(f->types_match(std::vector<Boxed_Value>(params.begin() + 1, params.end())));
    }

    static void bootstrap(Dispatch_Engine &s)
    {
      s.register_type<void>("void");

      s.register_type<std::string>("string");

      add_basic_constructors<bool>(s, "bool");
      add_basic_constructors<std::string>(s, "string");
      add_oper_assign<std::string>(s);

      
      register_function(s, &to_string<const std::string &>, "internal_to_string");
      register_function(s, &to_string<bool>, "internal_to_string");
      register_function(s, &unknown_assign, "=");

      bootstrap_pod_type<double>(s, "double");
      bootstrap_pod_type<int>(s, "int");
      bootstrap_pod_type<size_t>(s, "size_t");
      bootstrap_pod_type<char>(s, "char");
      bootstrap_pod_type<int64_t>(s, "int64_t");


      add_opers_comparison_pod(s);
      add_opers_arithmetic_pod(s);

      add_oper_add<std::string>(s);
      add_oper_add_equals <std::string>(s);
      add_opers_comparison<std::string>(s);

      register_function(s, &print, "print_string");
      register_function(s, &println, "println_string");

      s.register_function(boost::function<void ()>(boost::bind(&dump_system, boost::ref(s))), "dump_system");
      s.register_function(boost::function<void (Boxed_Value)>(boost::bind(&dump_object, _1)), "dump_object");

      s.add_object("_", Placeholder_Object());

      s.register_function(boost::shared_ptr<Proxy_Function>(new Dynamic_Proxy_Function(boost::bind(&bind_function, _1))), 
          "bind");

      register_function(s, &shared_ptr_clone<Proxy_Function>, "clone");
      register_function(s, &ptr_assign<Proxy_Function>, "=");

      s.register_function(boost::shared_ptr<Proxy_Function>(new Dynamic_Proxy_Function(boost::bind(&call_exists, _1))), 
          "call_exists");

      register_function(s, &type_match, "type_match");
    }
  };
}

#endif

