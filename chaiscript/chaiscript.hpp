// This file is distributed under the BSD License.
// See LICENSE.TXT for details.

#ifndef CHAISCRIPT_HPP_
#define CHAISCRIPT_HPP_

#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>

#include <iostream>
#include <map>
#include <fstream>

namespace chaiscript
{

    class TokenType { public: enum Type { File, Whitespace, Identifier, Integer, Operator, Parens_Open, Parens_Close,
        Square_Open, Square_Close, Curly_Open, Curly_Close, Comma, Quoted_String, Single_Quoted_String, Carriage_Return, Semicolon,
        Function_Def, Lambda_Def, Scoped_Block, Statement, Equation, Return, Expression, Term, Factor, Negate, Not, Comment,
        Value, Fun_Call, Method_Call, Comparison, If_Block, While_Block, Boolean, Real_Number, Array_Call, Variable_Decl, Array_Init, Map_Init,
        For_Block, Prefix, Break, Map_Pair }; };

    const char *tokentype_to_string(int tokentype) {
        const char *token_types[] = {"File", "Whitespace", "Identifier", "Integer", "Operator", "Parens_Open", "Parens_Close",
            "Square_Open", "Square_Close", "Curly_Open", "Curly_Close", "Comma", "Quoted_String", "Single_Quoted_String", "Carriage_Return", "Semicolon",
            "Function_Def", "Lambda_Def", "Scoped_Block", "Statement", "Equation", "Return", "Expression", "Term", "Factor", "Negate", "Not", "Comment",
            "Value", "Fun_Call", "Method_Call", "Comparison", "If_Block", "While_Block", "Boolean", "Real Number", "Array_Call", "Variable_Decl", "Array_Init", "Map_Init",
            "For_Block", "Prefix", "Break", "Map_Pair"};

        return token_types[tokentype];
    }
}

#include "dispatchkit.hpp"
#include "bootstrap.hpp"
#include "bootstrap_stl.hpp"

#include "langkit_lexer.hpp"
#include "langkit_parser.hpp"

#include "chaiscript_eval.hpp"
#include "chaiscript_engine.hpp"

#endif /* CHAISCRIPT_HPP_ */
