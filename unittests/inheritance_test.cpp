#include <chaiscript/utility/utility.hpp>

class Test
{
  public:
    std::string function() { return "Function"; }
};

class Test2 : public Test
{
  public:
    std::string function2() { return "Function2"; }
};

int main()
{

  chaiscript::ModulePtr m = chaiscript::ModulePtr(new chaiscript::Module());

  CHAISCRIPT_CLASS( m, 
      Test,
      (Test ())
      (Test (const Test &)),
      ((function))
    );

  CHAISCRIPT_CLASS( m, 
      Test2,
      (Test2 ())
      (Test2 (const Test2 &)),
      ((function2))
    );

  chaiscript::ChaiScript chai;

  m->add(chaiscript::dynamic_cast_conversion<Test2, Test>());


  chai.add(m);
  if (chai.eval<std::string>("var t = Test2(); t.function(); ") == "Function"
      && chai.eval<std::string>("var t = Test2(); t.function2(); ") == "Function2")
  {
    return EXIT_SUCCESS;
  } else {
    return EXIT_FAILURE;
  }

}
