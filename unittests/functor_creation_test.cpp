#include <chaiscript/utility/utility.hpp>

int main()
{

  chaiscript::ChaiScript chai;

  chai.eval("def func() { print(\"Hello World\"); } ");

  boost::function<void ()> f = chai.functor<void ()>("func");
  f();

  if (chai.functor<std::string (int)>("to_string")(6) != "6")
  {
    return EXIT_FAILURE;
  }

  if (chai.functor<std::string (const chaiscript::Boxed_Value &)>("to_string")(chaiscript::var(6)) == "6")
  {
    return EXIT_SUCCESS;
  } else {
    return EXIT_FAILURE;
  }


}
