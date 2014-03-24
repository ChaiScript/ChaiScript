// This file is distributed under the BSD License.
// See "license.txt" for details.
// Copyright 2009-2012, Jonathan Turner (jonathan@emptycrate.com)
// and Jason Turner (jason@emptycrate.com)
// http://www.chaiscript.com

#ifndef CHAISCRIPT_STDLIB_HPP_
#define CHAISCRIPT_STDLIB_HPP_

#include "chaiscript_defines.hpp"
#include "dispatchkit/bootstrap.hpp"
#include "dispatchkit/bootstrap_stl.hpp"

/// \file
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

        return lib;
      }

  };
}

#endif

