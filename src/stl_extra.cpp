
#include <chaiscript/chaiscript.hpp>
#include <string>


CHAISCRIPT_MODULE_EXPORT chaiscript::ModulePtr create_chaiscript_module_stl_extra()
{
  return chaiscript::bootstrap::list_type<std::list<chaiscript::Boxed_Value> >("List");
}
