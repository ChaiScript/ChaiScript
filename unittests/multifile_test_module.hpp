#include <chaiscript/chaiscript.hpp>

class Multi_Test_Module
{
  public:
    static int get_module_value();

    Multi_Test_Module();

    chaiscript::ModulePtr get_module();
};
