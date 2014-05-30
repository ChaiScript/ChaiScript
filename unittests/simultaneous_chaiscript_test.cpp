#include <chaiscript/chaiscript.hpp>

int do_something(int i)
{
  return i + 2;
}

int do_something_else(int i)
{
  return i * 2;
}



int main()
{
  chaiscript::ChaiScript chai;
  chai.add(chaiscript::fun(&do_something), "do_something");
  chai.add(chaiscript::var(1), "i");

  for (int i = 0; i < 10; ++i)
  {
    chaiscript::ChaiScript chai2;
    chai2.add(chaiscript::fun(&do_something_else), "do_something_else");

    std::stringstream ss;
    ss << i;

    if (chai.eval<int>("do_something(" + ss.str() + ")") != i + 2)
    {
      return EXIT_FAILURE;
    }

    if (chai2.eval<int>("do_something_else(" + ss.str() + ")") != i * 2)
    {
      return EXIT_FAILURE;
    }

    try {
      chai2.eval("do_something(1)");
      return EXIT_FAILURE; // should not get here
    } catch (const chaiscript::exception::eval_error &) {
      // nothing to do, expected case
    }

    try {
      chai2.eval("i");
      return EXIT_FAILURE; // should not get here
    } catch (const chaiscript::exception::eval_error &) {
      // nothing to do, expected case
    }

    try {
      chai.eval("do_something_else(1)");
      return EXIT_FAILURE; // should not get here
    } catch (const chaiscript::exception::eval_error &) {
      // nothing to do, expected case
    }
  }
}
