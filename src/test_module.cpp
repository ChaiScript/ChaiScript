
#include <chaiscript/chaiscript.hpp>
#include <chaiscript/dispatchkit/bootstrap.hpp>
#include <string>



class TestBaseType
{
  public:
#ifdef CHAISCRIPT_MSVC_12
#pragma warning(push)
#pragma warning(disable : 4351)
#endif
    // MSVC 12 warns that we are using new (correct) behavior 
    TestBaseType() : val(10), const_val(15), mdarray{} { }
    TestBaseType(int) : val(10), const_val(15), mdarray{} { }
    TestBaseType(int *) : val(10), const_val(15), mdarray{} { }
#ifdef CHAISCRIPT_MSVC_12
#pragma warning(pop)
#endif    
    
    TestBaseType(const TestBaseType &) = default;
    virtual ~TestBaseType() {}
    virtual int func() { return 0; }

    int base_only_func() { return -9; }

    const TestBaseType &constMe() const { return *this; }

    int val;
    const int const_val;

    int mdarray[2][3][5];
    std::function<int (int)> func_member;

    void set_string_val(std::string &t_str)
    {
      t_str = "42";
    }

  private:
    TestBaseType &operator=(const TestBaseType &) = delete;
};

class Type2
{
  public:
    Type2(TestBaseType t_bt)
      : m_bt(std::move(t_bt)),
        m_str("Hello World")
    {
    }

    int get_val() const
    {
      return m_bt.val;
    }


    const char *get_str() const
    {
      return m_str.c_str();
    }

  private:
    TestBaseType m_bt;
    std::string m_str;
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
    TestDerivedType(const TestDerivedType &) = default;
    TestDerivedType() = default;
    virtual int func() CHAISCRIPT_OVERRIDE { return 1; }
    int derived_only_func() { return 19; }

  private:
    TestDerivedType &operator=(const TestDerivedType &) = delete;
};

class TestMoreDerivedType : public TestDerivedType
{
  public:
    TestMoreDerivedType(const TestMoreDerivedType &) = default;
    TestMoreDerivedType() = default;
    virtual ~TestMoreDerivedType() {}
};

std::shared_ptr<TestBaseType> derived_type_factory()
{
  return std::make_shared<TestDerivedType>();
}

std::shared_ptr<TestBaseType> more_derived_type_factory()
{
  return std::make_shared<TestMoreDerivedType>();
}

std::shared_ptr<TestBaseType> null_factory()
{
  return std::shared_ptr<TestBaseType>();
}

std::string hello_world()
{
  return "Hello World";
}

static int global_i = 1;

int *get_new_int()
{
  return &global_i;
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
  m->add(chaiscript::user_type<Type2>(), "Type2");

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
  m->add(chaiscript::fun(&null_factory), "null_factory");

  m->add(chaiscript::fun(&TestDerivedType::func), "func");

  m->add(chaiscript::fun(&TestBaseType::func), "func");
  m->add(chaiscript::fun(&TestBaseType::val), "val");
  m->add(chaiscript::fun(&TestBaseType::const_val), "const_val");
  m->add(chaiscript::fun(&TestBaseType::base_only_func), "base_only_func");
  m->add(chaiscript::fun(&TestBaseType::set_string_val), "set_string_val");

#ifndef CHAISCRIPT_MSVC_12
  // we cannot support these in MSVC_12 because of a bug in the implementation of
  // std::reference_wrapper
  // Array types
  m->add(chaiscript::fun(&TestBaseType::mdarray), "mdarray");
  m->add(chaiscript::bootstrap::array<int[2][3][5]>("IntArray_2_3_5"));
  m->add(chaiscript::bootstrap::array<int[3][5]>("IntArray_3_5"));
  m->add(chaiscript::bootstrap::array<int[5]>("IntArray_5"));
  // end array types
#endif

  // member that is a function
  m->add(chaiscript::fun(&TestBaseType::func_member), "func_member");
  m->add(chaiscript::fun(&get_new_int), "get_new_int");


  m->add_global_const(chaiscript::const_var(TestValue1), "TestValue1");

  m->add(chaiscript::user_type<TestEnum>(), "TestEnum");

  m->add(chaiscript::fun(&to_int), "to_int");
  m->add(chaiscript::fun(&TestBaseType::constMe), "constMe");

  m->add(chaiscript::type_conversion<TestBaseType, Type2>([](const TestBaseType &t_bt) { return Type2(t_bt); }));

  m->add(chaiscript::fun(&Type2::get_val), "get_val");
  m->add(chaiscript::fun(&Type2::get_str), "get_str");
  m->add(chaiscript::type_conversion<const char *, std::string>());
  m->add(chaiscript::constructor<Type2 (const TestBaseType &)>(), "Type2");

  return m;
}


#ifdef __llvm__
#pragma clang diagnostic pop
#endif

#ifdef CHAISCRIPT_MSVC
#pragma warning(pop)
#endif
