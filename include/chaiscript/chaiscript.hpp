// This file is distributed under the BSD License.
// See "license.txt" for details.
// Copyright 2009-2011, Jonathan Turner (jonathan@emptycrate.com)
// and Jason Turner (jason@emptycrate.com)
// http://www.chaiscript.com

#ifndef CHAISCRIPT_HPP_
#define CHAISCRIPT_HPP_



/// \mainpage
/// <a href="http://www.chaiscript.com">ChaiScript</a> is a scripting language designed specifically for integration with C++. It provides
/// seamless integration with C++ on all levels, including shared_ptr objects, functors and exceptions.
/// 
/// The parts of the ChaiScript API that the average user will be concerned with are contained in the 
/// chaiscript namespace and the chaiscript::ChaiScript class.
///
/// The end user parts of the API are extremely simple both in size and ease of use.
///
/// Currently, all source control and project management aspects of ChaiScript occur on <a href="http://www.github.com">github</a>.
///
/// \sa chaiscript
/// \sa chaiscript::ChaiScript
/// \sa http://www.chaiscript.com
/// \sa http://www.github.com/ChaiScript/ChaiScript


/// \namespace chaiscript
/// \brief Namespace chaiscript contains every API call that the average user will be concerned with.

/// \namespace chaiscript::detail
/// \brief Classes and functions reserved for internal use. Items in this namespace are not supported.

#include "dispatchkit/dispatchkit.hpp"
#include "dispatchkit/bootstrap.hpp"
#include "dispatchkit/bootstrap_stl.hpp"
#include "dispatchkit/function_call.hpp"
#include "dispatchkit/dynamic_object.hpp"
#include "dispatchkit/boxed_pod_value.hpp"

#ifdef  BOOST_HAS_DECLSPEC
#define CHAISCRIPT_MODULE_EXPORT extern "C" __declspec(dllexport)
#else
#define CHAISCRIPT_MODULE_EXPORT extern "C" 
#endif

#include "language/chaiscript_eval.hpp"
#include "language/chaiscript_engine.hpp"

#endif /* CHAISCRIPT_HPP_ */
