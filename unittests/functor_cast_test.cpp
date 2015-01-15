#include <chaiscript/utility/utility.hpp>

int test_call(const std::function<int (int)> &f, int val)
{
  return f(val);
}

int main()
{

  chaiscript::ChaiScript chai;

  chai.add(chaiscript::fun(&test_call), "test_call");

  chai.eval("def func(i) { return i * 6; };");
  int d = chai.eval<int>("test_call(func, 3)");

  if (d == 3 * 6)
  {
    return EXIT_SUCCESS;
  } else {
    return EXIT_FAILURE;
  }

}
