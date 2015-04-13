#include <chaiscript/utility/utility.hpp>

class Test
{
  public:
    Test()
    {
      std::cout << "Test()\n";
      ++constructcount();
    }

    Test(const Test &)
    {
      std::cout << "Test(const Test &)\n";
      ++copycount();
    }

    Test(Test &&)
    {
      std::cout << "Test(Test &&)\n";
      ++movecount();
    }

    ~Test()
    {
      std::cout << "~Test()\n";
      ++destructcount();
    }

    static int& constructcount()
    {
      static int c = 0;
      return c;
    }

    static int& copycount()
    {
      static int c = 0;
      return c;
    }

    static int& movecount()
    {
      static int c = 0;
      return c;
    }

    static int& destructcount()
    {
      static int c = 0;
      return c;
    }
};

Test create()
{
  return Test();
}

int main()
{
  chaiscript::ModulePtr m = chaiscript::ModulePtr(new chaiscript::Module());

  m->add(chaiscript::user_type<Test>(), "Test");
  m->add(chaiscript::constructor<Test()>(), "Test");
  m->add(chaiscript::constructor<Test(const Test &)>(), "Test");
  m->add(chaiscript::fun(&create), "create");

  chaiscript::ChaiScript chai;
  chai.add(m);

  chai.eval(" { auto i = create(); } ");

  if (Test::destructcount() == 2 && Test::copycount() == 0 && Test::movecount() == 1 && Test::constructcount() == 1)
  {
    return EXIT_SUCCESS;
  } else {
    return EXIT_FAILURE;
  }
}
