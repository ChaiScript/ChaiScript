#include <chaiscript/utility/utility.hpp>

#include <chaiscript/chaiscript_stdlib.hpp>

int main()
{

  chaiscript::ChaiScript chai(chaiscript::Std_Lib::library());

  chai.eval("def func() { print(\"Hello World\"); } ");

  std::function<void ()> f = chai.eval<std::function<void ()> >("func");
  f();

  if (chai.eval<std::function<std::string (int)> >("to_string")(6) != "6")
  {
    return EXIT_FAILURE;
  }

  if (chai.eval<std::function<std::string (const chaiscript::Boxed_Value &)> >("to_string")(chaiscript::var(6)) == "6")
  {
    return EXIT_SUCCESS;
  } else {
    return EXIT_FAILURE;
  }


}
