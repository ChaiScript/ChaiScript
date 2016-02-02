// This file is distributed under the BSD License.
// See "license.txt" for details.
// Copyright 2009-2012, Jonathan Turner (jonathan@emptycrate.com)
// and Jason Turner (jason@emptycrate.com)
// http://www.chaiscript.com

#ifndef CHAISCRIPT_STDLIB_HPP_
#define CHAISCRIPT_STDLIB_HPP_

#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "chaiscript_defines.hpp"
#include "dispatchkit/dispatchkit.hpp"
#include "dispatchkit/bootstrap.hpp"
#include "dispatchkit/bootstrap_stl.hpp"
#include "dispatchkit/boxed_value.hpp"
#include "language/chaiscript_prelude.chai"
#include "utility/json_wrap.hpp"

#ifndef CHAISCRIPT_NO_THREADS
#include <future>
#endif


/// @file
///
/// This file generates the standard library that normal ChaiScript usage requires.

namespace chaiscript
{
  class Std_Lib
  {
    public:

      static ModulePtr library()
      {
        using namespace bootstrap;

        ModulePtr lib = Bootstrap::bootstrap();

        lib->add(standard_library::vector_type<std::vector<Boxed_Value> >("Vector"));
        lib->add(standard_library::string_type<std::string>("string"));
        lib->add(standard_library::map_type<std::map<std::string, Boxed_Value> >("Map"));
        lib->add(standard_library::pair_type<std::pair<Boxed_Value, Boxed_Value > >("Pair"));

#ifndef CHAISCRIPT_NO_THREADS
        lib->add(standard_library::future_type<std::future<chaiscript::Boxed_Value>>("future"));
        lib->add(chaiscript::fun([](const std::function<chaiscript::Boxed_Value ()> &t_func){ return std::async(std::launch::async, t_func);}), "async");
#endif

        lib->add(json_wrap::library());

        lib->eval(ChaiScript_Prelude::chaiscript_prelude() /*, "standard prelude"*/ );

        return lib;
      }

  };
}

#endif

