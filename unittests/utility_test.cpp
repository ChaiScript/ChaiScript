#include <chaiscript/chaiscript.hpp>
#include <chaiscript/chaiscript_stdlib.hpp>
#include <chaiscript/utility/utility.hpp>
#include <functional>

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

  using namespace chaiscript;

  /// \todo fix overload resolution for fun<>
  chaiscript::utility::add_class<Test>(*m,
      "Test",
      { constructor<Test ()>(),
        constructor<Test (const Test &)>() },
      { {fun(&Test::function), "function"},
        {fun(&Test::function2), "function2"},
        {fun(&Test::function3), "function3"},
        {fun(static_cast<std::string(Test::*)(double)>(&Test::functionOverload)), "functionOverload" },
        {fun(static_cast<std::string(Test::*)(int)>(&Test::functionOverload)), "functionOverload" },
        {fun(static_cast<Test & (Test::*)(const Test &)>(&Test::operator=)), "=" }
 
        }
      );



  chaiscript::ChaiScript chai(chaiscript::Std_Lib::library());;
  chai.add(m);
  if (chai.eval<std::string>("auto t = Test(); t.function2(); ") == "Function2"
      && chai.eval<std::string>("auto t2 = Test(); t2.functionOverload(1); ") == "int"
      && chai.eval<std::string>("auto t3 = Test(); t3.functionOverload(1.1); ") == "double")
  {
    chai.eval("t = Test();");
    return EXIT_SUCCESS;
  } else {
    return EXIT_FAILURE;
  }

}
