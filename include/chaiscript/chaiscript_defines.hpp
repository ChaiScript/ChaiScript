// This file is distributed under the BSD License.
// See "license.txt" for details.
// Copyright 2009-2012, Jonathan Turner (jonathan@emptycrate.com)
// Copyright 2009-2014, Jason Turner (jason@emptycrate.com)
// http://www.chaiscript.com

#ifndef CHAISCRIPT_DEFINES_HPP_
#define CHAISCRIPT_DEFINES_HPP_

#ifdef _MSC_VER
#define CHAISCRIPT_MSVC _MSC_VER
#define CHAISCRIPT_HAS_DECLSPEC
#endif

#if defined(_WIN32) || defined(__CYGWIN__)
#define CHAISCRIPT_WINDOWS
#endif

#if (defined(__GNUC__) && __GNUC__ > 4) || (__GNUC__ == 4 && __GNUC_MINOR__ >= 8)
/// Currently only g++>=4.8supports this natively
/// \todo Make this support other compilers when possible
#define CHAISCRIPT_HAS_THREAD_LOCAL
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

namespace chaiscript {
  static const int version_major = 5;
  static const int version_minor = 4;
  static const int version_patch = 0;
}

#endif

