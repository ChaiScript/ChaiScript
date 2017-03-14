// This file is distributed under the BSD License.
// See "license.txt" for details.
// Copyright 2009-2012, Jonathan Turner (jonathan@emptycrate.com)
// Copyright 2009-2017, Jason Turner (jason@emptycrate.com)
// http://www.chaiscript.com

#ifndef CHAISCRIPT_DEFINES_HPP_
#define CHAISCRIPT_DEFINES_HPP_

#ifdef _MSC_VER
#define CHAISCRIPT_STRINGIZE(x) "" #x
#define CHAISCRIPT_STRINGIZE_EXPANDED(x) CHAISCRIPT_STRINGIZE(x)
#define CHAISCRIPT_COMPILER_VERSION CHAISCRIPT_STRINGIZE_EXPANDED(_MSC_FULL_VER)
#define CHAISCRIPT_MSVC _MSC_VER
#define CHAISCRIPT_HAS_DECLSPEC

static_assert(_MSC_FULL_VER >= 190024210, "Visual C++ 2015 Update 3 or later required");

#else
#define CHAISCRIPT_COMPILER_VERSION __VERSION__
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


#if defined(__llvm__)
#define CHAISCRIPT_CLANG
#endif


#ifdef  CHAISCRIPT_HAS_DECLSPEC
#define CHAISCRIPT_MODULE_EXPORT extern "C" __declspec(dllexport)
#else
#define CHAISCRIPT_MODULE_EXPORT extern "C" 
#endif

#if defined(CHAISCRIPT_MSVC) || (defined(__GNUC__) && __GNUC__ >= 5) || defined(CHAISCRIPT_CLANG)
#define CHAISCRIPT_UTF16_UTF32
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
  static const int version_major = 6;
  static const int version_minor = 0;
  static const int version_patch = 0;

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

  struct Build_Info {
    static int version_major()
    {
      return chaiscript::version_major;
    }

    static int version_minor()
    {
      return chaiscript::version_minor;
    }

    static int version_patch()
    {
      return chaiscript::version_patch;
    }

    static std::string version()
    {
      return std::to_string(version_major()) + '.' + std::to_string(version_minor()) + '.' + std::to_string(version_patch());
    }

    static std::string compiler_id()
    {
      return compiler_name() + '-' + compiler_version();
    }

    static std::string build_id()
    {
      return compiler_id() + (debug_build()?"-Debug":"-Release");
    }

    static std::string compiler_version()
    {
      return chaiscript::compiler_version;
    }

    static std::string compiler_name()
    {
      return chaiscript::compiler_name;
    }

    static bool debug_build()
    {
      return chaiscript::debug_build;
    }
  };


  template<typename T>
    auto parse_num(const char *t_str) -> typename std::enable_if<std::is_integral<T>::value, T>::type
    {
      T t = 0;
      bool is_negative = false;
      for (char c = *t_str; (c = *t_str) != 0; ++t_str) {
        if (c == '-') {
          is_negative = true;
        } else if (c < '0' || c > '9') {
          return is_negative ? -t : t;
        } else {
          t *= 10;
          t += c - '0';
        }
      }
      return is_negative ? -t : t;
    }


  template<typename T>
    auto parse_num(const char *t_str) -> typename std::enable_if<!std::is_integral<T>::value, T>::type
    {
      T t = 0;
      T base = 0;
      T decimal_place = 0;
      bool exponent = false;
      bool neg_exponent = false;
      bool is_negative = false;

      const auto final_value = [](const T val, const bool neg, const T baseval, const bool hasexp, const bool negexp) -> T {
        if (!hasexp) {
          return neg ? -val : val;
        } else {
          return (T(neg?-1:1)) * baseval * std::pow(T(10), val*T(negexp?-1:1));
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
        } else if (c == '-') {
          if (exponent) {
            neg_exponent = true;
          } else {
            is_negative = true;
          }
        } else if (c == '+' && exponent) {
          neg_exponent = false;
        } else if (c < '0' || c > '9') {
          return final_value(t, is_negative, base, exponent, neg_exponent);
        } else if (decimal_place < T(10)) {
          t *= T(10);
          t += T(c - '0');
        } else {
          t += (T(c - '0') / (T(decimal_place)));
          decimal_place *= 10;
        }
      }

      return final_value(t, is_negative, base, exponent, neg_exponent);
    }

  template<typename T>
    T parse_num(const std::string &t_str)
    {
      return parse_num<T>(t_str.c_str());
    }

  enum class Options
  {
    No_Load_Modules,
    Load_Modules,
    No_External_Scripts,
    External_Scripts
  };

  static inline std::vector<Options> default_options()
  {
    return {Options::Load_Modules, Options::External_Scripts};
  }
}
#endif

