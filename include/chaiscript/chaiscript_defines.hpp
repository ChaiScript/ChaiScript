// This file is distributed under the BSD License.
// See "license.txt" for details.
// Copyright 2009-2012, Jonathan Turner (jonathan@emptycrate.com)
// Copyright 2009-2016, Jason Turner (jason@emptycrate.com)
// http://www.chaiscript.com

#ifndef CHAISCRIPT_DEFINES_HPP_
#define CHAISCRIPT_DEFINES_HPP_

#ifdef _MSC_VER
#define CHAISCRIPT_STRINGIZE(x) "" #x
#define CHAISCRIPT_COMPILER_VERSION CHAISCRIPT_STRINGIZE(_MSC_FULL_VER)
#define CHAISCRIPT_MSVC _MSC_VER
#define CHAISCRIPT_HAS_DECLSPEC
#if _MSC_VER <= 1800
#define CHAISCRIPT_MSVC_12
#endif
#else
#define CHAISCRIPT_COMPILER_VERSION __VERSION__
#endif

#ifndef CHAISCRIPT_MSVC_12
#define CHAISCRIPT_HAS_MAGIC_STATICS
#endif

#include <vector>

#if defined( _LIBCPP_VERSION )
#define CHAISCRIPT_LIBCPP
#endif

#if defined(_WIN32) || defined(__CYGWIN__)
#define CHAISCRIPT_WINDOWS
#endif

#if defined(_WIN32)
#if defined(__llvm__)
#define CHAISCRIPT_COMPILER_NAME "clang(windows)"
#elif defined(__GNUC__)
#define CHAISCRIPT_COMPILER_NAME "gcc(mingw)"
#else
#define CHAISCRIPT_COMPILER_NAME "msvc"
#endif
#else
#if defined(__llvm__)
#define CHAISCRIPT_COMPILER_NAME "clang"
#elif defined(__GNUC__)
#define CHAISCRIPT_COMPILER_NAME "gcc"
#else
#define CHAISCRIPT_COMPILER_NAME "unknown"
#endif
#endif

#if (defined(CHAISCRIPT_MSVC) && !defined(CHAISCRIPT_MSVC_12)) ||  (defined(__GNUC__) && __GNUC__ > 4) || (__GNUC__ == 4 && __GNUC_MINOR__ >= 8) || (defined(__llvm__) && !defined(CHAISCRIPT_LIBCPP))
/// Currently only g++>=4.8 supports this natively
/// \todo Make this support other compilers when possible
#define CHAISCRIPT_HAS_THREAD_LOCAL
#endif

#if (defined(__GNUC__) && __GNUC__ == 4 && __GNUC_MINOR__ == 6)
#define CHAISCRIPT_GCC_4_6
#endif

#if defined(__llvm__)
#define CHAISCRIPT_CLANG
#endif

#if (defined(__GNUC__) && __GNUC__ > 4) || (__GNUC__ == 4 && __GNUC_MINOR__ >= 7) || defined(CHAISCRIPT_MSVC) || defined(CHAISCRIPT_CLANG)
#define CHAISCRIPT_OVERRIDE override
#else
#define CHAISCRIPT_OVERRIDE
#endif


#ifdef  CHAISCRIPT_HAS_DECLSPEC
#define CHAISCRIPT_MODULE_EXPORT extern "C" __declspec(dllexport)
#else
#define CHAISCRIPT_MODULE_EXPORT extern "C" 
#endif

#ifdef CHAISCRIPT_MSVC_12
#define CHAISCRIPT_NOEXCEPT throw()
#define CHAISCRIPT_CONSTEXPR 
#else
#define CHAISCRIPT_NOEXCEPT noexcept
#define CHAISCRIPT_CONSTEXPR constexpr
#endif

#ifdef _DEBUG
#define CHAISCRIPT_DEBUG true
#else
#define CHAISCRIPT_DEBUG false
#endif

#include <memory>
#include <string>
#include <cmath>

namespace chaiscript {
  static const int version_major = 5;
  static const int version_minor = 8;
  static const int version_patch = 2;

  static const char *compiler_version = CHAISCRIPT_COMPILER_VERSION;
  static const char *compiler_name = CHAISCRIPT_COMPILER_NAME;
  static const bool debug_build = CHAISCRIPT_DEBUG;

  template<typename B, typename D, typename ...Arg>
  inline std::shared_ptr<B> make_shared(Arg && ... arg)
  {
#ifdef CHAISCRIPT_USE_STD_MAKE_SHARED
    return std::make_shared<D>(std::forward<Arg>(arg)...);
#else
    return std::shared_ptr<B>(static_cast<B*>(new D(std::forward<Arg>(arg)...)));
#endif
  }

  template<typename Iter, typename Distance>
    Iter advance_copy(Iter iter, Distance distance) {
      std::advance(iter, static_cast<typename std::iterator_traits<Iter>::difference_type>(distance));
      return iter;
    }


  template<typename T>
    auto parse_num(const char *t_str) -> typename std::enable_if<std::is_integral<T>::value, T>::type
    {
      T t = 0;
      for (char c = *t_str; (c = *t_str); ++t_str) {
        if (c < '0' || c > '9') {
          return t;
        }
        t *= 10;
        t += c - '0';
      }
      return t;
    }


  template<typename T>
    auto parse_num(const char *t_str) -> typename std::enable_if<!std::is_integral<T>::value, T>::type
    {
      T t = 0;
      T base = 0;
      T decimal_place = 0;
      bool exponent = false;
      bool neg_exponent = false;

      const auto final_value = [](const T val, const T baseval, const bool hasexp, const bool negexp) -> T {
        if (!hasexp) {
          return val;
        } else {
          return baseval * std::pow(T(10), val*T(negexp?-1:1));
        }
      };

      for(; *t_str != '\0'; ++t_str) {
        char c = *t_str;
        if (c == '.') {
          decimal_place = 10;
        } else if (c == 'e' || c == 'E') {
          exponent = true;
          decimal_place = 0;
          base = t;
          t = 0;
        } else if (c == '-' && exponent) {
          neg_exponent = true;
        } else if (c == '+' && exponent) {
          neg_exponent = false;
        } else if (c < '0' || c > '9') {
          return final_value(t, base, exponent, neg_exponent);
        } else if (decimal_place < T(10)) {
          t *= T(10);
          t += T(c - '0');
        } else {
          t += (T(c - '0') / (T(decimal_place)));
          decimal_place *= 10;
        }
      }

      return final_value(t, base, exponent, neg_exponent);
    }

  template<typename T>
    T parse_num(const std::string &t_str)
    {
      return parse_num<T>(t_str.c_str());
    }

}
#endif

