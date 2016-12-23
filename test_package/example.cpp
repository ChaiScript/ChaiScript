#include <chaiscript/chaiscript.hpp>
#include <chaiscript/chaiscript_stdlib.hpp>

double function(int i, double j)
{
  return i * j;
}

int main()
{
  chaiscript::ChaiScript chai(chaiscript::Std_Lib::library());
  chai.add(chaiscript::fun(&function), "function");

  double d = chai.eval<double>("function(3, 4.75);");
}
