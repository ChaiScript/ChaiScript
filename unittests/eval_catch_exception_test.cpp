// Tests to make sure that the order in which function dispatches occur is correct

#include <chaiscript/chaiscript.hpp>

int test_generic()
{
  chaiscript::ChaiScript chai;
  
  try {
    chai.eval("throw(runtime_error(\"error\"));");
  } catch (const chaiscript::Boxed_Value &bv) {
    const std::exception &e = chaiscript::boxed_cast<const std::exception &>(bv);
    if (e.what() == std::string("error"))
    {
      return EXIT_SUCCESS;
    }
  }

  std::cout << "test_generic failed" << std::endl;
  return EXIT_FAILURE;
}

int test_1()
{
  chaiscript::ChaiScript chai;
  
  try {
    chai.eval("throw(1)", chaiscript::exception_specification<int>());
  } catch (int e) {
    if (e == 1)
    {
      return EXIT_SUCCESS;
    }
  }

  std::cout << "test_1 failed" << std::endl;
  return EXIT_FAILURE;
}

int test_2()
{
  chaiscript::ChaiScript chai;
  
  try {
    chai.eval("throw(1.0)", chaiscript::exception_specification<int, double>());
  } catch (const double e) {
    if (e == 1.0)
    {
      return EXIT_SUCCESS;
    }
  }

  std::cout << "test_2 failed" << std::endl;
  return EXIT_FAILURE;
}

int test_5()
{
  chaiscript::ChaiScript chai;

  try {
    chai.eval("throw(runtime_error(\"error\"))", chaiscript::exception_specification<int, double, float, const std::string &, const std::exception &>());
  } catch (const double) {
    std::cout << "test_5 failed with double" << std::endl;
    return EXIT_FAILURE;
  } catch (int) {
    std::cout << "test_5 failed with int" << std::endl;
    return EXIT_FAILURE;
  } catch (float) {
    std::cout << "test_5 failed with float" << std::endl;
    return EXIT_FAILURE;
  } catch (const std::string &) {
    std::cout << "test_5 failed with string" << std::endl;
    return EXIT_FAILURE;
  } catch (const std::exception &) {
    return EXIT_SUCCESS;
  }

  std::cout << "test_5 failed" << std::endl;
  return EXIT_FAILURE;
}

int test_unhandled()
{
  chaiscript::ChaiScript chai;

  try {
    chai.eval("throw(\"error\")", chaiscript::exception_specification<int, double, float, const std::exception &>());
  } catch (double) {
    std::cout << "test_unhandled failed with double" << std::endl;
    return EXIT_FAILURE;
  } catch (int) {
    std::cout << "test_unhandled failed with int" << std::endl;
    return EXIT_FAILURE;
  } catch (float) {
    std::cout << "test_unhandled failed with float" << std::endl;
    return EXIT_FAILURE;
  } catch (const std::exception &) {
    std::cout << "test_unhandled failed with std::exception" << std::endl;
    return EXIT_FAILURE;
  } catch (const chaiscript::Boxed_Value &) {
    return EXIT_SUCCESS;
  }

  std::cout << "test_unhandled failed" << std::endl;
  return EXIT_FAILURE;
}


int main()
{
  if (test_generic() == EXIT_SUCCESS
    && test_1() == EXIT_SUCCESS
    && test_2() == EXIT_SUCCESS
    && test_5() == EXIT_SUCCESS
    && test_unhandled() == EXIT_SUCCESS) 
  {
    return EXIT_SUCCESS;
  } else {
    return EXIT_FAILURE;
  }
}
