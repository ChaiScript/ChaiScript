
#include <chaiscript/chaiscript.hpp>
#include <string>

CHAISCRIPT_MODULE_EXPORT  chaiscript::ModulePtr create_chaiscript_module_reflection()
{
  chaiscript::ModulePtr m(new chaiscript::Module());

  /*
  CHAISCRIPT_CLASS( m, 
      ,
      (Test ())
      (Test (const Test &)),
      ((function))
      ((function2))
      ((function3))
      ((functionOverload)(std::string (Test::*)(double)))
      ((functionOverload)(std::string (Test::*)(int)))
    );
*/


  return m;
}
