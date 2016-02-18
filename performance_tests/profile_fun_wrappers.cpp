#include <chaiscript/chaiscript.hpp>
#include <chaiscript/chaiscript_stdlib.hpp>

double f(const std::string &, double, bool) noexcept {
  return .0;
}

int main()
{
  chaiscript::ChaiScript chai(chaiscript::Std_Lib::library());

  chai.add(chaiscript::fun(&f), "f");

  const auto f = chai.eval<std::function<void ()>>(R"(fun(){ f("str", 1.2, false); })");

  for (int i = 0; i < 100000; ++i) {
    f();
  }

}
