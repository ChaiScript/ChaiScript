#include <chaiscript/utility/utility.hpp>

class Test {
 public:
  Test() : value_(5) {}

  short get_value() { return value_; }

  short value_;
};

int main()
{

  chaiscript::ChaiScript chai;
  chai.add(chaiscript::user_type<Test>(), "Test");
  chai.add(chaiscript::constructor<Test()>(), "Test");

  chai.add(chaiscript::fun(&Test::get_value), "get_value");

  chai.eval("var t := Test();");

  if (chai.eval<bool>("t.get_value() == 5"))
  {
    return EXIT_SUCCESS;
  } else {
    return EXIT_FAILURE;
  }
}
