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
      ((operator=))
    );

  chaiscript::ChaiScript chai;
  chai.add(m);
  if (chai.eval<std::string>("var t = Test(); t.function2(); ") == "Function2"
      && chai.eval<std::string>("var t2 = Test(); t2.functionOverload(1); ") == "int"
      && chai.eval<std::string>("var t3 = Test(); t3.functionOverload(1.1); ") == "double")
  {
    chai.eval("t = Test();");
    return EXIT_SUCCESS;
  } else {
    return EXIT_FAILURE;
  }

}
