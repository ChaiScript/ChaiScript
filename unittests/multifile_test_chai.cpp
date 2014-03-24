#include "multifile_test_chai.hpp"

#include <chaiscript/chaiscript_stdlib.hpp>

Multi_Test_Chai::Multi_Test_Chai()
  : m_chai(new chaiscript::ChaiScript(chaiscript::Std_Lib::library()))
{
}


std::shared_ptr<chaiscript::ChaiScript> Multi_Test_Chai::get_chai()
{
  return m_chai;
}
