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
  namespace bootstrap
  {
    namespace detail
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
    }

    /**
    * Add canonical form of "=" for type T
    */
    template<typename T>
    ModulePtr oper_equals(ModulePtr m = ModulePtr(new Module()))
    {
      m->add(fun(&detail::equals<const T&, const T&>), "=");
      return m;
    }

    /**
    * Add canonical form of "+" for type T
    */
    template<typename T>
    ModulePtr oper_add(ModulePtr m = ModulePtr(new Module()))
    {
      m->add(fun(&detail::add<T, const T&, const T&>), "+");
      return m;
    }

    /**
    * Add canonical form of "+=" for type T
    */
    template<typename T>
    ModulePtr oper_add_equals(ModulePtr m = ModulePtr(new Module()))
    {
      m->add(fun(&detail::addsequal<T, T>), "+=");
      return m;
    }

    /**
    * Add canonical form of "-" for type T
    */
    template<typename T>
    ModulePtr oper_subtract(ModulePtr m = ModulePtr(new Module()))
    {
      m->add(fun(&detail::subtract<T, const T&, const T&>), "-");
      return m;
    }

    /**
    * Add canonical form of "/" for type T
    */
    template<typename T>
    ModulePtr oper_divide(ModulePtr m = ModulePtr(new Module()))
    {
      m->add(fun(&detail::divide<T, const T&, const T&>), "/");
      return m;
    }

    /**
    * Add canonical form of "*" for type T
    */
    template<typename T>
    ModulePtr oper_multiply(ModulePtr m = ModulePtr(new Module()))
    {
      m->add(fun(&detail::multiply<T, const T&, const T&>), "*");
      return m;
    }

    /**
    * Add canonical form of "!=" for type T
    */
    template<typename T>
    ModulePtr oper_not_equals(ModulePtr m = ModulePtr(new Module()))
    {
      m->add(fun(&detail::not_equals<const T&, const T&>), "!=");
      return m;
    }

    /**
    * Add user defined assignment operator for T = U
    */
    template<typename T, typename U>
    ModulePtr oper_assign_overload(ModulePtr m = ModulePtr(new Module()))
    {
      m->add(fun(&detail::assign<T,U>), "=");
      return m;
    }


    /**
    * Add canonical form of "=" for type T
    */
    template<typename T>
    ModulePtr oper_assign(ModulePtr m = ModulePtr(new Module()))
    {
      m->add(fun(&detail::assign<T,T>), "=");
      return m;
    }


    /**
    * Add assignment operator for T = POD.
    */
    template<typename T>
    ModulePtr oper_assign_pod(ModulePtr m = ModulePtr(new Module()))
    {
      m->add(fun(&detail::assign_pod<T>), "=");
      return m;
    }


    /**
    * Add canonical form of "<" for type T
    */
    template<typename T>
    ModulePtr oper_less_than(ModulePtr m = ModulePtr(new Module()))
    {
      m->add(fun(&detail::less_than<const T&, const T&>), "<");
      return m;
    }

    /**
    * Add canonical form of ">" for type T
    */
    template<typename T>
    ModulePtr oper_greater_than(ModulePtr m = ModulePtr(new Module()))
    {
      m->add(fun(&detail::greater_than<const T&, const T&>), ">");
      return m;
    }

    /**
    * Add canonical form of "<=" for type T
    */
    template<typename T>
    ModulePtr oper_less_than_equals(ModulePtr m = ModulePtr(new Module()))
    {
      m->add(fun(&detail::less_than_equals<const T&, const T&>), "<=");
      return m;
    }

    /**
    * Add canonical form of ">=" for type T
    */
    template<typename T>
    ModulePtr oper_greater_than_equals(ModulePtr m = ModulePtr(new Module()))
    {
      m->add(fun(&detail::greater_than_equals<const T&, const T&>), ">=");
      return m;
    }

    /**
    * Add user defined comparison operators for T and R.
    * Examples: T < R, T == R, etc.
    */
    template<typename T, typename R>
    ModulePtr opers_comparison_overload(ModulePtr m = ModulePtr(new Module()))
    {
      m->add(fun(&detail::equals<const T&, const R&>), "==");
      m->add(fun(&detail::not_equals<const T&, const R&>), "!=");
      m->add(fun(&detail::less_than<const T&, const R&>), "<");
      m->add(fun(&detail::greater_than<const T&, const R&>), ">");
      m->add(fun(&detail::less_than_equals<const T&, const R&>), "<=");
      m->add(fun(&detail::greater_than_equals<const T&, const R&>), ">=");
      return m;
    }

    /**
    * Add canonical forms of all comparison operators for type T
    */
    template<typename T>
    ModulePtr opers_comparison(ModulePtr m = ModulePtr(new Module()))
    {
      opers_comparison_overload<T, T>(m);
      return m;
    }

    /**
    * Add all arithmetic operators that return a type of Ret, taking
    * a lhs of T and a rhs of R, when possible.
    * examples: Ret = T + R;
    * ++T
    * T *= R;
    */
    template<typename Ret, typename T, typename R>
    ModulePtr opers_arithmetic_overload(ModulePtr m = ModulePtr(new Module()))
    {
      m->add(fun(&detail::add<Ret, T, R>), "+");
      m->add(fun(&detail::subtract<Ret, T, R>), "-");
      m->add(fun(&detail::divide<Ret, T, R>), "/");
      m->add(fun(&detail::multiply<Ret, T, R>), "*");
      m->add(fun(&detail::timesequal<T, R>), "*=");
      m->add(fun(&detail::dividesequal<T, R>), "/=");
      m->add(fun(&detail::subtractsequal<T, R>), "-=");
      m->add(fun(&detail::addsequal<T, R>), "+=");
      m->add(fun(&detail::prefixincrement<T>), "++");
      m->add(fun(&detail::prefixdecrement<T>), "--");
      m->add(fun(&detail::prefixnegate<T>), "-");
      m->add(fun(&detail::prefixnot<T>), "!");
      return m;
    }

    /**
    * Add arithmetic assign operators for POD types:
    * example: POD *= T, POD /= T
    */
    template<typename T>
    ModulePtr opers_arithmetic_modify_pod(ModulePtr m = ModulePtr(new Module()))
    {
      m->add(fun(&detail::timesequal_pod<T>), "*=");
      m->add(fun(&detail::dividesequal_pod<T>), "/=");
      m->add(fun(&detail::subtractsequal_pod<T>), "-=");
      m->add(fun(&detail::addsequal_pod<T>), "+=");
      return m;
    }

    /**
    * Add a copy constructor for type T, also creates the standard
    * function "clone" for the type. "clone" is a synonym for
    * the copy constructor.
    */
    template<typename T>
    ModulePtr copy_constructor(const std::string &type, ModulePtr m = ModulePtr(new Module()))
    {
      m->add(constructor<T (const T &)>(), type);
      m->add(constructor<T (const T &)>(), "clone");
      return m;
    }

    /**
    * Add default and copy constructors (including "clone") for type T
    */
    template<typename T>
    ModulePtr basic_constructors(const std::string &type, ModulePtr m = ModulePtr(new Module()))
    {
      m->add(constructor<T ()>(), type);
      copy_constructor<T>(type, m);
      return m;
    }

    /**
    * Add POD type constructor for type T. ie: T = type(POD)
    */
    template<typename T>
    ModulePtr construct_pod(const std::string &type, ModulePtr m = ModulePtr(new Module()))
    {
      m->add(fun(&detail::construct_pod<T>), type);
      return m;
    }

    /**
    * add user defined single parameter constructor for type T.
    * T = type(const U &)
    */
    template<typename T, typename U>
    ModulePtr constructor_overload(const std::string &type, ModulePtr m = ModulePtr(new Module()))
    {
      m->add(constructor<T (const U &)>(), type);
      return m;
    }

    /**
    * Add canonical forms of all arithmetic operators for type T
    */
    template<typename T>
    ModulePtr opers_arithmetic(ModulePtr m = ModulePtr(new Module()))
    {
      opers_arithmetic_overload<T, T, T>(m);
      return m;
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
    ModulePtr bootstrap_pod_type(const std::string &name, ModulePtr m = ModulePtr(new Module()))
    {
      m->add(user_type<T>(), name);
      basic_constructors<T>(name, m);
      oper_assign<T>(m);
      oper_assign_pod<T>(m);
      construct_pod<T>(name, m);
      opers_arithmetic<T>(m);
      opers_arithmetic_modify_pod<T>(m);
      m->add(fun(&to_string<T>), "to_string");
      m->add(fun(&parse_string<T>), "to_" + name);
      return m;
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
    class Bootstrap
    {
    private:
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
      static void opers_comparison_pod(ModulePtr m = ModulePtr(new Module()))
      {
        m->add(fun(&detail::equals<Boxed_POD_Value, Boxed_POD_Value>), "==");
        m->add(fun(&detail::not_equals<Boxed_POD_Value, Boxed_POD_Value>), "!=");
        m->add(fun(&detail::less_than<Boxed_POD_Value, Boxed_POD_Value>), "<");
        m->add(fun(&detail::greater_than<Boxed_POD_Value, Boxed_POD_Value>), ">");
        m->add(fun(&detail::less_than_equals<Boxed_POD_Value, Boxed_POD_Value>), "<=");
        m->add(fun(&detail::greater_than_equals<Boxed_POD_Value, Boxed_POD_Value>), ">=");
      }

      /**
      * Add all arithmetic operators for PODs
      */
      static void opers_arithmetic_pod(ModulePtr m = ModulePtr(new Module()))
      {
        m->add(fun(&detail::add<Boxed_Value, Boxed_POD_Value, Boxed_POD_Value>), "+");
        m->add(fun(&detail::subtract<Boxed_Value, Boxed_POD_Value, Boxed_POD_Value>), "-");
        m->add(fun(&detail::divide<Boxed_Value, Boxed_POD_Value, Boxed_POD_Value>), "/");
        m->add(fun(&detail::multiply<Boxed_Value, Boxed_POD_Value, Boxed_POD_Value>), "*");
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

      static boost::shared_ptr<Dispatch_Engine> bootstrap2(boost::shared_ptr<Dispatch_Engine> e = boost::shared_ptr<Dispatch_Engine> (new Dispatch_Engine()))
      {
        e->add(user_type<void>(), "void");
        return e;
      }

    public:
      /**
      * perform all common bootstrap functions for std::string, void and POD types
      */
      static ModulePtr bootstrap(ModulePtr m = ModulePtr(new Module()))
      {
        m->add(user_type<void>(), "void");
        m->add(user_type<bool>(), "bool");
        m->add(user_type<Boxed_Value>(), "Object");
        m->add(user_type<Boxed_POD_Value>(), "PODObject");
        m->add(user_type<Proxy_Function>(), "function");

        basic_constructors<bool>("bool", m);
        oper_assign<std::string>(m);
        oper_assign<bool>(m);

        m->add(fun(&to_string<const std::string &>), "internal_to_string");
        m->add(fun(&to_string<bool>), "internal_to_string");
        m->add(fun(&unknown_assign), "=");

        bootstrap_pod_type<double>("double", m);
        bootstrap_pod_type<int>("int", m);
        bootstrap_pod_type<size_t>("size_t", m);
        bootstrap_pod_type<char>("char", m);
        bootstrap_pod_type<boost::int64_t>("int64_t", m);

        opers_comparison_pod(m);
        opers_arithmetic_pod(m);

        m->add(fun(&detail::modulus<int, int, int>), "%");

        m->add(fun(&print), "print_string");
        m->add(fun(&println), "println_string");

        m->add(Proxy_Function(new Dynamic_Proxy_Function(boost::bind(&bind_function, _1))), 
          "bind");

        m->add(fun(&shared_ptr_clone<Proxy_Function_Base>), "clone");
        m->add(fun(&ptr_assign<Proxy_Function_Base>), "=");

        m->add(Proxy_Function(new Dynamic_Proxy_Function(boost::bind(&call_exists, _1))), 
          "call_exists");

        m->add(fun(&type_match), "type_match");

        return m;
      }
    };
  }
}

#endif

