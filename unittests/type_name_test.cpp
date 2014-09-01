// Tests to make sure that the order in which function dispatches occur is correct

#include <chaiscript/chaiscript.hpp>
#include <cstdlib>

class MyClass
{
};

int main()
{
  chaiscript::ChaiScript chai;
  auto type = chaiscript::user_type<MyClass>();
  chai.add(type, "MyClass");

  if (chai.get_type_name(type) == "MyClass" && chai.get_type_name<MyClass>() == "MyClass")
  {
    return EXIT_SUCCESS;
  } else {
    return EXIT_FAILURE;
  }
}
