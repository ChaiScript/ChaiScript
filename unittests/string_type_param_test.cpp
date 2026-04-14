#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4062 4242 4566 4640 4702 6330 28251)
#endif

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunknown-pragmas"
#pragma GCC diagnostic ignored "-Wparentheses"
#pragma GCC diagnostic ignored "-Wignored-qualifiers"
#endif

#include <chaiscript/chaiscript.hpp>
#include <chaiscript/chaiscript_basic.hpp>

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

TEST_CASE("String type can be parameterized to wstring") {
  chaiscript::ChaiScript_WString chai;

  SECTION("String literals produce std::wstring") {
    auto result = chai.eval<std::wstring>("\"hello\"");
    CHECK(result == L"hello");
  }

  SECTION("String concatenation works with wstring") {
    auto result = chai.eval<std::wstring>("\"hello\" + \" world\"");
    CHECK(result == L"hello world");
  }

  SECTION("to_string works for numbers with wstring") {
    auto result = chai.eval<std::wstring>("to_string(42)");
    CHECK(result == L"42");
  }

  SECTION("String interpolation works with wstring") {
    auto result = chai.eval<std::wstring>("var x = 5; \"value: ${x}\"");
    CHECK(result == L"value: 5");
  }

  SECTION("Default ChaiScript still uses std::string") {
    chaiscript::ChaiScript default_chai;
    auto result = default_chai.eval<std::string>("\"hello\"");
    CHECK(result == "hello");
  }
}
