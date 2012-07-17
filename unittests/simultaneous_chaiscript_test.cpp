#include <chaiscript/chaiscript.hpp>

int dosomething(int i)
{
  return i + 2;
}

int dosomethingelse(int i)
{
  return i * 2;
}



int main()
{
  chaiscript::ChaiScript chai;
  chai.add(chaiscript::fun(&dosomething), "dosomething");
  chai.add(chaiscript::var(1), "i");

  for (int i = 0; i < 10; ++i)
  {
    chaiscript::ChaiScript chai2;
    chai2.add(chaiscript::fun(&dosomethingelse), "dosomethingelse");

    std::stringstream ss;
    ss << i;

    if (chai.eval<int>("dosomething(" + ss.str() + ")") != i + 2)
    {
      return EXIT_FAILURE;
    }

    if (chai2.eval<int>("dosomethingelse(" + ss.str() + ")") != i * 2)
    {
      return EXIT_FAILURE;
    }

    try {
      chai2.eval("dosomething(1)");
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
      chai.eval("dosomethingelse(1)");
      return EXIT_FAILURE; // should not get here
    } catch (const chaiscript::exception::eval_error &) {
      // nothing to do, expected case
    }
  }
}
