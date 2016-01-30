// All of these are necessary because of catch.hpp. It's OK, they'll be
// caught in other cpp files if chaiscript causes them

#include <chaiscript/utility/utility.hpp>
#include <chaiscript/dispatchkit/bootstrap_stl.hpp>

#ifdef CHAISCRIPT_MSVC
#pragma warning(push)
#pragma warning(disable : 4190 4640 28251 4702 6330)
#endif

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
#pragma GCC diagnostic ignored "-Wold-style-cast"
#endif

#ifdef __llvm__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wreturn-type-c-linkage"
#pragma clang diagnostic ignored "-Wold-style-cast"
#pragma clang diagnostic ignored "-Wexit-time-destructors"
#pragma clang diagnostic ignored "-Wfloat-equal"
#pragma clang diagnostic ignored "-Wunreachable-code"
#endif



#define CATCH_CONFIG_MAIN

#include "catch.hpp"

// lambda_tests
TEST_CASE("C++11 Lambdas Can Be Registered")
{

  // We cannot deduce the type of a lambda expression, you must either wrap it
  // in an std::function or provide the signature
  chaiscript::ChaiScript chai;

  chai.add(chaiscript::fun([]()->std::string { return "hello"; } ), "f1");

  // wrap
  chai.add(chaiscript::fun(std::function<std::string ()>([] { return "world"; } )), "f2");

  CHECK(chai.eval<std::string>("f1()") == "hello");
  CHECK(chai.eval<std::string>("f2()") == "world");
}


// dynamic_object tests
TEST_CASE("Dynamic_Object attributes can be shared with C++")
{
  chaiscript::ChaiScript chai;

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

  chaiscript::ChaiScript chai;

  chai.eval("def func() { print(\"Hello World\"); } ");

  std::function<void ()> f = chai.eval<std::function<void ()> >("func");
  f();

  CHECK(chai.eval<std::function<std::string (int)> >("to_string")(6) == "6");
  CHECK(chai.eval<std::function<std::string (const chaiscript::Boxed_Value &)> >("to_string")(chaiscript::var(6)) == "6");
}


TEST_CASE("ChaiScript can be created and destroyed on heap")
{
  chaiscript::ChaiScript *chai = new chaiscript::ChaiScript();
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
  chaiscript::ChaiScript chai;

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
  chaiscript::ChaiScript chai;

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
  chaiscript::ChaiScript chai;

  try {
    chai.eval("throw(1)", chaiscript::exception_specification<int>());
    REQUIRE(false);
  } catch (int e) {
    CHECK(e == 1);
  }
}

TEST_CASE("Throw int or double")
{
  chaiscript::ChaiScript chai;

  try {
    chai.eval("throw(1.0)", chaiscript::exception_specification<int, double>());
    REQUIRE(false);
  } catch (const double e) {
    CHECK(e == 1.0);
  }
}

TEST_CASE("Throw a runtime_error")
{
  chaiscript::ChaiScript chai;

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
  chaiscript::ChaiScript chai;

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
  chaiscript::ChaiScript chai;
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
  chaiscript::ChaiScript chai;
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

  chaiscript::ChaiScript chai;

  chai.add(chaiscript::fun(&functor_cast_test_call), "test_call");

  chai.eval("def func(i) { return i * 6; };");
  int d = chai.eval<int>("test_call(func, 3)");

  CHECK(d == 3 * 6);
}



int set_state_test_myfun()
{
  return 2;
}

TEST_CASE("Set and restore chai state")
{
  chaiscript::ChaiScript chai;

  // save the initial state of globals and locals
  chaiscript::ChaiScript::State firststate = chai.get_state();
  std::map<std::string, chaiscript::Boxed_Value> locals = chai.get_locals();

  // add some new globals and locals
  chai.add(chaiscript::var(1), "i");

  chai.add(chaiscript::fun(&set_state_test_myfun), "myfun");


  CHECK(chai.eval<int>("myfun()") == 2);

  CHECK(chai.eval<int>("i") == 1);

  chai.set_state(firststate);

  // set state should have reverted the state of the functions and dropped
  // the 'myfun'

  CHECK_THROWS_AS(chai.eval<int>("myfun()"), chaiscript::exception::eval_error &);

  // set state should not affect the local variables
  CHECK(chai.eval<int>("i") == 1);

  // After resetting the locals we expect the 'i' to be gone
  chai.set_locals(locals);

  CHECK_THROWS_AS(chai.eval<int>("i"), chaiscript::exception::eval_error &);
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
  chaiscript::ChaiScript chai;
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
  chaiscript::ChaiScript chai;
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
  chaiscript::ChaiScript chai;
  chai.add(chaiscript::fun(&simultaneous_chaiscript_do_something), "do_something");
  chai.add(chaiscript::var(1), "i");

  for (int i = 0; i < 10; ++i)
  {
    chaiscript::ChaiScript chai2;
    chai2.add(chaiscript::fun(&simultaneous_chaiscript_do_something_else), "do_something_else");

    CHECK(chai.eval<int>("do_something(" + std::to_string(i) + ")") == i + 2);
    CHECK(chai2.eval<int>("do_something_else(" + std::to_string(i) + ")") == i * 2);

    CHECK_THROWS_AS(chai2.eval("do_something(1)"), chaiscript::exception::eval_error &);
    CHECK_THROWS_AS(chai2.eval("i"), chaiscript::exception::eval_error &);
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


  chaiscript::ChaiScript chai;
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


  chaiscript::ChaiScript chai;
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

  chaiscript::ChaiScript chai;
  chai.add(m);

  chai.eval(" { auto i = create(); } ");

  CHECK(Object_Copy_Count_Test::copycount() == 0);
  CHECK(Object_Copy_Count_Test::constructcount() == 1);


#ifdef CHAISCRIPT_MSVC
  CHECK(Object_Copy_Count_Test::destructcount() == 3);
  CHECK(Object_Copy_Count_Test::movecount() == 2);
#else
  CHECK(Object_Copy_Count_Test::destructcount() == 2);
  CHECK(Object_Copy_Count_Test::movecount() == 1);
#endif
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

  chaiscript::ChaiScript chai;
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
  Object_Lifetime_Vector2(const Object_Lifetime_Vector2& cp) : x(cp.x), y(cp.y) {}

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
  chaiscript::ChaiScript _script;

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
  chaiscript::ChaiScript chai;
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
  chaiscript::ChaiScript chai;
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
  chaiscript::ChaiScript chai;
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
  chaiscript::ChaiScript chai;
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
  chaiscript::ChaiScript chai;

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
  chaiscript::ChaiScript chai;
  chai.add(chaiscript::map_conversion<std::map<std::string, int>>());
  chai.add(chaiscript::fun(&get_value_a), "get_value_a");

  const auto c = chai.eval<int>(R"(
    var m = ["a": 42];
    get_value_a(m);
  )");

  CHECK(c == 42);

}




