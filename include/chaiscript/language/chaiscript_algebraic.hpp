// This file is distributed under the BSD License.
// See "license.txt" for details.
// Copyright 2009-2012, Jonathan Turner (jonathan@emptycrate.com)
// Copyright 2009-2015, Jason Turner (jason@emptycrate.com)
// http://www.chaiscript.com

#ifndef CHAISCRIPT_ALGEBRAIC_HPP_
#define CHAISCRIPT_ALGEBRAIC_HPP_

#include <string>

namespace chaiscript
{

  struct Operators {
    enum Opers
    {
      boolean_flag,
      equals, less_than, greater_than, less_than_equal, greater_than_equal, not_equal, 
      non_const_flag, 
      assign, pre_increment, pre_decrement, assign_product, assign_sum,
      assign_quotient, assign_difference,
      non_const_int_flag,
      assign_bitwise_and, assign_bitwise_or, assign_shift_left, assign_shift_right,
      assign_remainder, assign_bitwise_xor,
      const_int_flag,
      shift_left, shift_right, remainder, bitwise_and, bitwise_or, bitwise_xor, bitwise_complement,
      const_flag,
      sum, quotient, product, difference, unary_plus, unary_minus, 
      invalid
    };

    static const char *to_string(Opers t_oper) {
      const char *opers[] = { 
        "",
        "==", "<", ">", "<=", ">=", "!=",
        "",
        "=", "++", "--", "*=", "+=",
        "/=", "-=",
        "",
        "&=", "|=", "<<=", ">>=",
        "%=", "^=",
        "", 
        "<<", ">>", "%", "&", "|", "^", "~",
        "",
        "+", "/", "*", "-", "+", "-",
        ""
      };
      return opers[t_oper];
    }

    static Opers to_operator(const std::string &t_str, bool t_is_unary = false)
    {
      if (t_str == "==")
      {
        return equals;
      } else if (t_str == "<") {
        return less_than;
      } else if (t_str == ">") {
        return greater_than;
      } else if (t_str == "<=") {
        return less_than_equal; 
      } else if (t_str == ">=") {
        return greater_than_equal;
      } else if (t_str == "!=") {
        return not_equal;
      } else if (t_str == "=") {
        return assign;
      } else if (t_str == "++") {
        return pre_increment;
      } else if (t_str == "--") {
        return pre_decrement;
      } else if (t_str == "*=") {
        return assign_product;
      } else if (t_str == "+=") {
        return assign_sum;
      } else if (t_str == "-=") {
        return assign_difference;
      } else if (t_str == "&=") {
        return assign_bitwise_and;
      } else if (t_str == "|=") {
        return assign_bitwise_or;
      } else if (t_str == "<<=") {
        return assign_shift_left;
      } else if (t_str == ">>=") {
        return assign_shift_right;
      } else if (t_str == "%=") {
        return assign_remainder;
      } else if (t_str == "^=") {
        return assign_bitwise_xor;
      } else if (t_str == "<<") {
        return shift_left;
      } else if (t_str == ">>") {
        return shift_right;
      } else if (t_str == "%") {
        return remainder;
      } else if (t_str == "&") { 
        return bitwise_and;
      } else if (t_str == "|") {
        return bitwise_or;
      } else if (t_str == "^") {
        return bitwise_xor;
      } else if (t_str == "~") {
        return bitwise_complement;
      } else if (t_str == "+") {
        if (t_is_unary) {
          return unary_plus;
        } else {
          return sum;
        }
      } else if (t_str == "-") {
        if (t_is_unary) {
          return unary_minus;
        } else {
          return difference;
        }
      } else if (t_str == "/") {
        return quotient;
      } else if (t_str == "*") {
        return product;
      } else {
        return invalid;
      } 
    }

  };
}

#endif /* _CHAISCRIPT_ALGEBRAIC_HPP */

