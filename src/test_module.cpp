
#include <chaiscript/chaiscript.hpp>
#include <string>

class TestBaseType
{
  public:
    TestBaseType() : val(10), const_val(15) { }
    TestBaseType(int) : val(10), const_val(15) {}
    TestBaseType(int *) : val(10), const_val(15) {}
    virtual ~TestBaseType() {}
    virtual int func() { return 0; }

    int base_only_func() { return -9; }

    const TestBaseType &constMe() const { return *this; }

    int val;
    const int const_val;

  private:
    TestBaseType &operator=(const TestBaseType &);
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
    virtual int func() CHAISCRIPT_OVERRIDE { return 1; }
    int derived_only_func() { return 19; }

  private:
    TestDerivedType &operator=(const TestDerivedType &);
};

class TestMoreDerivedType : public TestDerivedType
{
  public:
    virtual ~TestMoreDerivedType() {}
};

std::shared_ptr<TestBaseType> derived_type_factory()
{
  return std::shared_ptr<TestBaseType>(new TestDerivedType());
}

std::shared_ptr<TestBaseType> more_derived_type_factory()
{
  return std::shared_ptr<TestBaseType>(new TestMoreDerivedType());
}

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
#ifdef CHAISCRIPT_MSVC
#pragma warning(push)
#pragma warning(disable : 4190)
#endif

#ifdef __llvm__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wreturn-type-c-linkage"
#endif

CHAISCRIPT_MODULE_EXPORT  chaiscript::ModulePtr create_chaiscript_module_test_module()
{
  chaiscript::ModulePtr m(new chaiscript::Module());

  m->add(chaiscript::fun(hello_world), "hello_world");

  m->add(chaiscript::user_type<TestBaseType>(), "TestBaseType");
  m->add(chaiscript::user_type<TestDerivedType>(), "TestDerivedType");
  m->add(chaiscript::user_type<TestMoreDerivedType>(), "TestMoreDerivedType");

  m->add(chaiscript::constructor<TestBaseType ()>(), "TestBaseType");
//  m->add(chaiscript::constructor<TestBaseType (int)>(), "TestBaseType");
  m->add(chaiscript::constructor<TestBaseType (const TestBaseType &)>(), "TestBaseType");
  m->add(chaiscript::constructor<TestBaseType (int *)>(), "TestBaseType");

  m->add(chaiscript::constructor<TestDerivedType ()>(), "TestDerivedType");
  m->add(chaiscript::constructor<TestDerivedType (const TestDerivedType &)>(), "TestDerivedType");

  m->add(chaiscript::constructor<TestMoreDerivedType ()>(), "TestMoreDerivedType");
  m->add(chaiscript::constructor<TestMoreDerivedType (const TestMoreDerivedType &)>(), "TestMoreDerivedType");

  /// \todo automatic chaining of base classes?
  m->add(chaiscript::base_class<TestBaseType, TestDerivedType>());
  m->add(chaiscript::base_class<TestBaseType, TestMoreDerivedType>());
  m->add(chaiscript::base_class<TestDerivedType, TestMoreDerivedType>());

  m->add(chaiscript::fun(&TestDerivedType::derived_only_func), "derived_only_func");

  m->add(chaiscript::fun(&derived_type_factory), "derived_type_factory");
  m->add(chaiscript::fun(&more_derived_type_factory), "more_derived_type_factory");

  m->add(chaiscript::fun(&TestDerivedType::func), "func");

  m->add(chaiscript::fun(&TestBaseType::func), "func");
  m->add(chaiscript::fun(&TestBaseType::val), "val");
  m->add(chaiscript::fun(&TestBaseType::const_val), "const_val");
  m->add(chaiscript::fun(&TestBaseType::base_only_func), "base_only_func");

  m->add(chaiscript::fun(&get_new_int), "get_new_int");


  m->add_global_const(chaiscript::const_var(TestValue1), "TestValue1");

  m->add(chaiscript::user_type<TestEnum>(), "TestEnum");

  m->add(chaiscript::fun(&to_int), "to_int");
  m->add(chaiscript::fun(&TestBaseType::constMe), "constMe");


  return m;
}


#ifdef __llvm__
#pragma clang diagnostic pop
#endif

#ifdef CHAISCRIPT_MSVC
#pragma warning(pop)
#endif
