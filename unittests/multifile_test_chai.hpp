#include <chaiscript/chaiscript.hpp>

class Multi_Test_Chai
{
  public:
    Multi_Test_Chai();

    std::shared_ptr<chaiscript::ChaiScript> get_chai();

  private:
    std::shared_ptr<chaiscript::ChaiScript> m_chai;
};


