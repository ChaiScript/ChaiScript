
#ifndef CHAISCRIPT_PARSER_LIB
#define CHAISCRIPT_PARSER_LIB

#include <memory>

namespace chaiscript {
  namespace parser {
    class ChaiScript_Parser_Base;
  }
} // namespace chaiscript

std::unique_ptr<chaiscript::parser::ChaiScript_Parser_Base> create_chaiscript_parser();

#endif
