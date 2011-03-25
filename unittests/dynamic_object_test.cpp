#include <chaiscript/utility/utility.hpp>

template<typename LHS, typename RHS>
void assert_equal(const LHS &lhs, const RHS &rhs)
{
  if (lhs==rhs)
  {
    return;
  } else {
    std::cout << "Got: " << lhs << " expected " << rhs << std::endl;
    exit(EXIT_FAILURE);
  }
}

int main()
{

  chaiscript::ChaiScript chai;

  chai("attr bob::z; def bob::bob() { this.z = 10 }; var x = bob()");

  chaiscript::dispatch::Dynamic_Object &mydo = chai.eval<chaiscript::dispatch::Dynamic_Object &>("x");

  assert_equal(mydo.get_type_name(), "bob");

  assert_equal(chaiscript::boxed_cast<int>(mydo.get_attr("z")), 10);

  chai("x.z = 15");

  assert_equal(chaiscript::boxed_cast<int>(mydo.get_attr("z")), 15);

  int &z = chaiscript::boxed_cast<int&>(mydo.get_attr("z"));

  assert_equal(z, 15);

  z = 20;
  
  assert_equal(z, 20);

  assert_equal(chaiscript::boxed_cast<int>(chai("x.z")), 20);
  
  return EXIT_SUCCESS;

}
