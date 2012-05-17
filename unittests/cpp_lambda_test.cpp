#include <chaiscript/utility/utility.hpp>

#include <chaiscript/chaiscript_stdlib.hpp>

int main()
{

  // We cannot deduce the type of a lambda expression, you must either wrap it
  // in an std::function or provide the signature
  

  chaiscript::ChaiScript chai(chaiscript::Std_Lib::library());

  // provide the signature
  chai.add(chaiscript::fun<std::string ()>([] { return "hello"; } ), "f1");

  // wrap
  chai.add(chaiscript::fun(std::function<std::string ()>([] { return "world"; } )), "f2");

  if (chai.eval<std::string>("f1()") == "hello"
      && chai.eval<std::string>("f2()") == "world")
  {
    return EXIT_SUCCESS;
  } else {
    return EXIT_FAILURE;
  }



}
