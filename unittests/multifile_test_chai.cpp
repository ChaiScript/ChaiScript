#include "multifile_test_chai.hpp"

#include <chaiscript/chaiscript_stdlib.hpp>
#include <chaiscript/language/chaiscript_parser.hpp>

Multi_Test_Chai::Multi_Test_Chai()
  : m_chai(new chaiscript::ChaiScript_Basic(chaiscript::Std_Lib::library(),
           std::make_unique<chaiscript::parser::ChaiScript_Parser<chaiscript::eval::Noop_Tracer, chaiscript::optimizer::Optimizer_Default>>()))
{
}


std::shared_ptr<chaiscript::ChaiScript_Basic> Multi_Test_Chai::get_chai()
{
  return m_chai;
}
