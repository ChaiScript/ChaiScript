#include <chaiscript/chaiscript.hpp>


int myfun()
{
  return 2;
}

int main()
{

  chaiscript::ChaiScript chai;

  // save the initial state of globals and locals
  chaiscript::ChaiScript::State firststate = chai.get_state();
  std::map<std::string, chaiscript::Boxed_Value> locals = chai.get_locals();
  
  // add some new globals and locals
  chai.add(chaiscript::var(1), "i");

  chai.add(chaiscript::fun(&myfun), "myfun");


  bool didcall = chai.eval<int>("myfun()") == 2;

  bool hadi = chai.eval<int>("i") == 1;

  chai.set_state(firststate);

  // set state should have reverted the state of the functions and dropped
  // the 'myfun'
  bool didnotcall = false;

  try {
    chai.eval<int>("myfun()");
  } catch (const chaiscript::exception::eval_error &) {
    didnotcall = true;
  }

  // set state should not affect the local variables
  bool stillhasid = chai.eval<int>("i") == 1;

  // After resetting the locals we expect the 'i' to be gone
  chai.set_locals(locals);


  bool nolongerhasid = false;

  try {
    chai.eval<int>("i");
  } catch (const chaiscript::exception::eval_error &) {
    nolongerhasid = true;
  }

  if (didcall && hadi && didnotcall && stillhasid && nolongerhasid)
  {
    return EXIT_SUCCESS;
  } else {
    return EXIT_FAILURE;
  }
}
