#include <chaiscript/utility/utility.hpp>

double test_call(const boost::function<double (int)> &f, int val)
{
  return f(val);
}

int main()
{

  chaiscript::ChaiScript chai;
  
  chai.add(chaiscript::fun(&test_call), "test_call");

  chai.eval("def func(i) { return i * 3.5; };");
  double d = chai.eval<double>("test_call(func, 3)");
  
  if (d == 3 * 3.5)
  {
    return EXIT_SUCCESS;  
  } else {
    return EXIT_FAILURE;
  }

}
