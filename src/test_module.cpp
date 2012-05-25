
#include <chaiscript/chaiscript.hpp>
#include <string>

class TestBaseType
{
  public:
    TestBaseType() {}
    TestBaseType(int) {}
    TestBaseType(int *) {}
    virtual ~TestBaseType() {}
    virtual int func() { return 0; }

};

enum TestEnum
{
  TestValue1 = 1
};

int to_int(TestEnum t)
{
  return t;
}

class TestDerivedType : public TestBaseType
{
  public:
    virtual ~TestDerivedType() {}
    virtual int func() { return 1; }
};

std::string hello_world()
{
  return "Hello World";
}

int *get_new_int()
{
  return new int(1);
}

// MSVC doesn't like that we are using C++ return types from our C declared module
// but this is the best way to do it for cross platform compatibility
#ifdef BOOST_MSVC
#pragma warning(push)
#pragma warning(disable : 4190)
#endif


CHAISCRIPT_MODULE_EXPORT  chaiscript::ModulePtr create_chaiscript_module_test_module()
{
  chaiscript::ModulePtr m(new chaiscript::Module());

  m->add(chaiscript::fun(hello_world), "hello_world");

  m->add(chaiscript::user_type<TestBaseType>(), "TestBaseType");
  m->add(chaiscript::user_type<TestDerivedType>(), "TestDerivedType");

  m->add(chaiscript::constructor<TestBaseType ()>(), "TestBaseType");
//  m->add(chaiscript::constructor<TestBaseType (int)>(), "TestBaseType");
  m->add(chaiscript::constructor<TestBaseType (const TestBaseType &)>(), "TestBaseType");
  m->add(chaiscript::constructor<TestBaseType (int *)>(), "TestBaseType");

  m->add(chaiscript::constructor<TestDerivedType ()>(), "TestDerivedType");
  m->add(chaiscript::constructor<TestDerivedType (const TestDerivedType &)>(), "TestDerivedType");

  m->add(chaiscript::base_class<TestBaseType, TestDerivedType>());

  m->add(chaiscript::fun(&TestBaseType::func), "func");

  m->add(chaiscript::fun(&get_new_int), "get_new_int");

  m->add_global_const(chaiscript::const_var(TestValue1), "TestValue1");

  m->add(chaiscript::user_type<TestEnum>(), "TestEnum");

  m->add(chaiscript::fun(&to_int), "to_int");


  return m;
}


#ifdef BOOST_MSVC
#pragma warning(pop)
#endif
