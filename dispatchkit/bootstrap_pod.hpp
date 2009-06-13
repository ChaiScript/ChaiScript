#ifndef __dispatchkit_bootstrap_pod_hpp__
#define __dispatchkit_bootstrap_pod_hpp__


#include "register_function.hpp"


struct Pod_Bootstrap
{
  static Boxed_Value add(Boxed_POD_Value l, Boxed_POD_Value r)
  {
    return l + r;
  }

  static Boxed_Value subtract(Boxed_POD_Value l, Boxed_POD_Value r)
  {
    return l - r;
  }

  static Boxed_Value divide(Boxed_POD_Value l, Boxed_POD_Value r)
  {
    return l / r;
  }

  static Boxed_Value multiply(Boxed_POD_Value l, Boxed_POD_Value r)
  {
    return l * r;
  }

  static bool equals(Boxed_POD_Value l, Boxed_POD_Value r)
  {
    return l == r;
  }

  static bool not_equals(Boxed_POD_Value l, Boxed_POD_Value r)
  {
    return l != r;
  }

  static bool less_than(Boxed_POD_Value l, Boxed_POD_Value r)
  {
    return l < r;
  }

  static bool greater_than(Boxed_POD_Value l, Boxed_POD_Value r)
  {
    return l > r;
  }

  static bool less_than_equals(Boxed_POD_Value l, Boxed_POD_Value r)
  {
    return l <= r;
  }

  static bool greater_than_equals(Boxed_POD_Value l, Boxed_POD_Value r)
  {
    return l >= r;
  }

  static void add_opers_comparison(Dispatch_Engine &s)
  {
    register_function(s, &equals, "==");
    register_function(s, &not_equals, "!=");
    register_function(s, &less_than, "<");
    register_function(s, &greater_than, ">");
    register_function(s, &less_than_equals, "<=");
    register_function(s, &greater_than_equals, ">=");
  }

  static void add_opers_arithmetic(Dispatch_Engine &s)
  {
    register_function(s, &add, "+");
    register_function(s, &subtract, "-");
    register_function(s, &divide, "/");
    register_function(s, &multiply, "*");
  }
};

#endif

