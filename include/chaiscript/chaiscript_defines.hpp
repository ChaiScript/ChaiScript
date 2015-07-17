// This file is distributed under the BSD License.
// See "license.txt" for details.
// Copyright 2009-2012, Jonathan Turner (jonathan@emptycrate.com)
// Copyright 2009-2015, Jason Turner (jason@emptycrate.com)
// http://www.chaiscript.com

#ifndef CHAISCRIPT_DEFINES_HPP_
#define CHAISCRIPT_DEFINES_HPP_

#ifdef _MSC_VER
#define CHAISCRIPT_MSVC _MSC_VER
#define CHAISCRIPT_HAS_DECLSPEC
#if _MSC_VER <= 1800
#define CHAISCRIPT_MSVC_12
#endif
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

#if (defined(__GNUC__) && __GNUC__ > 4) || (__GNUC__ == 4 && __GNUC_MINOR__ >= 8) || (defined(__llvm__) && !defined(CHAISCRIPT_LIBCPP))  
/// Currently only g++>=4.8 supports this natively
/// \todo Make this support other compilers when possible
#define CHAISCRIPT_HAS_THREAD_LOCAL
#endif

#if (defined(__GNUC__) && __GNUC__ == 4 && __GNUC_MINOR__ == 6)
#define CHAISCRIPT_GCC_4_6
#endif

#if (defined(__GNUC__) && __GNUC__ > 4) || (__GNUC__ == 4 && __GNUC_MINOR__ >= 7) || defined(CHAISCRIPT_MSVC) || defined(__llvm__)
#define CHAISCRIPT_OVERRIDE override
#else
#define CHAISCRIPT_OVERRIDE
#endif


#ifdef  CHAISCRIPT_HAS_DECLSPEC
#define CHAISCRIPT_MODULE_EXPORT extern "C" __declspec(dllexport)
#else
#define CHAISCRIPT_MODULE_EXPORT extern "C" 
#endif

#ifdef CHAISCRIPT_MSVC
#define CHAISCRIPT_NOEXCEPT throw()
#define CHAISCRIPT_CONSTEXPR 
#else
#define CHAISCRIPT_NOEXCEPT noexcept
#define CHAISCRIPT_CONSTEXPR constexpr
#endif

#include <memory>

namespace chaiscript {
  static const int version_major = 5;
  static const int version_minor = 7;
  static const int version_patch = 1;

  template<typename B, typename D, typename ...Arg>
  inline std::shared_ptr<B> make_shared(Arg && ... arg)
  {
#ifdef CHAISCRIPT_USE_STD_MAKE_SHARED
    return std::make_shared<D>(std::forward<Arg>(arg)...);
#else
    return std::shared_ptr<B>(static_cast<B*>(new D(std::forward<Arg>(arg)...)));
#endif
  }

}
#endif

