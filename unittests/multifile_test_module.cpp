#include <chaiscript/chaiscript_basic.hpp>

#include "multifile_test_module.hpp"

Multi_Test_Module::Multi_Test_Module() = default;

int Multi_Test_Module::get_module_value()
{
  return 0;
}

chaiscript::ModulePtr Multi_Test_Module::get_module() const {
  chaiscript::ModulePtr module(new chaiscript::Module());

  module->add(chaiscript::fun(&Multi_Test_Module::get_module_value), "get_module_value");

  return module;
}
