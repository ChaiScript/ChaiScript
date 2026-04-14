// This file is distributed under the BSD License.
// See "license.txt" for details.
// Copyright 2009-2012, Jonathan Turner (jonathan@emptycrate.com)
// Copyright 2009-2018, Jason Turner (jason@emptycrate.com)
// http://www.chaiscript.com

// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#ifndef CHAISCRIPT_OPERATORS_HPP_
#define CHAISCRIPT_OPERATORS_HPP_

#include "../chaiscript_defines.hpp"
#include "../dispatchkit/dispatchkit.hpp"
#include "register_function.hpp"

namespace chaiscript::bootstrap::operators {
  template<typename T, typename ModuleType>
  void assign(ModuleType &m) {
    m.add(chaiscript::fun([](T &lhs, const T &rhs) -> T & { return lhs = rhs; }), "=");
  }

  template<typename T, typename ModuleType>
  void assign_bitwise_and(ModuleType &m) {
    m.add(chaiscript::fun([](T &lhs, const T &rhs) -> T & { return lhs &= rhs; }), "&=");
  }

  template<typename T, typename ModuleType>
  void assign_xor(ModuleType &m) {
    m.add(chaiscript::fun([](T &lhs, const T &rhs) -> T & { return lhs ^= rhs; }), "^=");
  }

  template<typename T, typename ModuleType>
  void assign_bitwise_or(ModuleType &m) {
    m.add(chaiscript::fun([](T &lhs, const T &rhs) -> T & { return lhs |= rhs; }), "|=");
  }

  template<typename T, typename ModuleType>
  void assign_difference(ModuleType &m) {
    m.add(chaiscript::fun([](T &lhs, const T &rhs) -> T & { return lhs -= rhs; }), "-=");
  }

  template<typename T, typename ModuleType>
  void assign_left_shift(ModuleType &m) {
    m.add(chaiscript::fun([](T &lhs, const T &rhs) -> T & { return lhs <<= rhs; }), "<<=");
  }

  template<typename T, typename ModuleType>
  void assign_product(ModuleType &m) {
    m.add(chaiscript::fun([](T &lhs, const T &rhs) -> T & { return lhs <<= rhs; }), "*=");
  }

  template<typename T, typename ModuleType>
  void assign_quotient(ModuleType &m) {
    m.add(chaiscript::fun([](T &lhs, const T &rhs) -> T & { return lhs /= rhs; }), "/=");
  }

  template<typename T, typename ModuleType>
  void assign_remainder(ModuleType &m) {
    m.add(chaiscript::fun([](T &lhs, const T &rhs) -> T & { return lhs %= rhs; }), "%=");
  }

  template<typename T, typename ModuleType>
  void assign_right_shift(ModuleType &m) {
    m.add(chaiscript::fun([](T &lhs, const T &rhs) -> T & { return lhs >>= rhs; }), ">>=");
  }

  template<typename T, typename ModuleType>
  void assign_sum(ModuleType &m) {
    m.add(chaiscript::fun([](T &lhs, const T &rhs) -> T & { return lhs += rhs; }), "+=");
  }

  template<typename T, typename ModuleType>
  void prefix_decrement(ModuleType &m) {
    m.add(chaiscript::fun([](T &lhs) -> T & { return --lhs; }), "--");
  }

  template<typename T, typename ModuleType>
  void prefix_increment(ModuleType &m) {
    m.add(chaiscript::fun([](T &lhs) -> T & { return ++lhs; }), "++");
  }

  template<typename T, typename ModuleType>
  void equal(ModuleType &m) {
    m.add(chaiscript::fun([](const T &lhs, const T &rhs) { return lhs == rhs; }), "==");
  }

  template<typename T, typename ModuleType>
  void greater_than(ModuleType &m) {
    m.add(chaiscript::fun([](const T &lhs, const T &rhs) { return lhs > rhs; }), ">");
  }

  template<typename T, typename ModuleType>
  void greater_than_equal(ModuleType &m) {
    m.add(chaiscript::fun([](const T &lhs, const T &rhs) { return lhs >= rhs; }), ">=");
  }

  template<typename T, typename ModuleType>
  void less_than(ModuleType &m) {
    m.add(chaiscript::fun([](const T &lhs, const T &rhs) { return lhs < rhs; }), "<");
  }

  template<typename T, typename ModuleType>
  void less_than_equal(ModuleType &m) {
    m.add(chaiscript::fun([](const T &lhs, const T &rhs) { return lhs <= rhs; }), "<=");
  }

  template<typename T, typename ModuleType>
  void logical_compliment(ModuleType &m) {
    m.add(chaiscript::fun([](const T &lhs) { return !lhs; }), "!");
  }

  template<typename T, typename ModuleType>
  void not_equal(ModuleType &m) {
    m.add(chaiscript::fun([](const T &lhs, const T &rhs) { return lhs != rhs; }), "!=");
  }

  template<typename T, typename ModuleType>
  void addition(ModuleType &m) {
    m.add(chaiscript::fun([](const T &lhs, const T &rhs) { return lhs + rhs; }), "+");
  }

  template<typename T, typename ModuleType>
  void unary_plus(ModuleType &m) {
    m.add(chaiscript::fun([](const T &lhs) { return +lhs; }), "+");
  }

  template<typename T, typename ModuleType>
  void subtraction(ModuleType &m) {
    m.add(chaiscript::fun([](const T &lhs, const T &rhs) { return lhs - rhs; }), "-");
  }

  template<typename T, typename ModuleType>
  void unary_minus(ModuleType &m) {
    m.add(chaiscript::fun([](const T &lhs) { return -lhs; }), "-");
  }

  template<typename T, typename ModuleType>
  void bitwise_and(ModuleType &m) {
    m.add(chaiscript::fun([](const T &lhs, const T &rhs) { return lhs & rhs; }), "&");
  }

  template<typename T, typename ModuleType>
  void bitwise_compliment(ModuleType &m) {
    m.add(chaiscript::fun([](const T &lhs) { return ~lhs; }), "~");
  }

  template<typename T, typename ModuleType>
  void bitwise_xor(ModuleType &m) {
    m.add(chaiscript::fun([](const T &lhs, const T &rhs) { return lhs ^ rhs; }), "^");
  }

  template<typename T, typename ModuleType>
  void bitwise_or(ModuleType &m) {
    m.add(chaiscript::fun([](const T &lhs, const T &rhs) { return lhs | rhs; }), "|");
  }

  template<typename T, typename ModuleType>
  void division(ModuleType &m) {
    m.add(chaiscript::fun([](const T &lhs, const T &rhs) { return lhs / rhs; }), "/");
  }

  template<typename T, typename ModuleType>
  void left_shift(ModuleType &m) {
    m.add(chaiscript::fun([](const T &lhs, const T &rhs) { return lhs << rhs; }), "<<");
  }

  template<typename T, typename ModuleType>
  void multiplication(ModuleType &m) {
    m.add(chaiscript::fun([](const T &lhs, const T &rhs) { return lhs * rhs; }), "*");
  }

  template<typename T, typename ModuleType>
  void remainder(ModuleType &m) {
    m.add(chaiscript::fun([](const T &lhs, const T &rhs) { return lhs % rhs; }), "%");
  }

  template<typename T, typename ModuleType>
  void right_shift(ModuleType &m) {
    m.add(chaiscript::fun([](const T &lhs, const T &rhs) { return lhs >> rhs; }), ">>");
  }
} // namespace chaiscript::bootstrap::operators

#endif
