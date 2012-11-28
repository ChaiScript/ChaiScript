// Tests to make sure that type conversions happen only when they should

#include <chaiscript/chaiscript.hpp>

void f1(int)
{
}

void f4(std::string)
{
}

void f2(int)
{
}

void f3(double)
{
}


int main()
{
  chaiscript::ChaiScript chai;

  chai.add(chaiscript::fun(&f1), "f1");
  chai.add(chaiscript::fun(&f2), "f2");
  chai.add(chaiscript::fun(&f3), "f2");
  chai.add(chaiscript::fun(&f1), "f3");
  chai.add(chaiscript::fun(&f4), "f3");

  // no overloads
  chai.eval("f1(0)");
  chai.eval("f1(0l)");
  chai.eval("f1(0ul)");
  chai.eval("f1(0ll)");
  chai.eval("f1(0ull)");
  chai.eval("f1(0.0)");
  chai.eval("f1(0.0f)");
  chai.eval("f1(0.0l)");

  // expected overloads
  chai.eval("f2(1)");
  chai.eval("f2(1.0)");

  // 1 non-arithmetic overload
  chai.eval("f2(1.0)");

  // this is the one call we expect to fail
  try {
    chai.eval("f2(1.0l)");
  } catch (const std::exception &) {
    return EXIT_SUCCESS;
  }

  // if the last one did not throw, we failed
  return EXIT_FAILURE;
}
