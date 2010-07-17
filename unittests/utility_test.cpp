#include <chaiscript/utility/utility.hpp>

class Test
{
  public:
    void function() {}
    std::string function2() { return "Function2"; }
    void function3() {}
    std::string functionOverload(double) { return "double"; }
    std::string functionOverload(int) { return "int"; }
};

int main()
{

  chaiscript::ModulePtr m = chaiscript::ModulePtr(new chaiscript::Module());

  CHAISCRIPT_CLASS( m, 
      Test,
      (Test ())
      (Test (const Test &)),
      ((function))
      ((function2))
      ((function3))
      ((functionOverload)(std::string (Test::*)(double)))
      ((functionOverload)(std::string (Test::*)(int)))
    );

  chaiscript::ChaiScript chai;
  chai.add(m);
  if (chai.eval<std::string>("var t = Test(); t.function2(); ") == "Function2"
      && chai.eval<std::string>("var t = Test(); t.functionOverload(1); ") == "int"
      && chai.eval<std::string>("var t = Test(); t.functionOverload(1.1); ") == "double")
  {
    return EXIT_SUCCESS;
  } else {
    return EXIT_FAILURE;
  }

}
