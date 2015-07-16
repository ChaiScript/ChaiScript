// Tests to make sure that the order in which function dispatches occur is correct

#include <chaiscript/chaiscript_defines.hpp>
#include <chaiscript/dispatchkit/type_info.hpp>
#include <iostream>
#include <cstdlib>

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


TEST_CASE("Type_Info objects generate expected results")
{
  const auto test_type = [](const chaiscript::Type_Info &ti, bool t_is_const, bool t_is_pointer, bool t_is_reference, bool t_is_void,
    bool t_is_undef, bool t_is_arithmetic)
  {
    CHECK(ti.is_const() == t_is_const);
    CHECK(ti.is_pointer() == t_is_pointer);
    CHECK(ti.is_reference() == t_is_reference);
    CHECK(ti.is_void() == t_is_void);
    CHECK(ti.is_undef() == t_is_undef);
    CHECK(ti.is_arithmetic() == t_is_arithmetic);
  };

  SECTION("void") {  test_type(chaiscript::user_type<void>(), false, false, false, true, false, false); }
  SECTION("const int") { test_type(chaiscript::user_type<const int>(), true, false, false, false, false, true); }
  SECTION("const int &") { test_type(chaiscript::user_type<const int &>(), true, false, true, false, false, true); }
  SECTION("int") { test_type(chaiscript::user_type<int>(), false, false, false, false, false, true); }
  SECTION("int *") { test_type(chaiscript::user_type<int *>(), false, true, false, false, false, false); }
  SECTION("const int *") { test_type(chaiscript::user_type<const int *>(), true, true, false, false, false, false); }
  SECTION("const bool &") { test_type(chaiscript::user_type<const bool &>(), true, false, true, false, false, false); }
  SECTION("default") { test_type(chaiscript::Type_Info(), false, false, false, false, true, false); }

  std::cout << "Size of Type_Info " << sizeof(chaiscript::Type_Info) << '\n';
}



