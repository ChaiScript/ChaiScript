#include <chaiscript/chaiscript.hpp>


extern "C"
{
  int do_something(int i)
  {
    return i % 2;
  }
}

int main()
{

  chaiscript::ChaiScript chai;
  chai.add(chaiscript::fun(&do_something), "do_something");

  return chai.eval<int>("do_something(101)") == 101 % 2?EXIT_SUCCESS:EXIT_FAILURE;

}
