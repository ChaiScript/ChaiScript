// This file is distributed under the BSD License.
// See LICENSE.TXT for details.

#ifndef WESLEY_HPP_
#define WESLEY_HPP_

#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>

#include <iostream>
#include <map>
#include <fstream>

class TokenType { public: enum Type { File, Whitespace, Identifier, Integer, Operator, Parens_Open, Parens_Close,
    Square_Open, Square_Close, Curly_Open, Curly_Close, Comma, Quoted_String, Single_Quoted_String, Carriage_Return, Semicolon,
    Function_Def, Scoped_Block, Statement, Equation, Return, Expression, Term, Factor, Negate, Comment,
    Value, Fun_Call, Method_Call, Comparison, If_Block, While_Block, Boolean, Real_Number, Array_Call, Variable_Decl, Array_Init,
    For_Block, Prefix, Break }; };

const char *tokentype_to_string(int tokentype) {
    const char *token_types[] = {"File", "Whitespace", "Identifier", "Integer", "Operator", "Parens_Open", "Parens_Close",
        "Square_Open", "Square_Close", "Curly_Open", "Curly_Close", "Comma", "Quoted_String", "Single_Quoted_String", "Carriage_Return", "Semicolon",
        "Function_Def", "Scoped_Block", "Statement", "Equation", "Return", "Expression", "Term", "Factor", "Negate", "Comment",
        "Value", "Fun_Call", "Method_Call", "Comparison", "If_Block", "While_Block", "Boolean", "Real Number", "Array_Call", "Variable_Decl", "Array_Init",
        "For_Block", "Prefix", "Break" };

    return token_types[tokentype];
}

#include "boxedcpp.hpp"
#include "bootstrap.hpp"
#include "bootstrap_stl.hpp"

#include "langkit_lexer.hpp"
#include "langkit_parser.hpp"

#include "wesley_eval.hpp"
#include "wesley_engine.hpp"

#endif /* WESLEY_HPP_ */
