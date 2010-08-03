#include <chaiscript/utility/utility.hpp>

int main()
{

  chaiscript::ChaiScript chai;

  chai.eval("def func() { print(\"Hello World\"); } ");

  boost::function<void ()> f = chai.functor<void ()>("func");

  f();

  return EXIT_SUCCESS;

}
