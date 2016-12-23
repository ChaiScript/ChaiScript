<a href="https://www.patreon.com/bePatron?u=2977989&redirect_uri=https%3A%2F%2Fwww.patreon.com%2Flefticus">
    <img height="40" width="204" src="https://s3-us-west-1.amazonaws.com/widget-images/become-patron-widget-medium%402x.png">
</a>


Master Status: [![Linux Build Status](https://travis-ci.org/ChaiScript/ChaiScript.png?branch=master)](https://travis-ci.org/ChaiScript/ChaiScript) [![Windows Build status](https://ci.appveyor.com/api/projects/status/6u3r4s81kkjqmsqw?svg=true)](https://ci.appveyor.com/project/lefticus/chaiscript) [![codecov.io](http://codecov.io/github/ChaiScript/ChaiScript/coverage.svg?branch=master)](http://codecov.io/github/ChaiScript/ChaiScript?branch=master)

Develop Status: [![Linux Build Status](https://travis-ci.org/ChaiScript/ChaiScript.png?branch=develop)](https://travis-ci.org/ChaiScript/ChaiScript) [![Windows Build status](https://ci.appveyor.com/api/projects/status/6u3r4s81kkjqmsqw/branch/develop?svg=true)](https://ci.appveyor.com/project/lefticus/chaiscript/branch/develop) [![codecov.io](http://codecov.io/github/ChaiScript/ChaiScript/coverage.svg?branch=develop)](http://codecov.io/github/ChaiScript/ChaiScript?branch=develop)

<a href="https://scan.coverity.com/projects/5297">
  <img alt="Coverity Scan Build Status"
       src="https://img.shields.io/coverity/scan/5297.svg"/>
</a>

[![badge](https://img.shields.io/badge/conan.io-ChaiScript%2F5.8.5-green.svg?logo=data:image/png;base64%2CiVBORw0KGgoAAAANSUhEUgAAAA4AAAAOCAMAAAAolt3jAAAA1VBMVEUAAABhlctjlstkl8tlmMtlmMxlmcxmmcxnmsxpnMxpnM1qnc1sn85voM91oM11oc1xotB2oc56pNF6pNJ2ptJ8ptJ8ptN9ptN8p9N5qNJ9p9N9p9R8qtOBqdSAqtOAqtR%2BrNSCrNJ/rdWDrNWCsNWCsNaJs9eLs9iRvNuVvdyVv9yXwd2Zwt6axN6dxt%2Bfx%2BChyeGiyuGjyuCjyuGly%2BGlzOKmzOGozuKoz%2BKqz%2BOq0OOv1OWw1OWw1eWx1eWy1uay1%2Baz1%2Baz1%2Bez2Oe02Oe12ee22ujUGwH3AAAAAXRSTlMAQObYZgAAAAFiS0dEAIgFHUgAAAAJcEhZcwAACxMAAAsTAQCanBgAAAAHdElNRQfgBQkREyOxFIh/AAAAiklEQVQI12NgAAMbOwY4sLZ2NtQ1coVKWNvoc/Eq8XDr2wB5Ig62ekza9vaOqpK2TpoMzOxaFtwqZua2Bm4makIM7OzMAjoaCqYuxooSUqJALjs7o4yVpbowvzSUy87KqSwmxQfnsrPISyFzWeWAXCkpMaBVIC4bmCsOdgiUKwh3JojLgAQ4ZCE0AMm2D29tZwe6AAAAAElFTkSuQmCC)](http://www.conan.io/source/ChaiScript/5.8.5/Manu343726/testing)

ChaiScript

http://www.chaiscript.com

(c) 2009-2012 Jonathan Turner
(c) 2009-2016 Jason Turner

Release under the BSD license, see "license.txt" for details.


Introduction
============

[![Gitter](https://badges.gitter.im/Join Chat.svg)](https://gitter.im/ChaiScript/ChaiScript?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)

ChaiScript is one of the only embedded scripting language designed from the 
ground up to directly target C++ and take advantage of modern C++ development 
techniques, working with the developer like he expects it to work.  Being a 
native C++ application, it has some advantages over existing embedded scripting 
languages:

1. It uses a header-only approach, which makes it easy to integrate with 
   existing projects.
2. It maintains type safety between your C++ application and the user scripts.
3. It supports a variety of C++ techniques including callbacks, overloaded 
   functions, class methods, and stl containers.


Requirements
============

ChaiScript requires a C++14 compiler to build with support for variadic 
templates.  It has been tested with gcc 4.9 and clang 3.6 (with libcxx). 
For more information see the build 
[dashboard](http://chaiscript.com/ChaiScript-BuildResults/index.html).

Usage
=====

* Add the ChaiScript include directory to your project's header search path
* Add `#include <chaiscript/chaiscript.hpp>` to your source file
* Instantiate the ChaiScript engine in your application.  For example, create a 
  new engine with the name `chai` like so: `chaiscript::ChaiScript chai`
* The default behavior is to load the ChaiScript standard library from a 
  loadable module. A second option is to compile the library into your code,
  see below for an example.

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



Or, if you want to compile the std lib into your code, which reduces
runtime requirements.

    /// main.cpp

    #include <chaiscript/chaiscript.hpp>
    #include <chaiscript/chaiscript_stdlib.hpp>

    double function(int i, double j)
    {
      return i * j;
    }

    int main()
    {
      chaiscript::ChaiScript chai(chaiscript::Std_Lib::library());
      chai.add(chaiscript::fun(&function), "function");

      double d = chai.eval<double>("function(3, 4.75);");
    }


