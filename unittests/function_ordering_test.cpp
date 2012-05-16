// Tests to make sure that the order in which function dispatches occur is correct

#include <chaiscript/utility/utility.hpp>

int test_one(const int &)
{
  return 1;
}

int test_two(int &)
{
  return 2;
}

int main()
{
  chaiscript::ChaiScript chai;
  chai.eval("def test_fun(x) { return 3; }");
  chai.eval("def test_fun(x) : x == \"hi\" { return 4; }");
//  chai.eval("def test_fun(x) { return 5; }");
  chai.add(chaiscript::fun(&test_one), "test_fun");
  chai.add(chaiscript::fun(&test_two), "test_fun");

  int res1 = chai.eval<int>("test_fun(1)");
  int res2 = chai.eval<int>("var i = 1; test_fun(i)");
  int res3 = chai.eval<int>("test_fun(\"bob\")");
  int res4 = chai.eval<int>("test_fun(\"hi\")");

  if (res1 == 1
      && res2 == 2 
      && res3 == 3
      && res4 == 4 )
  {
    return EXIT_SUCCESS;
  } else {
    return EXIT_FAILURE;
  }
}
