// This file is distributed under the BSD License.
// See "license.txt" for details.
// Copyright 2009-2012, Jonathan Turner (jonathan@emptycrate.com)
// Copyright 2009-2016, Jason Turner (jason@emptycrate.com)
// http://www.chaiscript.com

#include <chaiscript/chaiscript.hpp>
#include <chaiscript/dispatchkit/bootstrap_stl.hpp>
#include <list>
#include <string>

// MSVC doesn't like that we are using C++ return types from our C declared module
// but this is the best way to do it for cross platform compatibility
#ifdef CHAISCRIPT_MSVC
#pragma warning(push)
#pragma warning(disable : 4190)
#endif

#ifdef __llvm__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wreturn-type-c-linkage"
#endif

CHAISCRIPT_MODULE_EXPORT chaiscript::ModulePtr create_chaiscript_module_stl_extra()
{

  auto module = chaiscript::bootstrap::standard_library::list_type<std::list<chaiscript::Boxed_Value> >("List");
  module->add(chaiscript::bootstrap::standard_library::vector_type<std::vector<uint16_t> >("u16vector"));
  module->add(chaiscript::vector_conversion<std::vector<uint16_t>>());
  return module;
}

#ifdef __llvm__
#pragma clang diagnostic pop
#endif

#ifdef CHAISCRIPT_MSVC
#pragma warning(pop)
#endif
