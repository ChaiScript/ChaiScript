// This file is distributed under the BSD License.
// See "license.txt" for details.
// Copyright 2009-2012, Jonathan Turner (jonathan@emptycrate.com)
// Copyright 2009-2018, Jason Turner (jason@emptycrate.com)
// http://www.chaiscript.com

// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#ifndef CHAISCRIPT_BOXED_NUMERIC_HPP_
#define CHAISCRIPT_BOXED_NUMERIC_HPP_

#include <cstdint>
#include <sstream>
#include <string>

#include "../language/chaiscript_algebraic.hpp"
#include "any.hpp"
#include "boxed_cast.hpp"
#include "boxed_cast_helper.hpp"
#include "boxed_value.hpp"
#include "type_info.hpp"

namespace chaiscript {
  class Type_Conversions;
} // namespace chaiscript

namespace chaiscript::exception {
  struct arithmetic_error : std::runtime_error {
    explicit arithmetic_error(const std::string &reason)
        : std::runtime_error("Arithmetic error: " + reason) {
    }
    arithmetic_error(const arithmetic_error &) = default;
    ~arithmetic_error() noexcept override = default;
  };
} // namespace chaiscript::exception

namespace chaiscript {
// Due to the nature of generating every possible arithmetic operation, there
// are going to be warnings generated on every platform regarding size and sign,
// this is OK, so we're disabling size/and sign type warnings
#ifdef CHAISCRIPT_MSVC
#pragma warning(push)
#pragma warning(disable : 4244 4018 4389 4146 4365 4267 4242)
#endif

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunknown-pragmas"
#pragma GCC diagnostic ignored "-Wpragmas"
#pragma GCC diagnostic ignored "-Wsign-compare"
#pragma GCC diagnostic ignored "-Wfloat-equal"
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wfloat-conversion"
#pragma GCC diagnostic ignored "-Wswitch"
#endif

  /// \brief Represents any numeric type, generically. Used internally for generic operations between POD values
  class Boxed_Number {
  private:
    enum class Common_Types {
      t_int32,
      t_double,
      t_uint8,
      t_int8,
      t_uint16,
      t_int16,
      t_uint32,
      t_uint64,
      t_int64,
      t_float,
      t_long_double
    };

    template<typename T>
    constexpr static inline void check_divide_by_zero([[maybe_unused]] T t) {
#ifndef CHAISCRIPT_NO_PROTECT_DIVIDEBYZERO
      if constexpr (!std::is_floating_point<T>::value) {
        if (t == 0) {
          throw chaiscript::exception::arithmetic_error("divide by zero");
        }
      }
#endif
    }

    constexpr static Common_Types get_common_type(size_t t_size, bool t_signed) noexcept {
      return (t_size == 1 && t_signed) ? (Common_Types::t_int8)
          : (t_size == 1)              ? (Common_Types::t_uint8)
          : (t_size == 2 && t_signed)  ? (Common_Types::t_int16)
          : (t_size == 2)              ? (Common_Types::t_uint16)
          : (t_size == 4 && t_signed)  ? (Common_Types::t_int32)
          : (t_size == 4)              ? (Common_Types::t_uint32)
          : (t_size == 8 && t_signed)  ? (Common_Types::t_int64)
                                       : (Common_Types::t_uint64);
    }

    static Common_Types get_common_type(const Boxed_Value &t_bv) {
      const Type_Info &inp_ = t_bv.get_type_info();

      if (inp_ == user_type<int>()) {
        return get_common_type(sizeof(int), true);
      } else if (inp_ == user_type<double>()) {
        return Common_Types::t_double;
      } else if (inp_ == user_type<long double>()) {
        return Common_Types::t_long_double;
      } else if (inp_ == user_type<float>()) {
        return Common_Types::t_float;
      } else if (inp_ == user_type<char>()) {
        return get_common_type(sizeof(char), std::is_signed<char>::value);
      } else if (inp_ == user_type<unsigned char>()) {
        return get_common_type(sizeof(unsigned char), false);
      } else if (inp_ == user_type<unsigned int>()) {
        return get_common_type(sizeof(unsigned int), false);
      } else if (inp_ == user_type<long>()) {
        return get_common_type(sizeof(long), true);
      } else if (inp_ == user_type<long long>()) {
        return get_common_type(sizeof(long long), true);
      } else if (inp_ == user_type<unsigned long>()) {
        return get_common_type(sizeof(unsigned long), false);
      } else if (inp_ == user_type<unsigned long long>()) {
        return get_common_type(sizeof(unsigned long long), false);
      } else if (inp_ == user_type<std::int8_t>()) {
        return Common_Types::t_int8;
      } else if (inp_ == user_type<std::int16_t>()) {
        return Common_Types::t_int16;
      } else if (inp_ == user_type<std::int32_t>()) {
        return Common_Types::t_int32;
      } else if (inp_ == user_type<std::int64_t>()) {
        return Common_Types::t_int64;
      } else if (inp_ == user_type<std::uint8_t>()) {
        return Common_Types::t_uint8;
      } else if (inp_ == user_type<std::uint16_t>()) {
        return Common_Types::t_uint16;
      } else if (inp_ == user_type<std::uint32_t>()) {
        return Common_Types::t_uint32;
      } else if (inp_ == user_type<std::uint64_t>()) {
        return Common_Types::t_uint64;
      } else if (inp_ == user_type<wchar_t>()) {
        return get_common_type(sizeof(wchar_t), std::is_signed<wchar_t>::value);
      } else if (inp_ == user_type<char16_t>()) {
        return get_common_type(sizeof(char16_t), std::is_signed<char16_t>::value);
      } else if (inp_ == user_type<char32_t>()) {
        return get_common_type(sizeof(char32_t), std::is_signed<char32_t>::value);
      } else {
        throw chaiscript::detail::exception::bad_any_cast();
      }
    }

    template<typename LHS, typename RHS>
    static auto go(Operators::Opers t_oper, const Boxed_Value &t_bv, LHS *t_lhs, const LHS &c_lhs, const RHS &c_rhs) {
      switch (t_oper) {
        case Operators::Opers::equals:
          return const_var(c_lhs == c_rhs);
        case Operators::Opers::less_than:
          return const_var(c_lhs < c_rhs);
        case Operators::Opers::greater_than:
          return const_var(c_lhs > c_rhs);
        case Operators::Opers::less_than_equal:
          return const_var(c_lhs <= c_rhs);
        case Operators::Opers::greater_than_equal:
          return const_var(c_lhs >= c_rhs);
        case Operators::Opers::not_equal:
          return const_var(c_lhs != c_rhs);
        case Operators::Opers::sum:
          return const_var(c_lhs + c_rhs);
        case Operators::Opers::quotient:
          check_divide_by_zero(c_rhs);
          return const_var(c_lhs / c_rhs);
        case Operators::Opers::product:
          return const_var(c_lhs * c_rhs);
        case Operators::Opers::difference:
          return const_var(c_lhs - c_rhs);
        default:
          break;
      }

      if constexpr (!std::is_floating_point<LHS>::value && !std::is_floating_point<RHS>::value) {
        switch (t_oper) {
          case Operators::Opers::shift_left:
            return const_var(c_lhs << c_rhs);
          case Operators::Opers::shift_right:
            return const_var(c_lhs >> c_rhs);
          case Operators::Opers::remainder:
            check_divide_by_zero(c_rhs);
            return const_var(c_lhs % c_rhs);
          case Operators::Opers::bitwise_and:
            return const_var(c_lhs & c_rhs);
          case Operators::Opers::bitwise_or:
            return const_var(c_lhs | c_rhs);
          case Operators::Opers::bitwise_xor:
            return const_var(c_lhs ^ c_rhs);
          default:
            break;
        }
      }

      if (t_lhs) {
        switch (t_oper) {
          case Operators::Opers::assign:
            *t_lhs = c_rhs;
            return t_bv;
          case Operators::Opers::assign_product:
            *t_lhs *= c_rhs;
            return t_bv;
          case Operators::Opers::assign_sum:
            *t_lhs += c_rhs;
            return t_bv;
          case Operators::Opers::assign_quotient:
            check_divide_by_zero(c_rhs);
            *t_lhs /= c_rhs;
            return t_bv;
          case Operators::Opers::assign_difference:
            *t_lhs -= c_rhs;
            return t_bv;
          default:
            break;
        }

        if constexpr (!std::is_floating_point<LHS>::value && !std::is_floating_point<RHS>::value) {
          switch (t_oper) {
            case Operators::Opers::assign_bitwise_and:
              check_divide_by_zero(c_rhs);
              *t_lhs &= c_rhs;
              return t_bv;
            case Operators::Opers::assign_bitwise_or:
              *t_lhs |= c_rhs;
              return t_bv;
            case Operators::Opers::assign_shift_left:
              *t_lhs <<= c_rhs;
              return t_bv;
            case Operators::Opers::assign_shift_right:
              *t_lhs >>= c_rhs;
              return t_bv;
            case Operators::Opers::assign_remainder:
              *t_lhs %= c_rhs;
              return t_bv;
            case Operators::Opers::assign_bitwise_xor:
              *t_lhs ^= c_rhs;
              return t_bv;
            default:
              break;
          }
        }
      }

      throw chaiscript::detail::exception::bad_any_cast();
    }

    template<typename Callable>
    inline static auto visit(const Boxed_Value &bv, Callable &&callable) {
      switch (get_common_type(bv)) {
        case Common_Types::t_int32:
          return callable(*static_cast<const std::int32_t *>(bv.get_const_ptr()));
        case Common_Types::t_uint8:
          return callable(*static_cast<const std::uint8_t *>(bv.get_const_ptr()));
        case Common_Types::t_int8:
          return callable(*static_cast<const std::int8_t *>(bv.get_const_ptr()));
        case Common_Types::t_uint16:
          return callable(*static_cast<const std::uint16_t *>(bv.get_const_ptr()));
        case Common_Types::t_int16:
          return callable(*static_cast<const std::int16_t *>(bv.get_const_ptr()));
        case Common_Types::t_uint32:
          return callable(*static_cast<const std::uint32_t *>(bv.get_const_ptr()));
        case Common_Types::t_uint64:
          return callable(*static_cast<const std::uint64_t *>(bv.get_const_ptr()));
        case Common_Types::t_int64:
          return callable(*static_cast<const std::int64_t *>(bv.get_const_ptr()));
        case Common_Types::t_double:
          return callable(*static_cast<const double *>(bv.get_const_ptr()));
        case Common_Types::t_float:
          return callable(*static_cast<const float *>(bv.get_const_ptr()));
        case Common_Types::t_long_double:
          return callable(*static_cast<const long double *>(bv.get_const_ptr()));
      }
      throw chaiscript::detail::exception::bad_any_cast();
    }

    inline static Boxed_Value oper(Operators::Opers t_oper, const Boxed_Value &t_lhs) {
      auto unary_operator = [t_oper, &t_lhs](const auto &c_lhs) {
        auto *lhs = static_cast<std::decay_t<decltype(c_lhs)> *>(t_lhs.get_ptr());

        if (lhs) {
          switch (t_oper) {
            case Operators::Opers::pre_increment:
              ++(*lhs);
              return t_lhs;
            case Operators::Opers::pre_decrement:
              --(*lhs);
              return t_lhs;
            default:
              break;
          }
        }

        switch (t_oper) {
          case Operators::Opers::unary_minus:
            return const_var(-c_lhs);
          case Operators::Opers::unary_plus:
            return const_var(+c_lhs);
          default:
            break;
        }

        if constexpr (!std::is_floating_point_v<std::decay_t<decltype(c_lhs)>>) {
          switch (t_oper) {
            case Operators::Opers::bitwise_complement:
              return const_var(~c_lhs);
            default:
              break;
          }
        }

        throw chaiscript::detail::exception::bad_any_cast();
      };

      return visit(t_lhs, unary_operator);
    }

    inline static Boxed_Value oper(Operators::Opers t_oper, const Boxed_Value &t_lhs, const Boxed_Value &t_rhs) {
      auto lhs_visit = [t_oper, &t_lhs, &t_rhs](const auto &c_lhs) {
        auto *lhs = t_lhs.is_return_value() ? nullptr : static_cast<std::decay_t<decltype(c_lhs)> *>(t_lhs.get_ptr());

        auto rhs_visit = [t_oper, &t_lhs, lhs, &c_lhs](const auto &c_rhs) { return go(t_oper, t_lhs, lhs, c_lhs, c_rhs); };

        return visit(t_rhs, rhs_visit);
      };

      return visit(t_lhs, lhs_visit);
    }

    template<typename Target, typename Source>
    static inline Target get_as_aux(const Boxed_Value &t_bv) {
      return static_cast<Target>(*static_cast<const Source *>(t_bv.get_const_ptr()));
    }

    template<typename Source>
    static std::string to_string_aux(const Boxed_Value &v) {
      std::ostringstream oss;
      oss << *static_cast<const Source *>(v.get_const_ptr());
      return oss.str();
    }

  public:
    Boxed_Number()
        : bv(Boxed_Value(0)) {
    }

    explicit Boxed_Number(Boxed_Value v)
        : bv(std::move(v)) {
      validate_boxed_number(bv);
    }

    Boxed_Number(const Boxed_Number &) = default;
    Boxed_Number(Boxed_Number &&) = default;
    Boxed_Number &operator=(Boxed_Number &&) = default;

    template<typename T>
    explicit Boxed_Number(T t)
        : bv(Boxed_Value(t)) {
      validate_boxed_number(bv);
    }

    static Boxed_Value clone(const Boxed_Value &t_bv) { return Boxed_Number(t_bv).get_as(t_bv.get_type_info()).bv; }

    static bool is_floating_point(const Boxed_Value &t_bv) {
      const Type_Info &inp_ = t_bv.get_type_info();

      if (inp_ == user_type<double>()) {
        return true;
      } else if (inp_ == user_type<long double>()) {
        return true;
      } else if (inp_ == user_type<float>()) {
        return true;
      } else {
        return false;
      }
    }

    Boxed_Number get_as(const Type_Info &inp_) const {
      if (inp_.bare_equal(user_type<int>())) {
        return Boxed_Number(get_as<int>());
      } else if (inp_.bare_equal(user_type<double>())) {
        return Boxed_Number(get_as<double>());
      } else if (inp_.bare_equal(user_type<float>())) {
        return Boxed_Number(get_as<float>());
      } else if (inp_.bare_equal(user_type<long double>())) {
        return Boxed_Number(get_as<long double>());
      } else if (inp_.bare_equal(user_type<char>())) {
        return Boxed_Number(get_as<char>());
      } else if (inp_.bare_equal(user_type<unsigned char>())) {
        return Boxed_Number(get_as<unsigned char>());
      } else if (inp_.bare_equal(user_type<wchar_t>())) {
        return Boxed_Number(get_as<wchar_t>());
      } else if (inp_.bare_equal(user_type<char16_t>())) {
        return Boxed_Number(get_as<char16_t>());
      } else if (inp_.bare_equal(user_type<char32_t>())) {
        return Boxed_Number(get_as<char32_t>());
      } else if (inp_.bare_equal(user_type<unsigned int>())) {
        return Boxed_Number(get_as<unsigned int>());
      } else if (inp_.bare_equal(user_type<long>())) {
        return Boxed_Number(get_as<long>());
      } else if (inp_.bare_equal(user_type<long long>())) {
        return Boxed_Number(get_as<long long>());
      } else if (inp_.bare_equal(user_type<unsigned long>())) {
        return Boxed_Number(get_as<unsigned long>());
      } else if (inp_.bare_equal(user_type<unsigned long long>())) {
        return Boxed_Number(get_as<unsigned long long>());
      } else if (inp_.bare_equal(user_type<int8_t>())) {
        return Boxed_Number(get_as<int8_t>());
      } else if (inp_.bare_equal(user_type<int16_t>())) {
        return Boxed_Number(get_as<int16_t>());
      } else if (inp_.bare_equal(user_type<int32_t>())) {
        return Boxed_Number(get_as<int32_t>());
      } else if (inp_.bare_equal(user_type<int64_t>())) {
        return Boxed_Number(get_as<int64_t>());
      } else if (inp_.bare_equal(user_type<uint8_t>())) {
        return Boxed_Number(get_as<uint8_t>());
      } else if (inp_.bare_equal(user_type<uint16_t>())) {
        return Boxed_Number(get_as<uint16_t>());
      } else if (inp_.bare_equal(user_type<uint32_t>())) {
        return Boxed_Number(get_as<uint32_t>());
      } else if (inp_.bare_equal(user_type<uint64_t>())) {
        return Boxed_Number(get_as<uint64_t>());
      } else {
        throw chaiscript::detail::exception::bad_any_cast();
      }
    }

    template<typename Source, typename Target>
    static void check_type() {
#ifdef CHAISCRIPT_MSVC
// MSVC complains about this being redundant / tautologica l
#pragma warning(push)
#pragma warning(disable : 4127 6287)
#endif
      if (sizeof(Source) != sizeof(Target) || std::is_signed<Source>() != std::is_signed<Target>()
          || std::is_floating_point<Source>() != std::is_floating_point<Target>()) {
        throw chaiscript::detail::exception::bad_any_cast();
      }
#ifdef CHAISCRIPT_MSVC
#pragma warning(pop)
#endif
    }

    template<typename Target>
    Target get_as_checked() const {
      switch (get_common_type(bv)) {
        case Common_Types::t_int32:
          check_type<int32_t, Target>();
          return get_as_aux<Target, int32_t>(bv);
        case Common_Types::t_uint8:
          check_type<uint8_t, Target>();
          return get_as_aux<Target, uint8_t>(bv);
        case Common_Types::t_int8:
          check_type<int8_t, Target>();
          return get_as_aux<Target, int8_t>(bv);
        case Common_Types::t_uint16:
          check_type<uint16_t, Target>();
          return get_as_aux<Target, uint16_t>(bv);
        case Common_Types::t_int16:
          check_type<int16_t, Target>();
          return get_as_aux<Target, int16_t>(bv);
        case Common_Types::t_uint32:
          check_type<uint32_t, Target>();
          return get_as_aux<Target, uint32_t>(bv);
        case Common_Types::t_uint64:
          check_type<uint64_t, Target>();
          return get_as_aux<Target, uint64_t>(bv);
        case Common_Types::t_int64:
          check_type<int64_t, Target>();
          return get_as_aux<Target, int64_t>(bv);
        case Common_Types::t_double:
          check_type<double, Target>();
          return get_as_aux<Target, double>(bv);
        case Common_Types::t_float:
          check_type<float, Target>();
          return get_as_aux<Target, float>(bv);
        case Common_Types::t_long_double:
          check_type<long double, Target>();
          return get_as_aux<Target, long double>(bv);
      }

      throw chaiscript::detail::exception::bad_any_cast();
    }

    template<typename Target>
    Target get_as() const {
      switch (get_common_type(bv)) {
        case Common_Types::t_int32:
          return get_as_aux<Target, int32_t>(bv);
        case Common_Types::t_uint8:
          return get_as_aux<Target, uint8_t>(bv);
        case Common_Types::t_int8:
          return get_as_aux<Target, int8_t>(bv);
        case Common_Types::t_uint16:
          return get_as_aux<Target, uint16_t>(bv);
        case Common_Types::t_int16:
          return get_as_aux<Target, int16_t>(bv);
        case Common_Types::t_uint32:
          return get_as_aux<Target, uint32_t>(bv);
        case Common_Types::t_uint64:
          return get_as_aux<Target, uint64_t>(bv);
        case Common_Types::t_int64:
          return get_as_aux<Target, int64_t>(bv);
        case Common_Types::t_double:
          return get_as_aux<Target, double>(bv);
        case Common_Types::t_float:
          return get_as_aux<Target, float>(bv);
        case Common_Types::t_long_double:
          return get_as_aux<Target, long double>(bv);
      }

      throw chaiscript::detail::exception::bad_any_cast();
    }

    std::string to_string() const {
      switch (get_common_type(bv)) {
        case Common_Types::t_int32:
          return std::to_string(get_as<int32_t>());
        case Common_Types::t_uint8:
          return std::to_string(get_as<uint32_t>());
        case Common_Types::t_int8:
          return std::to_string(get_as<int32_t>());
        case Common_Types::t_uint16:
          return std::to_string(get_as<uint16_t>());
        case Common_Types::t_int16:
          return std::to_string(get_as<int16_t>());
        case Common_Types::t_uint32:
          return std::to_string(get_as<uint32_t>());
        case Common_Types::t_uint64:
          return std::to_string(get_as<uint64_t>());
        case Common_Types::t_int64:
          return std::to_string(get_as<int64_t>());
        case Common_Types::t_double:
          return to_string_aux<double>(bv);
        case Common_Types::t_float:
          return to_string_aux<float>(bv);
        case Common_Types::t_long_double:
          return to_string_aux<long double>(bv);
      }

      throw chaiscript::detail::exception::bad_any_cast();
    }

    static void validate_boxed_number(const Boxed_Value &v) {
      const Type_Info &inp_ = v.get_type_info();
      if (inp_ == user_type<bool>()) {
        throw chaiscript::detail::exception::bad_any_cast();
      }

      if (!inp_.is_arithmetic()) {
        throw chaiscript::detail::exception::bad_any_cast();
      }
    }

    static bool equals(const Boxed_Number &t_lhs, const Boxed_Number &t_rhs) {
      return boxed_cast<bool>(oper(Operators::Opers::equals, t_lhs.bv, t_rhs.bv));
    }

    static bool less_than(const Boxed_Number &t_lhs, const Boxed_Number &t_rhs) {
      return boxed_cast<bool>(oper(Operators::Opers::less_than, t_lhs.bv, t_rhs.bv));
    }

    static bool greater_than(const Boxed_Number &t_lhs, const Boxed_Number &t_rhs) {
      return boxed_cast<bool>(oper(Operators::Opers::greater_than, t_lhs.bv, t_rhs.bv));
    }

    static bool greater_than_equal(const Boxed_Number &t_lhs, const Boxed_Number &t_rhs) {
      return boxed_cast<bool>(oper(Operators::Opers::greater_than_equal, t_lhs.bv, t_rhs.bv));
    }

    static bool less_than_equal(const Boxed_Number &t_lhs, const Boxed_Number &t_rhs) {
      return boxed_cast<bool>(oper(Operators::Opers::less_than_equal, t_lhs.bv, t_rhs.bv));
    }

    static bool not_equal(const Boxed_Number &t_lhs, const Boxed_Number &t_rhs) {
      return boxed_cast<bool>(oper(Operators::Opers::not_equal, t_lhs.bv, t_rhs.bv));
    }

    static Boxed_Number pre_decrement(Boxed_Number t_lhs) { return Boxed_Number(oper(Operators::Opers::pre_decrement, t_lhs.bv)); }

    static Boxed_Number pre_increment(Boxed_Number t_lhs) { return Boxed_Number(oper(Operators::Opers::pre_increment, t_lhs.bv)); }

    static const Boxed_Number sum(const Boxed_Number &t_lhs, const Boxed_Number &t_rhs) {
      return Boxed_Number(oper(Operators::Opers::sum, t_lhs.bv, t_rhs.bv));
    }

    static const Boxed_Number unary_plus(const Boxed_Number &t_lhs) { return Boxed_Number(oper(Operators::Opers::unary_plus, t_lhs.bv)); }

    static const Boxed_Number unary_minus(const Boxed_Number &t_lhs) { return Boxed_Number(oper(Operators::Opers::unary_minus, t_lhs.bv)); }

    static const Boxed_Number difference(const Boxed_Number &t_lhs, const Boxed_Number &t_rhs) {
      return Boxed_Number(oper(Operators::Opers::difference, t_lhs.bv, t_rhs.bv));
    }

    static Boxed_Number assign_bitwise_and(Boxed_Number t_lhs, const Boxed_Number &t_rhs) {
      return Boxed_Number(oper(Operators::Opers::assign_bitwise_and, t_lhs.bv, t_rhs.bv));
    }

    static Boxed_Number assign(Boxed_Number t_lhs, const Boxed_Number &t_rhs) {
      return Boxed_Number(oper(Operators::Opers::assign, t_lhs.bv, t_rhs.bv));
    }

    static Boxed_Number assign_bitwise_or(Boxed_Number t_lhs, const Boxed_Number &t_rhs) {
      return Boxed_Number(oper(Operators::Opers::assign_bitwise_or, t_lhs.bv, t_rhs.bv));
    }

    static Boxed_Number assign_bitwise_xor(Boxed_Number t_lhs, const Boxed_Number &t_rhs) {
      return Boxed_Number(oper(Operators::Opers::assign_bitwise_xor, t_lhs.bv, t_rhs.bv));
    }

    static Boxed_Number assign_remainder(Boxed_Number t_lhs, const Boxed_Number &t_rhs) {
      return Boxed_Number(oper(Operators::Opers::assign_remainder, t_lhs.bv, t_rhs.bv));
    }

    static Boxed_Number assign_shift_left(Boxed_Number t_lhs, const Boxed_Number &t_rhs) {
      return Boxed_Number(oper(Operators::Opers::assign_shift_left, t_lhs.bv, t_rhs.bv));
    }

    static Boxed_Number assign_shift_right(Boxed_Number t_lhs, const Boxed_Number &t_rhs) {
      return Boxed_Number(oper(Operators::Opers::assign_shift_right, t_lhs.bv, t_rhs.bv));
    }

    static const Boxed_Number bitwise_and(const Boxed_Number &t_lhs, const Boxed_Number &t_rhs) {
      return Boxed_Number(oper(Operators::Opers::bitwise_and, t_lhs.bv, t_rhs.bv));
    }

    static const Boxed_Number bitwise_complement(const Boxed_Number &t_lhs) {
      return Boxed_Number(oper(Operators::Opers::bitwise_complement, t_lhs.bv, Boxed_Value(0)));
    }

    static const Boxed_Number bitwise_xor(const Boxed_Number &t_lhs, const Boxed_Number &t_rhs) {
      return Boxed_Number(oper(Operators::Opers::bitwise_xor, t_lhs.bv, t_rhs.bv));
    }

    static const Boxed_Number bitwise_or(const Boxed_Number &t_lhs, const Boxed_Number &t_rhs) {
      return Boxed_Number(oper(Operators::Opers::bitwise_or, t_lhs.bv, t_rhs.bv));
    }

    static Boxed_Number assign_product(Boxed_Number t_lhs, const Boxed_Number &t_rhs) {
      return Boxed_Number(oper(Operators::Opers::assign_product, t_lhs.bv, t_rhs.bv));
    }

    static Boxed_Number assign_quotient(Boxed_Number t_lhs, const Boxed_Number &t_rhs) {
      return Boxed_Number(oper(Operators::Opers::assign_quotient, t_lhs.bv, t_rhs.bv));
    }

    static Boxed_Number assign_sum(Boxed_Number t_lhs, const Boxed_Number &t_rhs) {
      return Boxed_Number(oper(Operators::Opers::assign_sum, t_lhs.bv, t_rhs.bv));
    }
    static Boxed_Number assign_difference(Boxed_Number t_lhs, const Boxed_Number &t_rhs) {
      return Boxed_Number(oper(Operators::Opers::assign_difference, t_lhs.bv, t_rhs.bv));
    }

    static const Boxed_Number quotient(const Boxed_Number &t_lhs, const Boxed_Number &t_rhs) {
      return Boxed_Number(oper(Operators::Opers::quotient, t_lhs.bv, t_rhs.bv));
    }

    static const Boxed_Number shift_left(const Boxed_Number &t_lhs, const Boxed_Number &t_rhs) {
      return Boxed_Number(oper(Operators::Opers::shift_left, t_lhs.bv, t_rhs.bv));
    }

    static const Boxed_Number product(const Boxed_Number &t_lhs, const Boxed_Number &t_rhs) {
      return Boxed_Number(oper(Operators::Opers::product, t_lhs.bv, t_rhs.bv));
    }

    static const Boxed_Number remainder(const Boxed_Number &t_lhs, const Boxed_Number &t_rhs) {
      return Boxed_Number(oper(Operators::Opers::remainder, t_lhs.bv, t_rhs.bv));
    }

    static const Boxed_Number shift_right(const Boxed_Number &t_lhs, const Boxed_Number &t_rhs) {
      return Boxed_Number(oper(Operators::Opers::shift_right, t_lhs.bv, t_rhs.bv));
    }

    static Boxed_Value do_oper(Operators::Opers t_oper, const Boxed_Value &t_lhs, const Boxed_Value &t_rhs) {
      return oper(t_oper, t_lhs, t_rhs);
    }

    static Boxed_Value do_oper(Operators::Opers t_oper, const Boxed_Value &t_lhs) { return oper(t_oper, t_lhs); }

    Boxed_Value bv;
  };

  namespace detail {
    /// Cast_Helper for converting from Boxed_Value to Boxed_Number
    template<>
    struct Cast_Helper<Boxed_Number> {
      static Boxed_Number cast(const Boxed_Value &ob, const Type_Conversions_State *) { return Boxed_Number(ob); }
    };

    /// Cast_Helper for converting from Boxed_Value to Boxed_Number
    template<>
    struct Cast_Helper<const Boxed_Number &> : Cast_Helper<Boxed_Number> {
    };

    /// Cast_Helper for converting from Boxed_Value to Boxed_Number
    template<>
    struct Cast_Helper<const Boxed_Number> : Cast_Helper<Boxed_Number> {
    };
  } // namespace detail

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

#ifdef CHAISCRIPT_MSVC
#pragma warning(pop)
#endif

} // namespace chaiscript

#endif
