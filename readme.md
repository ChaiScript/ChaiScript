ChaiScript

http://www.chaiscript.com

(c) 2009-2012 Jason Turner and Jonathan Turner

Release under the BSD license, see "license.txt" for details.

Introduction
============

ChaiScript is one of the only embedded scripting language designed from the 
ground up to directly target C++ and take advantage of modern C++ development 
techniques, working with the developer like he expects it to work.  Being a 
native C++ application, it has some advantages over existing embedded scripting 
languages:

1) It uses a header-only approach, which makes it easy to integrate with 
   existing projects.
2) It maintains type safety between your C++ application and the user scripts.
3) It supports a variety of C++ techniques including callbacks, overloaded 
   functions, class methods, and stl containers.

Requirements
============

ChaiScript requires a recent version of Boost (http://www.boost.org) to build.

Usage
=====

* Add the ChaiScript include directory to your project's header search path
* Add `#include <chaiscript/chaiscript.hpp>` to your source file
* Instantiate the ChaiScript engine in your application.  For example, create 
  a new engine with the name `chai` like so: `chaiscript::ChaiScript chai`

Once instantiated, the engine is ready to start running ChaiScript source.  You 
have two main options for processing ChaiScript source: a line at a time using 
`chai.eval(string)` and a file at a time using `chai.eval_file(fname)`

To make functions in your C++ code visible to scripts, they must be registered 
with the scripting engine.  To do so, call add:

    chai.add(chaiscript::fun(&my_function), "my_function_name");

Once registered the function will be visible to scripts as "my_function_name"

Examples
========

ChaiScript is similar to ECMAScript (aka JavaScript(tm)), but with some 
modifications to make it easier to use.  For usage examples see the "samples" 
directory, and for more in-depth look at the language, the unit tests in the 
"unittests" directory cover the most ground.

For examples of how to register parts of your C++ application, see 
"example.cpp" in the "src" directory. Example.cpp is verbose and shows every 
possible way of working with the library. For further documentation generate 
the doxygen documentation in the build folder or see the website 
http://www.chaiscript.com.


The shortest complete example possible follows:

    /// main.cpp

    #include <chaiscript/chaiscript.hpp>

    double function(int i, double j)
    {
      return i * j;
    }

    int main()
    {
      chaiscript::ChaiScript chai;
      chai.add(chaiscript::fun(&function), "function");

      double d = chai.eval<double>("function(3, 4.75);");
    }


