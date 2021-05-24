// All of these are necessary because of catch.hpp. It's OK, they'll be
// caught in other cpp files if chaiscript causes them


#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4062 4242 4566 4640 4702 6330 28251)
#endif


#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunknown-pragmas"
#pragma GCC diagnostic ignored "-Wparentheses"
// This one is necessary for the const return non-reference test
#pragma GCC diagnostic ignored "-Wignored-qualifiers"
#endif


#include <chaiscript/chaiscript.hpp>
#include <chaiscript/chaiscript_basic.hpp>
#include <chaiscript/utility/utility.hpp>
#include <chaiscript/dispatchkit/bootstrap_stl.hpp>

#include "../static_libs/chaiscript_parser.hpp"
#include "../static_libs/chaiscript_stdlib.hpp"



#define CATCH_CONFIG_MAIN

#include <clocale>

#include "catch.hpp"

// lambda_tests
TEST_CASE("C++11 Lambdas Can Be Registered")
{

  // We cannot deduce the type of a lambda expression, you must either wrap it
  // in an std::function or provide the signature
  chaiscript::ChaiScript_Basic chai(create_chaiscript_stdlib(),create_chaiscript_parser());

  chai.add(chaiscript::fun([]()->std::string { return "hello"; } ), "f1");

  // wrap
  chai.add(chaiscript::fun(std::function<std::string ()>([] { return "world"; } )), "f2");

  CHECK(chai.eval<std::string>("f1()") == "hello");
  CHECK(chai.eval<std::string>("f2()") == "world");
}

TEST_CASE("Lambdas can return boolean")
{
  chaiscript::ChaiScript_Basic chai(create_chaiscript_stdlib(),create_chaiscript_parser());

  // check lambdas returning bool from chaiscript:
  std::function<bool ()> chai_function;

  CHECK_NOTHROW(chai_function = chai.eval<std::function<bool ()>>("fun() { 42 != 0 }"));

  bool result = false;
  CHECK_NOTHROW(result = chai_function());

  CHECK(result == true);

  // check lambdas returning bool from C++:
  auto cpp_function = [](int x) { return x == 42; };
  CHECK_NOTHROW(chai.add(chaiscript::fun(cpp_function), "cpp_function"));
  CHECK_NOTHROW(result = chai.eval<bool>("cpp_function(314)"));
  CHECK(result == false);
}

// dynamic_object tests
TEST_CASE("Dynamic_Object attributes can be shared with C++")
{
  chaiscript::ChaiScript_Basic chai(create_chaiscript_stdlib(),create_chaiscript_parser());

  chai("attr bob::z; def bob::bob() { this.z = 10 }; auto x = bob()");

  chaiscript::dispatch::Dynamic_Object &mydo = chai.eval<chaiscript::dispatch::Dynamic_Object &>("x");

  CHECK(mydo.get_type_name() == "bob");

  CHECK(chaiscript::boxed_cast<int>(mydo.get_attr("z")) == 10);

  chai("x.z = 15");

  CHECK(chaiscript::boxed_cast<int>(mydo.get_attr("z")) == 15);

  int &z = chaiscript::boxed_cast<int&>(mydo.get_attr("z"));

  CHECK(z == 15);

  z = 20;
  
  CHECK(z == 20);
  CHECK(chaiscript::boxed_cast<int>(chai("x.z")) == 20);
}


TEST_CASE("Function objects can be created from chaiscript functions")
{

  chaiscript::ChaiScript_Basic chai(create_chaiscript_stdlib(),create_chaiscript_parser());

  chai.eval("def func() { print(\"Hello World\"); } ");

  std::function<void ()> f = chai.eval<std::function<void ()> >("func");
  f();

  CHECK(chai.eval<std::function<std::string (int)> >("to_string")(6) == "6");
  CHECK(chai.eval<std::function<std::string (const chaiscript::Boxed_Value &)> >("to_string")(chaiscript::var(6)) == "6");
}

TEST_CASE("Function callbacks can be set, tested and cleared")
{
  struct MyObject {
    std::function<int(int)> callback;
  };

  using namespace chaiscript;
  ModulePtr m = chaiscript::ModulePtr(new chaiscript::Module());
  ChaiScript_Basic chai(create_chaiscript_stdlib(),create_chaiscript_parser());
  utility::add_class<MyObject>(*m,
      "MyObject",
      { constructor<MyObject ()>(),
        constructor<MyObject (const MyObject &)>() },
      { {fun(&MyObject::callback), "callback"},
        {fun(static_cast<MyObject & (MyObject::*)(const MyObject &)>(&MyObject::operator=)), "=" }
        }
      );
  m->add(fun([](dispatch::Assignable_Proxy_Function const& func) { return !func; }), "empty");
  m->add(fun([](dispatch::Assignable_Proxy_Function& func) { func.clear(); }), "clear");
  chai.add(m);

  // Function object set from C++
  MyObject test;
  test.callback = [](int i) { return i + 10; };
  chai.add(var(std::ref(test)), "test");
  CHECK(chai.eval<int>("!test.callback.empty ? test.callback(13) : -1") == 23);
  CHECK(chai.eval<int>("test.callback(9)") == 19);
  CHECK(chai.eval<bool>("test.callback.empty") == false);
  chai.eval("test.callback.clear");
  CHECK(chai.eval<bool>("test.callback.empty") == true);
  CHECK_THROWS(chai.eval<int>("test.callback(1)") == 11);

  // Function object set from ChaiScript
  chai.eval("auto o = MyObject()");
  CHECK(chai.eval<bool>("o.callback.empty") == true);
  chai.eval("o.callback = fun(int i) { return i + 10 }");
  CHECK(chai.eval<bool>("o.callback.empty") == false);
  CHECK(chai.eval<int>("!o.callback.empty ? o.callback(13) : -1") == 23);
  CHECK(chai.eval<int>("o.callback(9)") == 19);
  chai.eval("o.callback.clear");
  CHECK(chai.eval<bool>("o.callback.empty") == true);
  CHECK_THROWS(chai.eval<int>("o.callback(1)") == 11);

}

TEST_CASE("ChaiScript can be created and destroyed on heap")
{
  auto *chai = new chaiscript::ChaiScript_Basic(create_chaiscript_stdlib(),create_chaiscript_parser());
  delete chai;
}


///////// Arithmetic Conversions

// Tests to make sure that type conversions happen only when they should
void arithmetic_conversions_f1(int)
{
}

void arithmetic_conversions_f4(std::string)
{
}

void arithmetic_conversions_f2(int)
{
}

void arithmetic_conversions_f3(double)
{
}

void arithmetic_conversions_f_func_return(const std::function<unsigned int (unsigned long)> &f)
{
  // test the ability to return an unsigned with auto conversion
  f(4);
}

TEST_CASE("Test automatic arithmetic conversions")
{
  chaiscript::ChaiScript_Basic chai(create_chaiscript_stdlib(),create_chaiscript_parser());

  chai.add(chaiscript::fun(&arithmetic_conversions_f1), "f1");
  chai.add(chaiscript::fun(&arithmetic_conversions_f2), "f2");
  chai.add(chaiscript::fun(&arithmetic_conversions_f3), "f2");
  chai.add(chaiscript::fun(&arithmetic_conversions_f1), "f3");
  chai.add(chaiscript::fun(&arithmetic_conversions_f4), "f3");

  chai.add(chaiscript::fun(&arithmetic_conversions_f_func_return), "func_return");

  // no overloads
  chai.eval("f1(0)");
  chai.eval("f1(0l)");
  chai.eval("f1(0ul)");
  chai.eval("f1(0ll)");
  chai.eval("f1(0ull)");
  chai.eval("f1(0.0)");
  chai.eval("f1(0.0f)");
  chai.eval("f1(0.0l)");

  // expected overloads
  chai.eval("f2(1)");
  chai.eval("f2(1.0)");

  // 1 non-arithmetic overload
  chai.eval("f2(1.0)");

  // various options for returning with conversions from chaiscript
  chai.eval("func_return(fun(x) { return 5u; })");
  chai.eval("func_return(fun(x) { return 5; })");
  chai.eval("func_return(fun(x) { return 5.0f; })");

  CHECK_THROWS(chai.eval("f2(1.0l)"));
}


/////// Exception handling


TEST_CASE("Generic exception handling with C++")
{
  chaiscript::ChaiScript_Basic chai(create_chaiscript_stdlib(),create_chaiscript_parser());

  try {
    chai.eval("throw(runtime_error(\"error\"));");
    REQUIRE(false);
  } catch (const chaiscript::Boxed_Value &bv) {
    const std::exception &e = chai.boxed_cast<const std::exception &>(bv);
    CHECK(e.what() == std::string("error"));
  }
}

TEST_CASE("Throw an int")
{
  chaiscript::ChaiScript_Basic chai(create_chaiscript_stdlib(),create_chaiscript_parser());

  try {
    chai.eval("throw(1)", chaiscript::exception_specification<int>());
    REQUIRE(false);
  } catch (int e) {
    CHECK(e == 1);
  }
}

TEST_CASE("Throw int or double")
{
  chaiscript::ChaiScript_Basic chai(create_chaiscript_stdlib(),create_chaiscript_parser());

  try {
    chai.eval("throw(1.0)", chaiscript::exception_specification<int, double>());
    REQUIRE(false);
  } catch (const double e) {
    CHECK(e == Approx(1.0));
  }
}

TEST_CASE("Deduction of pointer return types")
{
  int val = 5;
  int *val_ptr = &val;
  auto &val_ptr_ref = val_ptr;
  const auto &val_ptr_const_ref = val_ptr;

  auto get_val_ptr = [&]() -> int * { return val_ptr; };
  auto get_val_const_ptr = [&]() -> int const * { return val_ptr; };
  auto get_val_ptr_ref = [&]() -> int *& { return val_ptr_ref; };
  auto get_val_ptr_const_ref = [&]() -> int * const & { return val_ptr_const_ref; };
//  auto get_val_const_ptr_const_ref = [ref=std::cref(val_ptr)]() -> int const * const & { return ref.get(); };

  chaiscript::ChaiScript_Basic chai(create_chaiscript_stdlib(), create_chaiscript_parser());
  chai.add(chaiscript::fun(get_val_ptr), "get_val_ptr");
  chai.add(chaiscript::fun(get_val_const_ptr), "get_val_const_ptr");
  chai.add(chaiscript::fun(get_val_ptr_ref), "get_val_ptr_ref");
  chai.add(chaiscript::fun(get_val_ptr_const_ref), "get_val_ptr_const_ref");
//  chai.add(chaiscript::fun(get_val_const_ptr_const_ref), "get_val_const_ptr_const_ref");

  CHECK(chai.eval<int *>("get_val_ptr()") == &val);
  CHECK(*chai.eval<int *>("get_val_ptr()") == val);
  CHECK(chai.eval<int const *>("get_val_const_ptr()") == &val);
  CHECK(*chai.eval<int const *>("get_val_const_ptr()") == val);

  // note that we cannot maintain the references here,
  // chaiscript internals cannot handle that, effectively pointer to pointer
  CHECK(chai.eval<int *>("get_val_ptr_ref()") == &val);
  CHECK(*chai.eval<int *>("get_val_ptr_ref()") == val);
  CHECK(chai.eval<int *>("get_val_ptr_const_ref()") == &val);
  CHECK(*chai.eval<int *>("get_val_ptr_const_ref()") == val);
//  CHECK(chai.eval<int const *>("get_val_const_ptr_const_ref()") == &val);
}


TEST_CASE("Throw a runtime_error")
{
  chaiscript::ChaiScript_Basic chai(create_chaiscript_stdlib(),create_chaiscript_parser());

  try {
    chai.eval("throw(runtime_error(\"error\"))", chaiscript::exception_specification<int, double, float, const std::string &, const std::exception &>());
    REQUIRE(false);
  } catch (const double) {
    REQUIRE(false);
  } catch (int) {
    REQUIRE(false);
  } catch (float) {
    REQUIRE(false);
  } catch (const std::string &) {
    REQUIRE(false);
  } catch (const std::exception &) {
    REQUIRE(true);
  }
}

TEST_CASE("Throw unhandled type")
{
  chaiscript::ChaiScript_Basic chai(create_chaiscript_stdlib(),create_chaiscript_parser());

  try {
    chai.eval("throw(\"error\")", chaiscript::exception_specification<int, double, float, const std::exception &>());
    REQUIRE(false);
  } catch (double) {
    REQUIRE(false);
  } catch (int) {
    REQUIRE(false);
  } catch (float) {
    REQUIRE(false);
  } catch (const std::exception &) {
    REQUIRE(false);
  } catch (const chaiscript::Boxed_Value &) {
    REQUIRE(true);
  }
}


///////////// Tests to make sure no arity, dispatch or guard errors leak up past eval


int expected_eval_errors_test_one(const int &)
{
  return 1;
}

TEST_CASE("No unexpected exceptions leak")
{
  chaiscript::ChaiScript_Basic chai(create_chaiscript_stdlib(),create_chaiscript_parser());
  chai.add(chaiscript::fun(&expected_eval_errors_test_one), "test_fun");

  chai.eval("def guard_fun(i) : i.get_type_info().is_type_arithmetic() {} ");

  //// Dot notation

  // non-existent function
  CHECK_THROWS_AS(chai.eval("\"test\".test_one()"), chaiscript::exception::eval_error);
  // wrong parameter type
  CHECK_THROWS_AS(chai.eval("\"test\".test_fun()"), chaiscript::exception::eval_error);

  // wrong number of parameters
  CHECK_THROWS_AS(chai.eval("\"test\".test_fun(1)"), chaiscript::exception::eval_error);

  // guard failure
  CHECK_THROWS_AS(chai.eval("\"test\".guard_fun()"), chaiscript::exception::eval_error);


  // regular notation

  // non-existent function
  CHECK_THROWS_AS(chai.eval("test_one(\"test\")"), chaiscript::exception::eval_error);

  // wrong parameter type
  CHECK_THROWS_AS(chai.eval("test_fun(\"test\")"), chaiscript::exception::eval_error);

  // wrong number of parameters
  CHECK_THROWS_AS(chai.eval("test_fun(\"test\")"), chaiscript::exception::eval_error);

  // guard failure
  CHECK_THROWS_AS(chai.eval("guard_fun(\"test\")"), chaiscript::exception::eval_error);


  // index operator
  CHECK_THROWS_AS(chai.eval("var a = [1,2,3]; a[\"bob\"];"), chaiscript::exception::eval_error);

  // unary operator
  CHECK_THROWS_AS(chai.eval("++\"bob\""), chaiscript::exception::eval_error);

  // binary operator
  CHECK_THROWS_AS(chai.eval("\"bob\" + 1"), chaiscript::exception::eval_error);


}


//////// Tests to make sure that the order in which function dispatches occur is correct

#include <chaiscript/utility/utility.hpp>

int function_ordering_test_one(const int &)
{
  return 1;
}

int function_ordering_test_two(int &)
{
  return 2;
}

TEST_CASE("Function ordering")
{
  chaiscript::ChaiScript_Basic chai(create_chaiscript_stdlib(),create_chaiscript_parser());
  chai.eval("def test_fun(x) { return 3; }");
  chai.eval("def test_fun(x) : x == \"hi\" { return 4; }");
//  chai.eval("def test_fun(x) { return 5; }");
  chai.add(chaiscript::fun(&function_ordering_test_one), "test_fun");
  chai.add(chaiscript::fun(&function_ordering_test_two), "test_fun");

  CHECK(chai.eval<int>("test_fun(1)") == 1);
  CHECK(chai.eval<int>("auto i = 1; test_fun(i)") == 2);
  CHECK(chai.eval<int>("test_fun(\"bob\")") == 3);
  CHECK(chai.eval<int>("test_fun(\"hi\")") == 4);
}




int functor_cast_test_call(const std::function<int (int)> &f, int val)
{
  return f(val);
}

TEST_CASE("Functor cast")
{

  chaiscript::ChaiScript_Basic chai(create_chaiscript_stdlib(),create_chaiscript_parser());

  chai.add(chaiscript::fun(&functor_cast_test_call), "test_call");

  chai.eval("def func(i) { return i * 6; };");
  int d = chai.eval<int>("test_call(func, 3)");

  CHECK(d == 3 * 6);
}

TEST_CASE("Non-ASCII characters in the middle of string")
{
  chaiscript::ChaiScript_Basic chai(create_chaiscript_stdlib(),create_chaiscript_parser());
  CHECK_THROWS_AS(chai.eval<std::string>("prin\xeft \"Hello World\""), chaiscript::exception::eval_error);
}

TEST_CASE("Non-ASCII characters in the beginning of string")
{
    chaiscript::ChaiScript_Basic chai(create_chaiscript_stdlib(),create_chaiscript_parser());
    CHECK_THROWS_AS(chai.eval<std::string>("\xefprint \"Hello World\""), chaiscript::exception::eval_error);
}

TEST_CASE("Non-ASCII characters in the end of string")
{
    chaiscript::ChaiScript_Basic chai(create_chaiscript_stdlib(),create_chaiscript_parser());
    CHECK_THROWS_AS(chai.eval<std::string>("print \"Hello World\"\xef"), chaiscript::exception::eval_error);
}

TEST_CASE("BOM in string")
{
    chaiscript::ChaiScript_Basic chai(create_chaiscript_stdlib(),create_chaiscript_parser());
    CHECK_THROWS_AS(chai.eval<std::string>("\xef\xbb\xbfprint \"Hello World\""), chaiscript::exception::eval_error);
}

int set_state_test_myfun()
{
  return 2;
}

TEST_CASE("Set and restore chai state")
{
  chaiscript::ChaiScript_Basic chai(create_chaiscript_stdlib(),create_chaiscript_parser());

  // save the initial state of globals and locals
  auto firststate = chai.get_state();
  std::map<std::string, chaiscript::Boxed_Value> locals = chai.get_locals();

  // add some new globals and locals
  chai.add(chaiscript::var(1), "i");

  chai.add(chaiscript::fun(&set_state_test_myfun), "myfun");


  CHECK(chai.eval<int>("myfun()") == 2);

  CHECK(chai.eval<int>("i") == 1);

  chai.set_state(firststate);

  // set state should have reverted the state of the functions and dropped
  // the 'myfun'

  CHECK_THROWS_AS(chai.eval<int>("myfun()"), chaiscript::exception::eval_error);

  // set state should not affect the local variables
  CHECK(chai.eval<int>("i") == 1);

  // After resetting the locals we expect the 'i' to be gone
  chai.set_locals(locals);

  CHECK_THROWS_AS(chai.eval<int>("i"), chaiscript::exception::eval_error);
}


//// Short comparisons

class Short_Comparison_Test {
 public:
  Short_Comparison_Test() : value_(5) {}

  short get_value() const { return value_; }

  short value_;
};

TEST_CASE("Short comparison with int")
{
  chaiscript::ChaiScript_Basic chai(create_chaiscript_stdlib(),create_chaiscript_parser());
  chai.add(chaiscript::user_type<Short_Comparison_Test>(), "Test");
  chai.add(chaiscript::constructor<Short_Comparison_Test()>(), "Test");

  chai.add(chaiscript::fun(&Short_Comparison_Test::get_value), "get_value");

  chai.eval("auto &t = Test();");

  CHECK(chai.eval<bool>("t.get_value() == 5"));
}




///// Test lookup of type names


class Type_Name_MyClass
{
};

TEST_CASE("Test lookup of type names")
{
  chaiscript::ChaiScript_Basic chai(create_chaiscript_stdlib(),create_chaiscript_parser());
  auto type = chaiscript::user_type<Type_Name_MyClass>();
  chai.add(type, "MyClass");

  CHECK(chai.get_type_name(type) == "MyClass");
  CHECK(chai.get_type_name<Type_Name_MyClass>() == "MyClass");
}


/////// make sure many chaiscript objects can exist simultaneously 


int simultaneous_chaiscript_do_something(int i)
{
  return i + 2;
}

int simultaneous_chaiscript_do_something_else(int i)
{
  return i * 2;
}



TEST_CASE("Simultaneous ChaiScript tests")
{
  chaiscript::ChaiScript_Basic chai(create_chaiscript_stdlib(),create_chaiscript_parser());
  chai.add(chaiscript::fun(&simultaneous_chaiscript_do_something), "do_something");
  chai.add(chaiscript::var(1), "i");

  for (int i = 0; i < 10; ++i)
  {
    chaiscript::ChaiScript_Basic chai2(create_chaiscript_stdlib(),create_chaiscript_parser());
    chai2.add(chaiscript::fun(&simultaneous_chaiscript_do_something_else), "do_something_else");

    CHECK(chai.eval<int>("do_something(" + std::to_string(i) + ")") == i + 2);
    CHECK(chai2.eval<int>("do_something_else(" + std::to_string(i) + ")") == i * 2);

    CHECK_THROWS_AS(chai2.eval("do_something(1)"), chaiscript::exception::eval_error);
    CHECK_THROWS_AS(chai2.eval("i"), chaiscript::exception::eval_error);
    CHECK_NOTHROW(chai2.eval("do_something_else(1)"));
  }
}



/////////////// test utility functions

class Utility_Test
{
  public:
    void function() {}
    std::string function2() { return "Function2"; }
    void function3() {}
    std::string functionOverload(double) { return "double"; }
    std::string functionOverload(int) { return "int"; }
};

TEST_CASE("Utility_Test utility class wrapper")
{

  chaiscript::ModulePtr m = chaiscript::ModulePtr(new chaiscript::Module());

  using namespace chaiscript;

  /// \todo fix overload resolution for fun<>
  chaiscript::utility::add_class<Utility_Test>(*m,
      "Utility_Test",
      { constructor<Utility_Test ()>(),
        constructor<Utility_Test (const Utility_Test &)>() },
      { {fun(&Utility_Test::function), "function"},
        {fun(&Utility_Test::function2), "function2"},
        {fun(&Utility_Test::function3), "function3"},
        {fun(static_cast<std::string(Utility_Test::*)(double)>(&Utility_Test::functionOverload)), "functionOverload" },
        {fun(static_cast<std::string(Utility_Test::*)(int)>(&Utility_Test::functionOverload)), "functionOverload" },
        {fun(static_cast<Utility_Test & (Utility_Test::*)(const Utility_Test &)>(&Utility_Test::operator=)), "=" }
 
        }
      );


  chaiscript::ChaiScript_Basic chai(create_chaiscript_stdlib(),create_chaiscript_parser());
  chai.add(m);

  CHECK(chai.eval<std::string>("auto t = Utility_Test(); t.function2(); ") == "Function2");
  CHECK(chai.eval<std::string>("auto t2 = Utility_Test(); t2.functionOverload(1); ") == "int");
  CHECK(chai.eval<std::string>("auto t3 = Utility_Test(); t3.functionOverload(1.1); ") == "double");
  chai.eval("t = Utility_Test();");

}


enum Utility_Test_Numbers
{
  ONE,
  TWO,
  THREE
};

void do_something_with_enum_vector(const std::vector<Utility_Test_Numbers> &v)
{
  CHECK(v.size() == 3);
  CHECK(v[0] == ONE);
  CHECK(v[1] == THREE);
  CHECK(v[2] == TWO);
}

TEST_CASE("Utility_Test utility class wrapper for enum")
{

  chaiscript::ModulePtr m = chaiscript::ModulePtr(new chaiscript::Module());

  using namespace chaiscript;

  chaiscript::utility::add_class<Utility_Test_Numbers>(*m,
      "Utility_Test_Numbers",
      { { ONE, "ONE" },
        { TWO, "TWO" },
        { THREE, "THREE" }

        }
      );


  chaiscript::ChaiScript_Basic chai(create_chaiscript_stdlib(),create_chaiscript_parser());
  chai.add(m);

  CHECK(chai.eval<Utility_Test_Numbers>("ONE ") == 0);
  CHECK(chai.eval<Utility_Test_Numbers>("TWO ") == 1);
  CHECK(chai.eval<Utility_Test_Numbers>("THREE ") == 2);

  CHECK(chai.eval<bool>("ONE == 0"));

  chai.add(chaiscript::fun(&do_something_with_enum_vector), "do_something_with_enum_vector");
  chai.add(chaiscript::vector_conversion<std::vector<Utility_Test_Numbers>>());
  CHECK_NOTHROW(chai.eval("var a = [ONE, TWO, THREE]"));
  CHECK_NOTHROW(chai.eval("do_something_with_enum_vector([ONE, THREE, TWO])"));
  CHECK_NOTHROW(chai.eval("[ONE]"));

  const auto v = chai.eval<std::vector<Utility_Test_Numbers>>("a");
  CHECK(v.size() == 3);
  CHECK(v.at(1) == TWO);

  CHECK(chai.eval<bool>("ONE == ONE"));
  CHECK(chai.eval<bool>("ONE != TWO"));
  CHECK_NOTHROW(chai.eval("var o = ONE; o = TWO"));


}


////// Object copy count test

class Object_Copy_Count_Test
{
  public:
    Object_Copy_Count_Test()
    {
      std::cout << "Object_Copy_Count_Test()\n";
      ++constructcount();
    }

    Object_Copy_Count_Test(const Object_Copy_Count_Test &)
    {
      std::cout << "Object_Copy_Count_Test(const Object_Copy_Count_Test &)\n";
      ++copycount();
    }

    Object_Copy_Count_Test(Object_Copy_Count_Test &&)
    {
      std::cout << "Object_Copy_Count_Test(Object_Copy_Count_Test &&)\n";
      ++movecount();
    }

    ~Object_Copy_Count_Test()
    {
      std::cout << "~Object_Copy_Count_Test()\n";
      ++destructcount();
    }

    static int& constructcount()
    {
      static int c = 0;
      return c;
    }

    static int& copycount()
    {
      static int c = 0;
      return c;
    }

    static int& movecount()
    {
      static int c = 0;
      return c;
    }

    static int& destructcount()
    {
      static int c = 0;
      return c;
    }
};

Object_Copy_Count_Test object_copy_count_create()
{
  return Object_Copy_Count_Test();
}

TEST_CASE("Object copy counts")
{
  chaiscript::ModulePtr m = chaiscript::ModulePtr(new chaiscript::Module());

  m->add(chaiscript::user_type<Object_Copy_Count_Test>(), "Object_Copy_Count_Test");
  m->add(chaiscript::constructor<Object_Copy_Count_Test()>(), "Object_Copy_Count_Test");
  m->add(chaiscript::constructor<Object_Copy_Count_Test(const Object_Copy_Count_Test &)>(), "Object_Copy_Count_Test");
  m->add(chaiscript::fun(&object_copy_count_create), "create");

  chaiscript::ChaiScript_Basic chai(create_chaiscript_stdlib(),create_chaiscript_parser());
  chai.add(m);

  chai.eval(" { auto i = create(); } ");

  CHECK(Object_Copy_Count_Test::copycount() == 0);
  CHECK(Object_Copy_Count_Test::constructcount() == 1);
  CHECK(Object_Copy_Count_Test::destructcount() == 2);
  CHECK(Object_Copy_Count_Test::movecount() == 1);
}


///////////////////// Object lifetime test 1


class Object_Lifetime_Test
{
  public:
    Object_Lifetime_Test()
    {
      ++count();
    }

    Object_Lifetime_Test(const Object_Lifetime_Test &)
    {
      ++count();
    }

    ~Object_Lifetime_Test()
    {
      --count();
    }

    static int& count()
    {
      static int c = 0;
      return c;
    }
};

TEST_CASE("Object lifetime tests")
{
  chaiscript::ModulePtr m = chaiscript::ModulePtr(new chaiscript::Module());

  m->add(chaiscript::user_type<Object_Lifetime_Test>(), "Object_Lifetime_Test");
  m->add(chaiscript::constructor<Object_Lifetime_Test()>(), "Object_Lifetime_Test");
  m->add(chaiscript::constructor<Object_Lifetime_Test(const Object_Lifetime_Test &)>(), "Object_Lifetime_Test");
  m->add(chaiscript::fun(&Object_Lifetime_Test::count), "count");

  chaiscript::ChaiScript_Basic chai(create_chaiscript_stdlib(),create_chaiscript_parser());
  chai.add(m);

  CHECK(chai.eval<int>("count()") == 0);
  CHECK(chai.eval<int>("auto i = 0; { auto t = Object_Lifetime_Test(); } return i;") == 0);
  CHECK(chai.eval<int>("i = 0; { auto t = Object_Lifetime_Test(); i = count(); } return i;") == 1);
  CHECK(chai.eval<int>("i = 0; { auto t = Object_Lifetime_Test(); { auto t2 = Object_Lifetime_Test(); i = count(); } } return i;") == 2);
  CHECK(chai.eval<int>("i = 0; { auto t = Object_Lifetime_Test(); { auto t2 = Object_Lifetime_Test(); } i = count(); } return i;") == 1);
  CHECK(chai.eval<int>("i = 0; { auto t = Object_Lifetime_Test(); { auto t2 = Object_Lifetime_Test(); }  } i = count(); return i;") == 0);
}



//// Object lifetime tests 2

template<typename T>
struct Object_Lifetime_Vector2
{
  Object_Lifetime_Vector2() : x(0), y(0) {}
  Object_Lifetime_Vector2(T px, T py) : x(px), y(py) {}
  Object_Lifetime_Vector2(const Object_Lifetime_Vector2& cp) noexcept : x(cp.x), y(cp.y) {}

  Object_Lifetime_Vector2& operator+=(const Object_Lifetime_Vector2& vec_r)
  {
    x += vec_r.x;
    y += vec_r.y;
    return *this;
  }

  Object_Lifetime_Vector2 operator+(const Object_Lifetime_Vector2& vec_r)
  {
    return Object_Lifetime_Vector2(*this += vec_r);
  }

  Object_Lifetime_Vector2 &operator=(const Object_Lifetime_Vector2& ver_r)
  {
    x = ver_r.x;
    y = ver_r.y;
    return *this;
  }


  T x;
  T y;
};

Object_Lifetime_Vector2<float> Object_Lifetime_Vector2_GetValue()
{
  return Object_Lifetime_Vector2<float>(10,15);
}

TEST_CASE("Object lifetime test 2")
{
  chaiscript::ChaiScript_Basic _script(create_chaiscript_stdlib(),create_chaiscript_parser());

  //Registering stuff
  _script.add(chaiscript::user_type<Object_Lifetime_Vector2<float>>(), "Object_Lifetime_Vector2f");
  _script.add(chaiscript::constructor<Object_Lifetime_Vector2<float> ()>(), "Object_Lifetime_Vector2f");
  _script.add(chaiscript::constructor<Object_Lifetime_Vector2<float> (float, float)>(), "Object_Lifetime_Vector2f");
  _script.add(chaiscript::constructor<Object_Lifetime_Vector2<float> (const Object_Lifetime_Vector2<float>&)>(), "Object_Lifetime_Vector2f");
  _script.add(chaiscript::fun(&Object_Lifetime_Vector2<float>::x), "x");
  _script.add(chaiscript::fun(&Object_Lifetime_Vector2<float>::y), "y");
  _script.add(chaiscript::fun(&Object_Lifetime_Vector2<float>::operator +), "+");
  _script.add(chaiscript::fun(&Object_Lifetime_Vector2<float>::operator +=), "+=");
  _script.add(chaiscript::fun(&Object_Lifetime_Vector2<float>::operator =), "=");
  _script.add(chaiscript::fun(&Object_Lifetime_Vector2_GetValue), "getValue");

  _script.eval(R"(
    var test = 0.0
    var test2 = Object_Lifetime_Vector2f(10,10)

    test = getValue().x
    print(test)
    print(test2.x)
    )");

  CHECK(_script.eval<std::string>("to_string(test)") == "10");
  CHECK(_script.eval<std::string>("to_string(test2.x)") == "10");

}




///// Non-polymorphic base class conversions
class Non_Poly_Base {};
class Non_Poly_Derived : public Non_Poly_Base {};
int myfunction(Non_Poly_Base *)
{
  return 2;
}

TEST_CASE("Test Derived->Base with non-polymorphic classes")
{
  chaiscript::ChaiScript_Basic chai(create_chaiscript_stdlib(),create_chaiscript_parser());
  chai.add(chaiscript::base_class<Non_Poly_Base, Non_Poly_Derived>());
  Non_Poly_Derived d;
  chai.add(chaiscript::var(&d), "d");
  chai.add(chaiscript::fun(&myfunction), "myfunction");
  CHECK(chai.eval<int>("myfunction(d)") == 2);
}


struct TestCppVariableScope
{
  void print()
  {
    std::cout << "Printed" << std::endl;
  }
};

TEST_CASE("Variable Scope When Calling From C++")
{
  chaiscript::ChaiScript_Basic chai(create_chaiscript_stdlib(),create_chaiscript_parser());
  chai.add(chaiscript::user_type<TestCppVariableScope>(), "Test");
  chai.add(chaiscript::constructor<TestCppVariableScope()>(), "Test");
  chai.add(chaiscript::fun(&TestCppVariableScope::print), "print");
  chai.eval(R"(var t := Test();

      def func()
      {
        t.print();
      }

      )");

  CHECK_THROWS(chai.eval("func()"));

  chai.eval("dump_object(t)");

  auto func = chai.eval<std::function<void()>>("func");
  CHECK_THROWS(func());
}

TEST_CASE("Variable Scope When Calling From C++ 2")
{
  chaiscript::ChaiScript_Basic chai(create_chaiscript_stdlib(),create_chaiscript_parser());
  chai.eval("var obj = 2;");
  auto func = chai.eval<std::function<void()>>("fun(){ return obj; }");
  CHECK_THROWS(func());
}

void ulonglong(unsigned long long i) {
  std::cout << i << '\n';
}


void longlong(long long i) {
  std::cout << i << '\n';
}

TEST_CASE("Test long long dispatch")
{
  chaiscript::ChaiScript_Basic chai(create_chaiscript_stdlib(),create_chaiscript_parser());
  chai.add(chaiscript::fun(&longlong), "longlong");
  chai.add(chaiscript::fun(&ulonglong), "ulonglong");
  chai.eval("longlong(15)");
  chai.eval("ulonglong(15)");
}


struct Returned_Converted_Config
{
  int num_iterations;
  int something_else;
  std::string a_string;
  std::function<int (const std::string &)> a_function;
};



TEST_CASE("Return of converted type from script")
{
  chaiscript::ChaiScript_Basic chai(create_chaiscript_stdlib(),create_chaiscript_parser());

  chai.add(chaiscript::constructor<Returned_Converted_Config ()>(), "Returned_Converted_Config");
  chai.add(chaiscript::fun(&Returned_Converted_Config::num_iterations), "num_iterations");
  chai.add(chaiscript::fun(&Returned_Converted_Config::something_else), "something_else");
  chai.add(chaiscript::fun(&Returned_Converted_Config::a_string), "a_string");
  chai.add(chaiscript::fun(&Returned_Converted_Config::a_function), "a_function");
  chai.add(chaiscript::vector_conversion<std::vector<Returned_Converted_Config>>());

  auto c = chai.eval<std::vector<Returned_Converted_Config>>(R"(
    var c = Returned_Converted_Config();

    c.num_iterations = 5;
    c.something_else = c.num_iterations * 2;
    c.a_string = "string";
    c.a_function = fun(s) { s.size(); }

    print("making vector");
    var v = [];
    print("adding config item");
    v.push_back_ref(c);
    print("returning vector");
    v;

  )");


  std::cout << typeid(decltype(c)).name() << std::endl;

  std::cout << "Info: " << c.size() << " " << &c[0] << std::endl;

  std::cout << "num_iterations " << c[0].num_iterations << '\n'
            << "something_else " << c[0].something_else << '\n'
            << "a_string " << c[0].a_string << '\n'
            << "a_function " << c[0].a_function("bob") << '\n';

  chai.add(chaiscript::user_type<Returned_Converted_Config>(), "Returned_Converted_Config");
}


int get_value_a(const std::map<std::string, int> &t_m)
{
  return t_m.at("a");
}


TEST_CASE("Map conversions")
{
  chaiscript::ChaiScript_Basic chai(create_chaiscript_stdlib(),create_chaiscript_parser());
  chai.add(chaiscript::map_conversion<std::map<std::string, int>>());
  chai.add(chaiscript::fun(&get_value_a), "get_value_a");

  const auto c = chai.eval<int>(R"(
    var m = ["a": 42];
    get_value_a(m);
  )");

  CHECK(c == 42);
}


TEST_CASE("Parse floats with non-posix locale")
{
#ifdef CHAISCRIPT_MSVC
  std::setlocale(LC_ALL, "en-ZA");
#else
  std::setlocale(LC_ALL, "en_ZA.utf8");
#endif
  chaiscript::ChaiScript_Basic chai(create_chaiscript_stdlib(),create_chaiscript_parser());
  const double parsed = chai.eval<double>("print(1.3); 1.3");
  CHECK(parsed == Approx(1.3));
  const std::string str = chai.eval<std::string>("to_string(1.3)");
  CHECK(str == "1.3");
}



bool FindBitmap(int &ox, int &oy, long) {
  ox = 1; 
  oy = 2; 
  return true;
}

TEST_CASE("Mismatched numeric types only convert necessary params")
{
  chaiscript::ChaiScript_Basic chai(create_chaiscript_stdlib(),create_chaiscript_parser());

  chai.add(chaiscript::fun(&FindBitmap), "FindBitmap");
  int x = 0;
  int y = 0;
  chai.add(chaiscript::var(&x), "x");
  chai.add(chaiscript::var(&y), "y");
  chai.eval( "if ( FindBitmap ( x, y, 0) ) { print(\"found at \" + to_string(x) + \", \" + to_string(y))}" );
  CHECK(x == 1);
  CHECK(y == 2);

}

TEST_CASE("type_conversion to bool")
{
  auto module = std::make_shared<chaiscript::Module>();
  struct T {
    operator bool() const { return true; }
  };
  module->add(chaiscript::type_conversion<T, bool>());
}

TEST_CASE("Make sure ChaiScript object still compiles / executes")
{
  chaiscript::ChaiScript chai;
}

struct Count_Tracer
{
  int count = 0;
  template<typename T>
    void trace(const chaiscript::detail::Dispatch_State &, const chaiscript::eval::AST_Node_Impl<T> *)
    {
      ++count;
    }
};


TEST_CASE("Test count tracer")
{
  using Parser_Type = chaiscript::parser::ChaiScript_Parser< chaiscript::eval::Tracer<Count_Tracer>, chaiscript::optimizer::Optimizer_Default >;

  chaiscript::ChaiScript_Basic chai(chaiscript::Std_Lib::library(),
      std::make_unique<Parser_Type>());

  Parser_Type &parser = dynamic_cast<Parser_Type &>(chai.get_parser());

  const auto count = parser.get_tracer().count;

  chai.eval("");

  CHECK(parser.get_tracer().count > count);
}


TEST_CASE("Test stdlib options")
{
  const auto test_has_external_scripts = [](chaiscript::ChaiScript_Basic &chai) { 
    CHECK_NOTHROW(chai.eval("`use`"));
    CHECK_NOTHROW(chai.eval("`eval_file`"));
  };

  const auto test_no_external_scripts = [](chaiscript::ChaiScript_Basic &chai) { 
    CHECK_THROWS(chai.eval("`use`"));
    CHECK_THROWS(chai.eval("`eval_file`"));
  };

  const auto test_has_load_modules = [](chaiscript::ChaiScript_Basic &chai) { 
    CHECK_NOTHROW(chai.eval("`load_module`"));
  };

  const auto test_no_load_modules = [](chaiscript::ChaiScript_Basic &chai) { 
    CHECK_THROWS(chai.eval("`load_module`"));
  };

  SECTION( "Defaults" ) {
    chaiscript::ChaiScript_Basic chai(create_chaiscript_stdlib(),create_chaiscript_parser());
    test_has_external_scripts(chai);
    test_has_load_modules(chai);
  }

  SECTION( "Load_Modules, External_Scripts" ) {
    chaiscript::ChaiScript_Basic chai(create_chaiscript_stdlib(),create_chaiscript_parser(), {}, {}, 
        {chaiscript::Options::Load_Modules, chaiscript::Options::External_Scripts} );
    test_has_external_scripts(chai);
    test_has_load_modules(chai);
  }

  SECTION( "No_Load_Modules, No_External_Scripts" ) {
    chaiscript::ChaiScript_Basic chai(create_chaiscript_stdlib(),create_chaiscript_parser(), {}, {}, 
        {chaiscript::Options::No_Load_Modules, chaiscript::Options::No_External_Scripts} );
    test_no_external_scripts(chai);
    test_no_load_modules(chai);
  }

  SECTION( "No_Load_Modules, Load_Modules" ) {
    chaiscript::ChaiScript_Basic chai(create_chaiscript_stdlib(),create_chaiscript_parser(), {}, {}, 
        {chaiscript::Options::No_Load_Modules, chaiscript::Options::Load_Modules} );
    test_no_external_scripts(chai);
    test_no_load_modules(chai);
  }

  SECTION( "No_External_Scripts, External_Scripts" ) {
    chaiscript::ChaiScript_Basic chai(create_chaiscript_stdlib(),create_chaiscript_parser(), {}, {}, 
        {chaiscript::Options::No_External_Scripts, chaiscript::Options::External_Scripts} );
    test_no_external_scripts(chai);
    test_no_load_modules(chai);
  }

  SECTION( "No_External_Scripts, Load_Modules" ) {
    chaiscript::ChaiScript_Basic chai(create_chaiscript_stdlib(),create_chaiscript_parser(), {}, {}, 
        {chaiscript::Options::No_External_Scripts, chaiscript::Options::Load_Modules} );
    test_no_external_scripts(chai);
    test_has_load_modules(chai);
  }

  SECTION( "External_Scripts, No_Load_Modules" ) {
    chaiscript::ChaiScript_Basic chai(create_chaiscript_stdlib(),create_chaiscript_parser(), {}, {}, 
        {chaiscript::Options::External_Scripts, chaiscript::Options::No_Load_Modules} );
    test_has_external_scripts(chai);
    test_no_load_modules(chai);
  }
}


void uservalueref(int &&)
{
}

void usemoveonlytype(std::unique_ptr<int> &&)
{
}


TEST_CASE("Pass r-value reference to func")
{
  chaiscript::ChaiScript_Basic chai(create_chaiscript_stdlib(),create_chaiscript_parser());

  chai.add(chaiscript::fun(&uservalueref), "uservalueref");
  chai.add(chaiscript::fun(&usemoveonlytype), "usemoveonlytype");

  chai.add(chaiscript::var(std::make_unique<int>(1)), "iptr");
  chai.eval("usemoveonlytype(iptr)");
}

TEST_CASE("Use unique_ptr")
{
  chaiscript::ChaiScript_Basic chai(create_chaiscript_stdlib(),create_chaiscript_parser());

  chai.add(chaiscript::fun([](int &i){ ++i; }), "inci");
  chai.add(chaiscript::fun([](int i){ ++i; }), "copyi");
  chai.add(chaiscript::fun([](int *i){ ++(*i); }), "derefi");
  chai.add(chaiscript::fun([](const std::unique_ptr<int> &i){ ++(*i); }), "constrefuniqptri");
  chai.add(chaiscript::fun([](std::unique_ptr<int> &i){ ++(*i); }), "refuniqptri");
  chai.add(chaiscript::fun([](std::unique_ptr<int> &&i){ ++(*i); }), "rvaluniqptri");
  chai.add(chaiscript::var(std::make_unique<int>(1)), "iptr");


  CHECK(chai.eval<int>("iptr") == 1);
  chai.eval("inci(iptr)");
  CHECK(chai.eval<int>("iptr") == 2);
  chai.eval("copyi(iptr)");
  CHECK(chai.eval<int>("iptr") == 2);
  chai.eval("derefi(iptr)");
  CHECK(chai.eval<int>("iptr") == 3);
  chai.eval("constrefuniqptri(iptr)");
  CHECK(chai.eval<int>("iptr") == 4);
  chai.eval("refuniqptri(iptr)");
  CHECK(chai.eval<int>("iptr") == 5);
  chai.eval("rvaluniqptri(iptr)");
  CHECK(chai.eval<int>("iptr") == 6);
}


class Unique_Ptr_Test_Class
{
  public:
    Unique_Ptr_Test_Class() = default;
    Unique_Ptr_Test_Class(const Unique_Ptr_Test_Class&) = default;
    Unique_Ptr_Test_Class(Unique_Ptr_Test_Class &&) = default;
    Unique_Ptr_Test_Class &operator=(const Unique_Ptr_Test_Class&) = default;
    Unique_Ptr_Test_Class &operator=(Unique_Ptr_Test_Class&&) = default;
    virtual ~Unique_Ptr_Test_Class() = default;

    int getI() const {return 5;}
};


std::unique_ptr<Unique_Ptr_Test_Class> make_Unique_Ptr_Test_Class()
{
  return std::make_unique<Unique_Ptr_Test_Class>();
}

TEST_CASE("Call methods through unique_ptr")
{
  chaiscript::ChaiScript_Basic chai(create_chaiscript_stdlib(),create_chaiscript_parser());

  chai.add(chaiscript::var(std::make_unique<Unique_Ptr_Test_Class>()), "uptr");
  chai.add(chaiscript::fun(make_Unique_Ptr_Test_Class), "make_Unique_Ptr_Test_Class");
  chai.add(chaiscript::fun(&Unique_Ptr_Test_Class::getI), "getI");
  CHECK(chai.eval<int>("uptr.getI()") == 5);
  CHECK(chai.eval<int>("var uptr2 = make_Unique_Ptr_Test_Class(); uptr2.getI()") == 5);
}


class Unique_Ptr_Test_Base_Class
{
  public:
    int getI() const {return 5;}
};

class Unique_Ptr_Test_Derived_Class : public Unique_Ptr_Test_Base_Class
{};

std::unique_ptr<Unique_Ptr_Test_Derived_Class> make_Unique_Ptr_Test_Derived_Class()
{
  return std::make_unique<Unique_Ptr_Test_Derived_Class>();
}

TEST_CASE("Call methods on base class through unique_ptr<derived>")
{
  chaiscript::ChaiScript_Basic chai(create_chaiscript_stdlib(),create_chaiscript_parser());

  chai.add(chaiscript::var(std::make_unique<Unique_Ptr_Test_Derived_Class>()), "uptr");
  chai.add(chaiscript::fun(make_Unique_Ptr_Test_Derived_Class), "make_Unique_Ptr_Test_Derived_Class");
  chai.add(chaiscript::fun(&Unique_Ptr_Test_Base_Class::getI), "getI");
  chai.add(chaiscript::base_class<Unique_Ptr_Test_Base_Class, Unique_Ptr_Test_Derived_Class>());
  CHECK(chai.eval<int>("uptr.getI()") == 5);
  CHECK(chai.eval<int>("var uptr2 = make_Unique_Ptr_Test_Derived_Class(); uptr2.getI()") == 5);
}


class A
{
  public:
    A() = default;
    A(const A&) = default;
    A(A &&) = default;
    A &operator=(const A&) = default;
    A &operator=(A&&) = default;
    virtual ~A() = default;
};

class B : public A
{
  public:
    B() = default;
};

TEST_CASE("Test typed chaiscript functions to perform conversions")
{
  chaiscript::ChaiScript_Basic chai(create_chaiscript_stdlib(),create_chaiscript_parser());

  //-------------------------------------------------------------------------

  chai.add(chaiscript::user_type<A>(), "A");

  chai.add(chaiscript::user_type<B>(), "B");
  chai.add(chaiscript::base_class<A, B>());

  chai.add(chaiscript::fun([](const B &)
        {
        }), "CppFunctWithBArg");

  chai.add(chaiscript::fun([]() -> std::shared_ptr<A> 
        {
        return (std::shared_ptr<A>(new B()));
        }), "Create");

  chai.eval(R"(
            var inst = Create() // A*

            // it prints "A"
            inst.type_name().print() 

            // Ok it is casted using conversion
            CppFunctWithBArg(inst)

            // Define a function with B as argument
            def ChaiFuncWithBArg(B inst)
            {
                    print("ok")
            }

            // don't work
            ChaiFuncWithBArg(inst)
            )");
}

struct Reference_MyClass
{
  Reference_MyClass(double& t_x) : x(t_x) {}
  double& x;
};

TEST_CASE("Test reference member being registered")
{
  chaiscript::ChaiScript_Basic chai(create_chaiscript_stdlib(),create_chaiscript_parser());
  // Note, C++ will not allow us to do this:
  // chai.add(chaiscript::fun(&Reference_MyClass::x) , "x");
  chai.add(chaiscript::fun([](Reference_MyClass &r) -> decltype(auto) { return (r.x); }), "x");
  chai.add(chaiscript::fun([](const Reference_MyClass &r) -> decltype(auto) { return (r.x); }), "x");
  double d;
  chai.add(chaiscript::var(Reference_MyClass(d)), "ref");
  chai.eval("ref.x = 2.3");
  CHECK(d == Approx(2.3));
}

// starting with C++20 u8"" strings cannot be compared with std::string
// and the support for std::u8strings is still terrible.
TEST_CASE("Test unicode matches C++")
{
  chaiscript::ChaiScript_Basic chai(create_chaiscript_stdlib(), create_chaiscript_parser());
  CHECK("\U000000AC" == chai.eval<std::string>(R"("\U000000AC")"));
  CHECK("\xF0\x9F\x8D\x8C" == chai.eval<std::string>(R"("\xF0\x9F\x8D\x8C")"));
  CHECK("\U0001F34C" == chai.eval<std::string>(R"("\U0001F34C")"));
  CHECK("\u2022" == chai.eval<std::string>(R"("\u2022")"));
}


const int add_3(const int &i)
{
  return i + 3;
}

TEST_CASE("Test returning by const non-reference")
{
  chaiscript::ChaiScript_Basic chai(create_chaiscript_stdlib(),create_chaiscript_parser());
  // Note, C++ will not allow us to do this:
  // chai.add(chaiscript::fun(&Reference_MyClass::x) , "x");
  chai.add(chaiscript::fun(&add_3), "add_3");
  auto v = chai.eval<int>("add_3(12)");
  CHECK(v == 15);
}


struct MyException : std::runtime_error
{
  using std::runtime_error::runtime_error;
  int value = 5;
};

void throws_a_thing()
{
  throw MyException("Hello World");
}

TEST_CASE("Test throwing and catching custom exception")
{
  chaiscript::ChaiScript_Basic chai(create_chaiscript_stdlib(),create_chaiscript_parser());
  chai.add(chaiscript::user_type<MyException>(), "MyException");
  chai.add(chaiscript::base_class<std::runtime_error, MyException>()); // be sure to register base class relationship
  chai.add(chaiscript::fun(&throws_a_thing), "throws_a_thing");
  chai.add(chaiscript::fun(&MyException::value), "value");

  const auto s = chai.eval<std::string>("fun(){ try { throws_a_thing(); } catch (MyException ex) { return ex.what(); } }()");
  CHECK(s == "Hello World");

  // this has an explicit clone to prevent returning a pointer to the `value` from inside of MyException
  const auto i = chai.eval<int>("fun(){ try { throws_a_thing(); } catch (MyException ex) { var v = clone(ex.value); print(v); return v; } }()");
  CHECK(i == 5);
}


TEST_CASE("Test ability to get 'use' function from default construction")
{
  chaiscript::ChaiScript chai;
  const auto use_function = chai.eval<std::function<chaiscript::Boxed_Value (const std::string &)>>("use");
}

TEST_CASE("Throw an exception when trying to add same conversion twice")
{
  struct my_int {
      int value;
      my_int(int val): value(val) {}
  };

  chaiscript::ChaiScript chai;
  chai.add(chaiscript::type_conversion<int, my_int>([](int x) {
      std::cout << "My_int type conversion 1\n";
      return my_int(x);
  }));
  CHECK_THROWS_AS(chai.add(chaiscript::type_conversion<int, my_int>([](int x) {
      std::cout << "My_int type conversion 2\n";
      return my_int(x);
  })), chaiscript::exception::conversion_error);
}

TEST_CASE("Test if non copyable/movable types can be registered")
{
    struct Noncopyable {
        Noncopyable() {str = "test";}
        Noncopyable(const Noncopyable&) = delete;
        Noncopyable& operator=(const Noncopyable&) = delete;

        std::string str;
    };

    struct Nonmovable {
        Nonmovable() {str = "test";}
        Nonmovable(Nonmovable&&) = delete;
        Nonmovable& operator=(Nonmovable&&) = delete;

        std::string str;
    };

    struct Nothing {
        Nothing() {str = "test";}

        Nothing(Nothing&&) = delete;
        Nothing& operator=(Nothing&&) = delete;

        Nothing(const Nothing&) = delete;
        Nothing& operator=(const Nothing&) = delete;

        std::string str;
    };

    chaiscript::ChaiScript chai;
    chai.add(chaiscript::user_type<Noncopyable>(), "Noncopyable");
    chai.add(chaiscript::constructor<Noncopyable()>(), "Noncopyable");

    chai.add(chaiscript::user_type<Nonmovable>(), "Nonmovable");
    chai.add(chaiscript::constructor<Nonmovable()>(), "Nonmovable");

    chai.add(chaiscript::user_type<Nothing>(), "Nothing");
    chai.add(chaiscript::constructor<Nothing()>(), "Nothing");
}
