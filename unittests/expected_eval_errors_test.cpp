// Tests to make sure no arity, dispatch or guard errors leak up past eval

#include <chaiscript/utility/utility.hpp>

int test_one(const int &)
{
  return 1;
}

int main()
{
  chaiscript::ChaiScript chai;
  chai.add(chaiscript::fun(&test_one), "test_fun");

  chai.eval("def guard_fun(i) : i.get_type_info().is_type_arithmetic() {} ");

  bool eval_error = true;

  // Dot notation

  try {
    // non-existant function
    chai.eval("\"test\".test_one()");
    eval_error = false;
  } catch (const chaiscript::exception::eval_error &) {
  }

  try {
    // wrong parameter type
    chai.eval("\"test\".test_fun()");
    eval_error = false;
  } catch (const chaiscript::exception::eval_error &) {
  }

  try {
    // wrong number of parameters
    chai.eval("\"test\".test_fun(1)");
    eval_error = false;
  } catch (const chaiscript::exception::eval_error &) {
  }

  try {
    // guard failure
    chai.eval("\"test\".guard_fun(1)");
    eval_error = false;
  } catch (const chaiscript::exception::eval_error &) {
  }



  // regular notation

  try {
    // non-existant function
    chai.eval("test_one(\"test\")");
    eval_error = false;
  } catch (const chaiscript::exception::eval_error &) {
  }

  try {
    // wrong parameter type
    chai.eval("test_fun(\"test\")");
    eval_error = false;
  } catch (const chaiscript::exception::eval_error &) {
  }

  try {
    // wrong number of parameters
    chai.eval("test_fun(\"test\")");
    eval_error = false;
  } catch (const chaiscript::exception::eval_error &) {
  }

  try {
    // guard failure
    chai.eval("guard_fun(\"test\")");
    eval_error = false;
  } catch (const chaiscript::exception::eval_error &) {
  }


  // index operator
  try {
    chai.eval("var a = [1,2,3]; a[\"bob\"];");
    eval_error = false;
  } catch (const chaiscript::exception::eval_error &) {
  }

  // unary operator
  try {
    chai.eval("++\"bob\"");
    eval_error = false;
  } catch (const chaiscript::exception::eval_error &) {
  }

  // binary operator
  try {
    chai.eval("\"bob\" + 1");
    eval_error = false;
  } catch (const chaiscript::exception::eval_error &) {
  }


  if (eval_error)
  {
    return EXIT_SUCCESS;
  } else {
    return EXIT_FAILURE;
  }

}
