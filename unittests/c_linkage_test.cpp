#include <chaiscript/chaiscript_basic.hpp>
#include "../static_libs/chaiscript_parser.hpp"
#include "../static_libs/chaiscript_stdlib.hpp"


extern "C"
{
  int do_something(int i)
  {
    return i % 2;
  }
}

int main()
{
  chaiscript::ChaiScript_Basic chai(create_chaiscript_stdlib(),create_chaiscript_parser());
  chai.add(chaiscript::fun(&do_something), "do_something");

  return chai.eval<int>("do_something(101)") == 101 % 2?EXIT_SUCCESS:EXIT_FAILURE;

}
