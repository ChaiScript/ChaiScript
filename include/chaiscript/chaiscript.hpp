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

/// \page LangObjectSystemRef ChaiScript Language Object Model Reference
///
///
/// ChaiScript supports has an object system built in, for types defined within the ChaiScript system.
///
/// \code
/// attr Rectangle::height
/// attr Rectangle::width
/// def Rectangle::Rectangle() { this.height = 10; this.width = 20 }
/// def Rectangle::area() { this.height * this.width }
/// 
/// var rect = Rectangle()
/// rect.height = 30
/// print(rect.area())
/// \endcode
///
/// \sa \ref keywordattr
/// \sa \ref keyworddef


/// \page LangKeywordRef ChaiScript Language Keyword Reference
///
///
/// <hr>
/// \section keywordattr attr
/// Defines a ChaiScript object attribute
/// 
/// \code 
/// Attribute Definition ::= "attr" class_name "::" attribute_name
/// \endcode
///
/// \sa \ref LangObjectSystemRef
///
/// <hr>
/// \section keywordbreak break
/// Stops execution of a looping block.
///
/// \code
/// Break Statement ::= "break"
/// \endcode
///
/// \sa \ref keywordfor
/// \sa \ref keywordwhile 
///
///
/// <hr>
/// \section keyworddef def
/// Begins a function or method definition
///
/// \code
/// Function Definition ::= [annotation + CR/LF] "def" identifier "(" [arg ("," arg)*] ")" [":" guard] block
/// Method Definition ::= [annotation + CR/LF] "def" class_name "::" method_name "(" [arg ("," arg)*] ")" [":" guard] block
/// \endcode
/// 
/// annotation: meta-annotation on function, currently used as documentation. Optional.
/// identifier: name of function. Required.
/// args: comma-delimited list of parameter names. Optional.
/// guards: guarding statement that act as a prerequisite for the function. Optional.
/// { }: scoped block as function body. Required.
///
/// Functions return values in one of two ways:
///
/// By using an explicit return call, optionally passing the value to be returned.
/// By implicitly returning the value of the last expression (if it is not a while or for loop).
///
/// Method definitions for known types extend those types with new methods. This includes C++ and ChaiScript defined types.
/// Method definitions for unknown types implicitly define the named type.
///
/// \sa \ref LangObjectSystemRef
///
///
/// <hr>
/// \section keywordelse else
/// \sa \ref keywordif 
///
///
/// <hr>
/// \section keywordfor for
/// \code
/// For Block ::= "for" "(" [initial] ";" stop_condition ";" loop_expression ")" block
/// \endcode
/// This loop can be broken using the \ref keywordbreak command.
///
///
/// <hr>
/// \section keywordfun fun
/// Begins an anonymous function declaration (sometimes called a lambda).
///
/// \code
/// Lambda ::= "fun" "(" [variable] ("," variable)*  ")" block
/// \endcode
///
/// \b Examples:
///
/// \code
/// // Generate an anonymous function object that adds 2 to its parameter
/// var f = fun(x) { x + 2; }
/// \endcode
///
/// \sa \ref keyworddef for more details on ChaiScript functions
///
///
/// <hr>
/// \section keywordif if
/// Begins a conditional block of code that only executes if the condition evaluates as true.
/// \code
/// If Block ::= "if" "(" condition ")" block
/// Else If Block ::= "else if" "(" condition ")" block
/// Else Block ::= "else" block
/// \endcode
/// \b Example:
/// \code
/// if (true) {
///   // do something
/// } else if (false) {
///   // do something else
/// } else {
///   // otherwise do this
/// }
/// \endcode
///
///
/// <hr>
/// \section keywordtry try
/// \code
/// Try Block ::= "try" block 
///  ("catch" ["(" variable ")"] [":" guards] block)+ 
///    ["finally" block]
/// \endcode
///
/// \sa ChaiScript_Language::throw
///
/// <hr>
/// \section keywordwhile while
/// 
/// Begins a conditional block of code that loops 0 or more times, as long as the condition is true
///
/// \code
/// While Block ::= "while" "(" condition ")" block
/// \endcode
/// This loop can be broken using the \ref keywordbreak command.


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
