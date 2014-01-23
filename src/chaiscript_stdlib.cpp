
#include <chaiscript/chaiscript_stdlib.hpp>


// MSVC doesn't like that we are using C++ return types from our C declared module
// but this is the best way to do it for cross platform compatibility
#ifdef CHAISCRIPT_MSVC
#pragma warning(push)
#pragma warning(disable : 4190)
#endif

#ifdef __llvm__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wreturn-type-c-linkage"
#endif


CHAISCRIPT_MODULE_EXPORT  chaiscript::ModulePtr create_chaiscript_module_chaiscript_stdlib()
{
  return chaiscript::Std_Lib::library();
}


#ifdef __llvm__
#pragma clang diagnostic pop
#endif

#ifdef CHAISCRIPT_MSVC
#pragma warning(pop)
#endif
