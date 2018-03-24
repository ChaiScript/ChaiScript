#ifndef MULTIFILE_TEST_CHAI_HPP
#define MULTIFILE_TEST_CHAI_HPP
#include <chaiscript/chaiscript_basic.hpp>

class Multi_Test_Chai
{
  public:
    Multi_Test_Chai();

    std::shared_ptr<chaiscript::ChaiScript_Basic> get_chai() const;

  private:
    std::shared_ptr<chaiscript::ChaiScript_Basic> m_chai{};
};


#endif // MULTIFILE_TEST_CHAI_HPP
