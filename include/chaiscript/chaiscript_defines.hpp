// This file is distributed under the BSD License.
// See "license.txt" for details.
// Copyright 2009-2012, Jonathan Turner (jonathan@emptycrate.com)
// and Jason Turner (jason@emptycrate.com)
// http://www.chaiscript.com

#ifndef CHAISCRIPT_DEFINES_HPP_
#define CHAISCRIPT_DEFINES_HPP_

#ifdef _MSC_VER
#define CHAISCRIPT_MSVC _MSC_VER
#define CHAISCRIPT_HAS_DECLSPEC
#endif

#ifdef _WIN32
#define CHAISCRIPT_WINDOWS
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



#endif

