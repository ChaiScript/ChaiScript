#include <chaiscript/utility/utility.hpp>

int main()
{

  chaiscript::ChaiScript chai;

  chai.eval("def func() { print(\"Hello World\"); } ");

  boost::function<void ()> f = chai.eval<boost::function<void ()> >("func");
  f();

  if (chai.eval<boost::function<std::string (int)> >("to_string")(6) != "6")
  {
    return EXIT_FAILURE;
  }

  if (chai.eval<boost::function<std::string (const chaiscript::Boxed_Value &)> >("to_string")(chaiscript::var(6)) == "6")
  {
    return EXIT_SUCCESS;
  } else {
    return EXIT_FAILURE;
  }


}
