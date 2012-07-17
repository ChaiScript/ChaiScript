#include <chaiscript/chaiscript.hpp>


extern "C"
{
  int dosomething(int i)
  {
    return i % 2;
  }
}

int main()
{

  chaiscript::ChaiScript chai;
  chai.add(chaiscript::fun(&dosomething), "dosomething");

  return chai.eval<int>("dosomething(101)") == 101 % 2?EXIT_SUCCESS:EXIT_FAILURE;

}
