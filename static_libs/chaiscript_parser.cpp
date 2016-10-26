#include "../include/chaiscript/language/chaiscript_parser.hpp"
#include "chaiscript_parser.hpp"

std::unique_ptr<chaiscript::parser::ChaiScript_Parser_Base> create_chaiscript_parser()
{
  return std::make_unique<chaiscript::parser::ChaiScript_Parser<chaiscript::eval::Noop_Tracer, chaiscript::optimizer::Optimizer_Default>>();
}

