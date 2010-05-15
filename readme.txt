ChaiScript v2.3.3
http://www.chaiscript.com
(c) 2009-2010 Jason Turner and Jonathan Turner
Release under the BSD license, see "license.txt" for details.

[Introduction]

ChaiScript is one of the first (and perhaps only) embedded scripting language designed from the ground up to directly target C++.  Being a native C++ application, it has some advantages over existing embedded scripting languages:

1) It uses a header-only approach, which makes it easy to integrate with existing projects.
2) It maintains type safety between your C++ application and the user scripts.
3) It supports a variety of C++ techniques including callbacks, overloaded functions, class methods, and stl containers.

[Requirements]

ChaiScript requires a recent version of Boost (http://www.boost.org) to build.

[Usage]

* Add the ChaiScript include directory to your project's header search path
* Add "#include <chaiscript/chaiscript.hpp> to your source file
* Instantiate the ChaiScript engine in your application.  For example, create a new engine with the name 'chai' like so: "chaiscript::ChaiScript_Engine chai"

Once instantiated, the engine is ready to start running ChaiScript source.  You have two main options for processing ChaiScript source: a line at a time using "chai.evaluate_string(string)" and a file at a time using "chai.evaluate_file(fname)"

To make functions in your C++ code visible to scripts, they must be registered with the scripting engine.  To do so, call register_function:

dispatchkit::register_function(chai.get_eval_engine(), &my_function, "my_function_name");

Once registered the function will be visible to scripts as "my_function_name"

[Examples]

ChaiScript is similar to ECMAScript (aka JavaScript(tm)), but with some modifications to make it easier to use.  For usage examples see the "samples" directory, and for more in-depth look at the language, the unit tests in the "unittests" directory cover the most ground.

For example of how to register parts of your C++ application, see "example.cpp" in the "src" directory.
