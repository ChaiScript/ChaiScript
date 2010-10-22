#include "multifile_test_chai.hpp"

Multi_Test_Chai::Multi_Test_Chai()
  : m_chai(new chaiscript::ChaiScript())
{
}


boost::shared_ptr<chaiscript::ChaiScript> Multi_Test_Chai::get_chai()
{
  return m_chai;
}
