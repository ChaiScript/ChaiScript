// Tests to make sure that the order in which function dispatches occur is correct

#include <chaiscript/chaiscript.hpp>


int main()
{
  chaiscript::ChaiScript chai;
  
  try {
    chai.eval("throw(runtime_error(\"error\"));");
  } catch (const chaiscript::Boxed_Value &bv) {
    const std::exception &e = chaiscript::boxed_cast<const std::exception &>(bv);
    if (e.what() == std::string("error"))
    {
      return EXIT_SUCCESS;
    }
  }

  return EXIT_FAILURE;
}
