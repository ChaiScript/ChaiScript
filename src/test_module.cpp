
#include <chaiscript/chaiscript.hpp>
#include <string>

std::string hello_world()
{
  return "Hello World";
}

#ifdef _MSC_VER
#define EXPORT __declspec(dllexport)
#else
#define EXPORT 
#endif

extern "C" 
{
EXPORT chaiscript::ModulePtr create_chaiscript_module_test()
  {
    chaiscript::ModulePtr m(new chaiscript::Module());

    m->add(chaiscript::fun(hello_world), "hello_world");
    return m;
  }
}
