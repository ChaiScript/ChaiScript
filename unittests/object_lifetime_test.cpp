#include <chaiscript/utility/utility.hpp>

class Test
{
  public:
    Test()
    {
      ++count();
    }

    Test(const Test &)
    {
      ++count();
    }

    ~Test()
    {
      --count();
    }

    static int& count()
    {
      static int c = 0;
      return c;
    }
};

int main()
{
  chaiscript::ModulePtr m = chaiscript::ModulePtr(new chaiscript::Module());

  CHAISCRIPT_CLASS( m, 
      Test,
      (Test ())
      (Test (const Test &)),
      ((count))
    );

  chaiscript::ChaiScript chai;
  chai.add(m);
//  chai.add(chaiscript::fun(&Test::count), "count");

  int count = chai.eval<int>("count()");

  int count2 = chai.eval<int>("var i = 0; { var t = Test(); } return i;");

  int count3 = chai.eval<int>("i = 0; { var t = Test(); i = count(); } return i;");

  int count4 = chai.eval<int>("i = 0; { var t = Test(); { var t2 = Test(); i = count(); } } return i;");
 
  int count5 = chai.eval<int>("i = 0; { var t = Test(); { var t2 = Test(); } i = count(); } return i;");

  int count6 = chai.eval<int>("i = 0; { var t = Test(); { var t2 = Test(); }  } i = count(); return i;");

  if (count == 0
      && count2 == 0
      && count3 == 1
      && count4 == 2
      && count5 == 1
      && count6 == 0)
  {
    return EXIT_SUCCESS;
  } else {
    return EXIT_FAILURE;
  }
}
