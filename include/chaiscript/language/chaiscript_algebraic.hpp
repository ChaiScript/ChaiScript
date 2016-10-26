// This file is distributed under the BSD License.
// See "license.txt" for details.
// Copyright 2009-2012, Jonathan Turner (jonathan@emptycrate.com)
// Copyright 2009-2016, Jason Turner (jason@emptycrate.com)
// http://www.chaiscript.com

#ifndef CHAISCRIPT_ALGEBRAIC_HPP_
#define CHAISCRIPT_ALGEBRAIC_HPP_

#include <string>

namespace chaiscript
{

  struct Operators {
    enum class Opers
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
      static const char *opers[] = { 
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
      return opers[static_cast<int>(t_oper)];
    }

    static Opers to_operator(const std::string &t_str, bool t_is_unary = false)
    {
      if (t_str == "==")
      {
        return Opers::equals;
      } else if (t_str == "<") {
        return Opers::less_than;
      } else if (t_str == ">") {
        return Opers::greater_than;
      } else if (t_str == "<=") {
        return Opers::less_than_equal; 
      } else if (t_str == ">=") {
        return Opers::greater_than_equal;
      } else if (t_str == "!=") {
        return Opers::not_equal;
      } else if (t_str == "=") {
        return Opers::assign;
      } else if (t_str == "++") {
        return Opers::pre_increment;
      } else if (t_str == "--") {
        return Opers::pre_decrement;
      } else if (t_str == "*=") {
        return Opers::assign_product;
      } else if (t_str == "+=") {
        return Opers::assign_sum;
      } else if (t_str == "-=") {
        return Opers::assign_difference;
      } else if (t_str == "&=") {
        return Opers::assign_bitwise_and;
      } else if (t_str == "|=") {
        return Opers::assign_bitwise_or;
      } else if (t_str == "<<=") {
        return Opers::assign_shift_left;
      } else if (t_str == ">>=") {
        return Opers::assign_shift_right;
      } else if (t_str == "%=") {
        return Opers::assign_remainder;
      } else if (t_str == "^=") {
        return Opers::assign_bitwise_xor;
      } else if (t_str == "<<") {
        return Opers::shift_left;
      } else if (t_str == ">>") {
        return Opers::shift_right;
      } else if (t_str == "%") {
        return Opers::remainder;
      } else if (t_str == "&") { 
        return Opers::bitwise_and;
      } else if (t_str == "|") {
        return Opers::bitwise_or;
      } else if (t_str == "^") {
        return Opers::bitwise_xor;
      } else if (t_str == "~") {
        return Opers::bitwise_complement;
      } else if (t_str == "+") {
        if (t_is_unary) {
          return Opers::unary_plus;
        } else {
          return Opers::sum;
        }
      } else if (t_str == "-") {
        if (t_is_unary) {
          return Opers::unary_minus;
        } else {
          return Opers::difference;
        }
      } else if (t_str == "/") {
        return Opers::quotient;
      } else if (t_str == "*") {
        return Opers::product;
      } else {
        return Opers::invalid;
      } 
    }

  };
}

#endif /* _CHAISCRIPT_ALGEBRAIC_HPP */

