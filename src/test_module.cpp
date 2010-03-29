
#include <chaiscript/chaiscript.hpp>
#include <string>

std::string hello_world()
{
  return "Hello World";
}

CHAISCRIPT_MODULE_EXPORT  chaiscript::ModulePtr create_chaiscript_module_test_module()
{
  chaiscript::ModulePtr m(new chaiscript::Module());

  m->add(chaiscript::fun(hello_world), "hello_world");
  return m;
}
