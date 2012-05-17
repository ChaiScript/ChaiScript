#include <chaiscript/chaiscript.hpp>

class Multi_Test_Chai
{
  public:
    Multi_Test_Chai();

    boost::shared_ptr<chaiscript::ChaiScript> get_chai();

  private:
    boost::shared_ptr<chaiscript::ChaiScript> m_chai;
};


