// This file is distributed under the BSD License.
// See "license.txt" for details.
// Copyright 2009, Jonathan Turner (jturner@minnow-lang.org)
// and Jason Turner (lefticus@gmail.com)
// http://www.chaiscript.com

#ifndef __bootstrap_hpp
#define __bootstrap_hpp__

#include "dispatchkit.hpp"
#include "register_function.hpp"

namespace chaiscript 
{
  /**
   * Set of helper functions for common operators
   */
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

  template<typename Ret, typename P1, typename P2>
  Ret modulus(P1 p1, P2 p2)
  {
    return p1 % p2;
  }

  template<typename P1, typename P2>
  P1 &assign(P1 &p1, const P2 &p2)
  {
    return (p1 = p2);
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

  template<typename P1, typename P2>
  P1 &dividesequal(P1 &p1, const P2 &p2)
  {
    return (p1 /= p2);
  }

  template<typename P1, typename P2>
  P1 &addsequal(P1 &p1, const P2 &p2)
  {
    return (p1 += p2);
  }

  template<typename P1, typename P2>
  P1 &subtractsequal(P1 &p1, const P2 &p2)
  {
    return (p1 -= p2);
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

  /* Special helpers for generating generic "POD" type operators
   * The POD operators are needed for general support of C++ POD
   * types without iterating out all possible combinations of operators
   * (<, >, +, +=, *=, \=, -, <=, >=, ==) and types
   * (char, uint8_t, int8_t, uint16_t, int16_t...)
   */
  template<typename P1>
    P1 &assign_pod(P1 &p1, Boxed_POD_Value v)
    {
      if (v.m_isfloat)
      {
        return (p1 = P1(v.d));
      } else {
        return (p1 = P1(v.i));
      }
    }

  template<typename P1>
    P1 construct_pod(Boxed_POD_Value v)
    {
      if (v.m_isfloat)
      {
        return P1(v.d);
      } else {
        return P1(v.i);
      }    
    }

  template<typename P1>
    P1 &timesequal_pod(P1 &p1, Boxed_POD_Value r)
    {
      if (r.m_isfloat)
      {
        return p1 *= P1(r.d);
      } else {
        return p1 *= P1(r.i);
      }
    }

  template<typename P1>
    P1 &dividesequal_pod(P1 &p1, Boxed_POD_Value r)
    {
      if (r.m_isfloat)
      {
        return p1 /= P1(r.d);
      } else {
        return p1 /= P1(r.i);
      }
    }

  template<typename P1>
    P1 &addsequal_pod(P1 &p1, Boxed_POD_Value r)
    {
      if (r.m_isfloat)
      {
        return p1 += P1(r.d);
      } else {
        return p1 += P1(r.i);
      }
    }

  template<typename P1>
    P1 &subtractsequal_pod(P1 &p1, Boxed_POD_Value r)
    {
      if (r.m_isfloat)
      {
        return p1 -= P1(r.d);
      } else {
        return p1 -= P1(r.i);
      }
    }


  /**
   * Add canonical form of "=" for type T
   */
  template<typename T>
  void add_oper_equals(Dispatch_Engine &s)
  {
    s.add(fun(&equals<const T&, const T&>), "=");
  }

  /**
   * Add canonical form of "+" for type T
   */
  template<typename T>
  void add_oper_add(Dispatch_Engine &s)
  {
    s.add(fun(&add<T, const T&, const T&>), "+");
  }

  /**
   * Add canonical form of "+=" for type T
   */
  template<typename T>
  void add_oper_add_equals(Dispatch_Engine &s)
  {
    s.add(fun(&addsequal<T, T>), "+=");
  }

  /**
   * Add canonical form of "-" for type T
   */
  template<typename T>
  void add_oper_subtract(Dispatch_Engine &s)
  {
    s.add(fun(&subtract<T, const T&, const T&>), "-");
  }

  /**
   * Add canonical form of "/" for type T
   */
  template<typename T>
  void add_oper_divide(Dispatch_Engine &s)
  {
    s.add(fun(&divide<T, const T&, const T&>), "/");
  }

  /**
   * Add canonical form of "*" for type T
   */
  template<typename T>
  void add_oper_multiply(Dispatch_Engine &s)
  {
    s.add(fun(&multiply<T, const T&, const T&>), "*");
  }

  /**
   * Add canonical form of "!=" for type T
   */
  template<typename T>
  void add_oper_not_equals(Dispatch_Engine &s)
  {
    s.add(fun(&not_equals<const T&, const T&>), "!=");
  }

  /**
   * Add user defined assignment operator for T = U
   */
  template<typename T, typename U>
  void add_oper_assign_overload(Dispatch_Engine &s)
  {
    s.add(fun(&assign<T,U>), "=");
  }


  /**
   * Add canonical form of "=" for type T
   */
  template<typename T>
  void add_oper_assign(Dispatch_Engine &s)
  {
    s.add(fun(&assign<T,T>), "=");
  }


  /**
   * Add assignment operator for T = POD.
   */
  template<typename T>
  void add_oper_assign_pod(Dispatch_Engine &s)
  {
    s.add(fun(&assign_pod<T>), "=");
  }


  /**
   * Add canonical form of "<" for type T
   */
  template<typename T>
  void add_oper_less_than(Dispatch_Engine &s)
  {
    s.add(fun(&less_than<const T&, const T&>), "<");
  }

  /**
   * Add canonical form of ">" for type T
   */
  template<typename T>
  void add_oper_greater_than(Dispatch_Engine &s)
  {
    s.add(fun(&greater_than<const T&, const T&>), ">");
  }

  /**
   * Add canonical form of "<=" for type T
   */
  template<typename T>
  void add_oper_less_than_equals(Dispatch_Engine &s)
  {
    s.add(fun(&less_than_equals<const T&, const T&>), "<=");
  }

  /**
   * Add canonical form of ">=" for type T
   */
  template<typename T>
  void add_oper_greater_than_equals(Dispatch_Engine &s)
  {
    s.add(fun(&greater_than_equals<const T&, const T&>), ">=");
  }

  /**
   * Add user defined comparison operators for T and R.
   * Examples: T < R, T == R, etc.
   */
  template<typename T, typename R>
  void add_opers_comparison_overload(Dispatch_Engine &s)
  {
    s.add(fun(&equals<const T&, const R&>), "==");
    s.add(fun(&not_equals<const T&, const R&>), "!=");
    s.add(fun(&less_than<const T&, const R&>), "<");
    s.add(fun(&greater_than<const T&, const R&>), ">");
    s.add(fun(&less_than_equals<const T&, const R&>), "<=");
    s.add(fun(&greater_than_equals<const T&, const R&>), ">=");
  }

  /**
   * Add canonical forms of all comparison operators for type T
   */
  template<typename T>
  void add_opers_comparison(Dispatch_Engine &s)
  {
    add_opers_comparison_overload<T, T>(s);
  }

  /**
   * Add all arithmetic operators that return a type of Ret, taking
   * a lhs of T and a rhs of R, when possible.
   * examples: Ret = T + R;
   * ++T
   * T *= R;
   */
  template<typename Ret, typename T, typename R>
  void add_opers_arithmetic_overload(Dispatch_Engine &s)
  {
    s.add(fun(&add<Ret, T, R>), "+");
    s.add(fun(&subtract<Ret, T, R>), "-");
    s.add(fun(&divide<Ret, T, R>), "/");
    s.add(fun(&multiply<Ret, T, R>), "*");
    s.add(fun(&timesequal<T, R>), "*=");
    s.add(fun(&dividesequal<T, R>), "/=");
    s.add(fun(&subtractsequal<T, R>), "-=");
    s.add(fun(&addsequal<T, R>), "+=");
    s.add(fun(&prefixincrement<T>), "++");
    s.add(fun(&prefixdecrement<T>), "--");
    s.add(fun(&prefixnegate<T>), "-");
    s.add(fun(&prefixnot<T>), "!");
  }

  /**
   * Add arithmetic assign operators for POD types:
   * example: POD *= T, POD /= T
   */
  template<typename T>
  void add_opers_arithmetic_modify_pod(Dispatch_Engine &s)
  {
    s.add(fun(&timesequal_pod<T>), "*=");
    s.add(fun(&dividesequal_pod<T>), "/=");
    s.add(fun(&subtractsequal_pod<T>), "-=");
    s.add(fun(&addsequal_pod<T>), "+=");
  }

  /**
   * Add a copy constructor for type T, also creates the standard
   * function "clone" for the type. "clone" is a synonym for
   * the copy constructor.
   */
  template<typename T>
    void add_copy_constructor(Dispatch_Engine &s, const std::string &type)
    {
      s.add(constructor<T (const T &)>(), type);
      s.add(constructor<T (const T &)>(), "clone");
    }

  /**
   * Add default and copy constructors (including "clone") for type T
   */
  template<typename T>
  void add_basic_constructors(Dispatch_Engine &s, const std::string &type)
  {
    s.add(constructor<T ()>(), type);
    add_copy_constructor<T>(s, type);
  }

  /**
   * Add POD type constructor for type T. ie: T = type(POD)
   */
  template<typename T>
  void add_construct_pod(Dispatch_Engine &s, const std::string &type)
  {
    s.add(fun(&construct_pod<T>), type);
  }

  /**
   * add user defined single parameter constructor for type T.
   * T = type(const U &)
   */
  template<typename T, typename U>
  void add_constructor_overload(Dispatch_Engine &s, const std::string &type)
  {
    s.add(constructor<T (const U &)>(), type);
  }

  /**
   * Add canonical forms of all arithmetic operators for type T
   */
  template<typename T>
  void add_opers_arithmetic(Dispatch_Engine &s)
  {
    add_opers_arithmetic_overload<T, T, T>(s);

  }

  /**
   * to_string function for internal use. Uses ostream operator<<
   */
  template<typename Input>
  std::string to_string(Input i)
  {
    return boost::lexical_cast<std::string>(i);
  }

  /**
   * Boolean specialization of internal to_string function 
   */
  template<> std::string to_string(bool b)
  {
    if (b)
    {
      return "true";
    } else {
      return "false";
    }
  }

  /**
   * Internal function for converting from a string to a value
   * uses ostream operator >> to perform the conversion
   */
  template<typename Input>
  Input parse_string(const std::string &i)
  {
    return boost::lexical_cast<Input>(i);
  }

  /**
   * Add all common functions for a POD type. All operators, and
   * common conversions
   */
  template<typename T>
  void bootstrap_pod_type(Dispatch_Engine &s, const std::string &name)
  {
    s.add(user_type<T>(), name);
    add_basic_constructors<T>(s, name);
    add_oper_assign<T>(s);
    add_oper_assign_pod<T>(s);
    add_construct_pod<T>(s, name);
    add_opers_arithmetic<T>(s);
    add_opers_arithmetic_modify_pod<T>(s);
    s.add(fun(&to_string<T>), "to_string");
    s.add(fun(&parse_string<T>), "to_" + name);
  }

  /**
   * "clone" function for a shared_ptr type. This is used in the case
   * where you do not want to make a deep copy of an object during cloning
   * but want to instead maintain the shared_ptr. It is needed internally
   * for handling of Proxy_Function object (that is,
   * function variables.
   */
  template<typename Type>
    boost::shared_ptr<Type> shared_ptr_clone(boost::shared_ptr<Type> f)
    {
      return f;
    }

  /**
   * Assignment function for shared_ptr objects, does not perform a copy of the
   * object pointed to, instead maintains the shared_ptr concept.
   * Similar to shared_ptr_clone. Used for Proxy_Function.
   */
  template<typename Type>
    Boxed_Value ptr_assign(Boxed_Value lhs, boost::shared_ptr<Type> rhs)
    {
      lhs.assign(Boxed_Value(rhs));

      return lhs;
    }

  /**
   * Class consisting of only static functions. All default bootstrapping occurs
   * from this class.
   */
  struct Bootstrap
  {
    /**
     * Function allowing for assignment of an unknown type to any other value
     */
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

    /**
     * Add all comparison operators for POD types
     */
    static void add_opers_comparison_pod(Dispatch_Engine &s)
    {
      s.add(fun(&equals<Boxed_POD_Value, Boxed_POD_Value>), "==");
      s.add(fun(&not_equals<Boxed_POD_Value, Boxed_POD_Value>), "!=");
      s.add(fun(&less_than<Boxed_POD_Value, Boxed_POD_Value>), "<");
      s.add(fun(&greater_than<Boxed_POD_Value, Boxed_POD_Value>), ">");
      s.add(fun(&less_than_equals<Boxed_POD_Value, Boxed_POD_Value>), "<=");
      s.add(fun(&greater_than_equals<Boxed_POD_Value, Boxed_POD_Value>), ">=");
    }

    /**
     * Add all arithmetic operators for PODs
     */
    static void add_opers_arithmetic_pod(Dispatch_Engine &s)
    {
      s.add(fun(&add<Boxed_Value, Boxed_POD_Value, Boxed_POD_Value>), "+");
      s.add(fun(&subtract<Boxed_Value, Boxed_POD_Value, Boxed_POD_Value>), "-");
      s.add(fun(&divide<Boxed_Value, Boxed_POD_Value, Boxed_POD_Value>), "/");
      s.add(fun(&multiply<Boxed_Value, Boxed_POD_Value, Boxed_POD_Value>), "*");
    }

    /**
     * Return true if the two Boxed_Value's share the same internal type
     */
    static bool type_match(Boxed_Value l, Boxed_Value r)
    {
      return l.get_type_info() == r.get_type_info();
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

    /**
     * Create a bound function object. The first param is the function to bind
     * the remaining parameters are the args to bind into the
     * result
     */
    static Boxed_Value bind_function(const std::vector<Boxed_Value> &params)
    {
      if (params.size() < 2)
      {
        throw arity_error(params.size(), 2);
      }

      Proxy_Function f = boxed_cast<Proxy_Function >(params[0]);

      return Boxed_Value(Proxy_Function(new Bound_Function(f,
            std::vector<Boxed_Value>(params.begin() + 1, params.end()))));
    }

    /**
     * Returns true if a call can be made that consists of the first parameter
     * (the function) with the remaining parameters as its arguments.
     */
    static Boxed_Value call_exists(const std::vector<Boxed_Value> &params)
    {
      if (params.size() < 1)
      {
        throw arity_error(params.size(), 1);
      }

      Proxy_Function f = boxed_cast<Proxy_Function >(params[0]);

      return Boxed_Value(f->types_match(std::vector<Boxed_Value>(params.begin() + 1, params.end())));
    }

    /**
     * perform all common bootstrap functions for std::string, void and POD types
     */
    static void bootstrap(Dispatch_Engine &s)
    {
      s.add(user_type<void>(), "void");
      s.add(user_type<bool>(), "bool");
      s.add(user_type<Boxed_Value>(), "Object");
      s.add(user_type<Boxed_POD_Value>(), "PODObject");
      s.add(user_type<Proxy_Function>(), "function");

      add_basic_constructors<bool>(s, "bool");
      add_oper_assign<std::string>(s);
      add_oper_assign<bool>(s);

      s.add(fun(&to_string<const std::string &>), "internal_to_string");
      s.add(fun(&to_string<bool>), "internal_to_string");
      s.add(fun(&unknown_assign), "=");

      bootstrap_pod_type<double>(s, "double");
      bootstrap_pod_type<int>(s, "int");
      bootstrap_pod_type<size_t>(s, "size_t");
      bootstrap_pod_type<char>(s, "char");
      bootstrap_pod_type<boost::int64_t>(s, "int64_t");

      add_opers_comparison_pod(s);
      add_opers_arithmetic_pod(s);
      s.add(fun(&modulus<int, int, int>), "%");

      s.add(fun(&print), "print_string");
      s.add(fun(&println), "println_string");

      s.add(fun(boost::function<void ()>(boost::bind(&dump_system, boost::ref(s)))), "dump_system");
      s.add(fun(boost::function<void (Boxed_Value)>(boost::bind(&dump_object, _1, boost::ref(s)))), "dump_object");
      s.add(fun(boost::function<bool (Boxed_Value, const std::string &)>(boost::bind(&is_type, boost::ref(s), _2, _1))),
          "is_type");

      s.add(Proxy_Function(new Dynamic_Proxy_Function(boost::bind(&bind_function, _1))), 
          "bind");

      s.add(fun(&shared_ptr_clone<Proxy_Function_Base>), "clone");
      s.add(fun(&ptr_assign<Proxy_Function_Base>), "=");

      s.add(Proxy_Function(new Dynamic_Proxy_Function(boost::bind(&call_exists, _1))), 
          "call_exists");

      s.add(fun(&type_match), "type_match");
    }
  };
}

#endif

