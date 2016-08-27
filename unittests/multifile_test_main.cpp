#include "multifile_test_chai.hpp"
#include "multifile_test_module.hpp"

#include <chaiscript/chaiscript_basic.hpp>

int main()
{
  Multi_Test_Chai chaitest;
  Multi_Test_Module chaimodule;

  std::shared_ptr<chaiscript::ChaiScript_Basic> chai = chaitest.get_chai();
  chai->add(chaimodule.get_module());
  return chai->eval<int>("get_module_value()");
}
