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

#if defined(CHAISCRIPT_MSVC) ||  (defined(__GNUC__) && __GNUC__ > 4) || (__GNUC__ == 4 && __GNUC_MINOR__ >= 8) || (defined(__llvm__) && !defined(CHAISCRIPT_LIBCPP))
/// \todo Make this support other compilers when possible
#define CHAISCRIPT_HAS_THREAD_LOCAL
#endif

#if defined(__llvm__)
#define CHAISCRIPT_CLANG
#endif


#ifdef  CHAISCRIPT_HAS_DECLSPEC
#define CHAISCRIPT_MODULE_EXPORT extern "C" __declspec(dllexport)
#else
#define CHAISCRIPT_MODULE_EXPORT extern "C" 
#endif

#ifdef _DEBUG
#define CHAISCRIPT_DEBUG true
#else
#define CHAISCRIPT_DEBUG false
#endif

#include <memory>

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

  template<typename Iter, typename Distance>
    Iter advance_copy(Iter iter, Distance distance) {
      std::advance(iter, static_cast<typename std::iterator_traits<Iter>::difference_type>(distance));
      return iter;
    }

}
#endif

