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
/// \section gettingstarted Getting Started
/// 
/// \subsection basics Basics
/// 
/// Basic simple example:
///
/// \code
/// #include <chaiscript/chaiscript.hpp>
/// 
/// double function(int i, double j)
/// {
///   return i * j;
/// }
///
/// int main()
/// {
///   chaiscript::ChaiScript chai;
///   chai.add(&function, "function");
///
///   double d = chai.eval<double>("function(3, 4.75);");
/// } 
/// \endcode
///
/// <hr>
/// \subsection eval Evaluating Scripts
///
/// Scripts can be evaluated with the () operator, eval method or eval_file method.
///
/// \subsubsection parenoperator () Operator
///
/// operator() can be used as a handy shortcut for evaluating ChaiScript snippets.
/// \code
/// chaiscript::ChaiScript chai;
/// chai("print(\"hello world\")");
/// \endcode
/// 
/// \sa chaiscript::ChaiScript::operator()(const std::string &)
///
/// \subsubsection evalmethod Method 'eval'
/// 
/// The eval method is somewhat more verbose and can be used to get typesafely return values
/// from the script.
///
/// \code
/// chaiscript::ChaiScript chai;
/// chai.eval("callsomefunc()");
/// int result = chai.eval<int>("1 + 3");
/// // result now equals 4
/// \endcode
///
/// \sa chaiscript::ChaiScript::eval
///
/// \subsubsection evalfilemethod Method 'eval_file'
/// 
/// The 'eval_file' method loads a file from disk and executes the script in it
/// 
/// \code
/// chaiscript::ChaiScript chai;
/// chai.eval_file("myfile.chai");
/// std::string result = chai.eval_file<std::string>("myfile.chai") // extract the last value returned from the file
/// \endcode
/// 
/// \sa chaiscript::ChaiScript::eval_file
///
/// <hr>
/// \subsection addingitems Adding Items to ChaiScript
///
/// ChaiScript supports 4 basic things that can be added: objects, functions, type infos and Modules
///
/// \subsubsection addingobjects Adding Objects
///
/// Named objects can be created with the #var function.
/// 
/// \code
/// using namespace chaiscript;
/// ChaiScript chai;
/// int i = 5;
/// chai.add(var(i), "i");
/// chai("print(i)");
/// \endcode
///
/// Immutable objects can be created with the chaiscript::const_var function.
///
/// \code
/// chai.add(const_var(i), "i");
/// chai("i = 5"); // exception throw, cannot assign const var
/// \endcode
/// 
/// Named variables can only be accessed from the context they are created in.
/// If you want a global variable, it must be const, and created with the 
/// chaiscript::ChaiScript::add_global_const function.
///
/// \code
/// chai.add_global_const(const_var(i), "i");
/// chai("def somefun() { print(i); }; sumfun();");
/// \endcode
/// 
/// \subsubsection addingfunctions Adding Functions
/// 
/// Functions, methods and members are all added using the same function: chaiscript::fun.
///
/// \code
/// using namespace chaiscript;
/// 
/// class MyClass {
///   public:
///     int memberdata;
///     void method();
///     void method2(int);
///     static void staticmethod();
///     void overloadedmethod();
///     void overloadedmethod(const std::string &);
/// };
/// 
/// ChaiScript chai;
/// chai.add(fun(&MyClass::memberdata), "memberdata");
/// chai.add(fun(&MyClass::method), "method");
/// chai.add(fun(&MyClass::staticmethod), "staticmethod");
/// \endcode
///
/// Overloaded methods will need some help, to hint the compiler as to which overload you want:
///
/// \code
/// chai.add(fun<void (MyClass::*)()>(&MyClass::overloadedmethod), "overloadedmethod"));
/// chai.add(fun<void (MyClass::*)(const std::string &)>(&MyClass::overloadedmethod, "overloadedmethod"));
/// \endcode
///
/// There are also shortcuts built into chaiscript::fun for binding up to the first two parameters of the function.
/// 
/// \code
/// MyClass obj;
/// chai.add(fun(&MyClass::method, &obj), "method");
/// chai("method()"); // equiv to obj.method()
/// chai.add(fun(&MyClass::method2, &obj, 3), "method2");
/// chai("method2()"); // equiv to obj.method2(3)
/// \endcode
///
/// \subsubsection addingtypeinfo Adding Type Info
///
/// ChaiScript will automatically support any type implicitly provided to it in the form
/// of objects and function parameters / return types. However, it can be nice to let ChaiScript 
/// know more details about the types you are giving it. For instance, the "clone" functionality
/// cannot work unless there is a copy constructor registered and the name of the type is known
/// (so that ChaiScript can look up the copy constructor).
///
/// Continuing with the example "MyClass" from above:
///
/// \code
/// chai.add(user_type<MyClass>(), "MyClass");
/// \endcode
///
/// \subsubsection addingmodules Adding Modules
/// 
/// Modules are holders for collections of ChaiScript registrations.
///
/// \code
/// ModulePtr module = get_sum_module();
/// chai.add(module);
/// \endcode
/// 
/// \sa chaiscript::Module
/// 
/// <hr>
/// \subsection helpermacro Class Helper Macro
/// 
/// Much of the work of adding new classes to ChaiScript can be reduced with the help
/// of the CHAISCRIPT_CLASS helper macro.
///
/// \code
/// class Test
/// {
///   public:
///     void function() {}
///     std::string function2() { return "Function2"; }
///     void function3() {}
///     std::string functionOverload(double) { return "double"; }
///     std::string functionOverload(int) { return "int"; }
/// };
///
/// int main()
/// {
///
///   chaiscript::ModulePtr m = chaiscript::ModulePtr(new chaiscript::Module());
///
///   CHAISCRIPT_CLASS( m, 
///       Test,
///       (Test ())
///       (Test (const Test &)),
///       ((function))
///       ((function2))
///       ((function3))
///       ((functionOverload)(std::string (Test::*)(double)))
///       ((functionOverload)(std::string (Test::*)(int)))
///       ((operator=))
///     );
/// 
///   chaiscript::ChaiScript chai;
///   chai.add(m);
/// }
/// \endcode
/// 
/// \sa \ref addingmodules
///
/// \subsection pointerconversions Pointer / Object Conversions
///
/// As much as possible, ChaiScript attempts to convert between &, *, const &, const *, boost::shared_ptr<T>,
/// boost::shared_ptr<const T> and boost::reference_wrapand value types automatically.
///
/// If a var object was created in C++ from a pointer, it cannot be convered to a shared_ptr (this would add invalid reference counting).
/// Const may be added, but never removed. 
///
/// The take away is that you can pretty much expect function calls to Just Work when you need them to.
///
/// \code
/// void fun1(const int *);
/// void fun2(int *);
/// void fun3(int);
/// void fun4(int &);
/// void fun5(const int &);
/// void fun5(boost::shared_ptr<int>);
/// void fun6(boost::shared_ptr<const int>);
/// void fun7(const boost::shared_ptr<int> &);
/// void fun8(const boost::shared_ptr<const int> &);
///
/// int main()
/// {
///   using namespace chaiscript
///   chaiscript::ChaiScript chai;
///   chai.add(fun(fun1), "fun1");
///   chai.add(fun(fun2), "fun2");
///   chai.add(fun(fun3), "fun3");
///   chai.add(fun(fun4), "fun4");
///   chai.add(fun(fun5), "fun5");
///   chai.add(fun(fun6), "fun6");
///   chai.add(fun(fun7), "fun7");
///   chai.add(fun(fun8), "fun8");
///
///   chai("var i = 10;");
///   chai("fun1(i)");
///   chai("fun2(i)");
///   chai("fun3(i)");
///   chai("fun4(i)");
///   chai("fun5(i)");
///   chai("fun6(i)");
///   chai("fun7(i)");
///   chai("fun8(i)");
/// }  add demo for reference_wrapper
/// \endcode
///
/// See the unit test unittests/
/// 
/// base classes
/// function objects
///
/// <hr>
/// 
/// \sa chaiscript
/// \sa chaiscript::ChaiScript
/// \sa http://www.chaiscript.com
/// \sa http://www.github.com/ChaiScript/ChaiScript

/// \page LangObjectSystemRef ChaiScript Language Object Model Reference
///
///
/// ChaiScript has an object system built in, for types defined within the ChaiScript system.
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
