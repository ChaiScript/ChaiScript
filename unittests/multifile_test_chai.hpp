#include <chaiscript/chaiscript_basic.hpp>

class Multi_Test_Chai
{
  public:
    Multi_Test_Chai();

    std::shared_ptr<chaiscript::ChaiScript_Basic> get_chai();

  private:
    std::shared_ptr<chaiscript::ChaiScript_Basic> m_chai;
};


