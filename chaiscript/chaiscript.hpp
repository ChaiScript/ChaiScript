// This file is distributed under the BSD License.
// See LICENSE.TXT for details.

#ifndef CHAISCRIPT_HPP_
#define CHAISCRIPT_HPP_

#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>

#include <stdexcept>
#include <tr1/memory>
#include <iostream>
#include <map>
#include <fstream>

#include "dispatchkit.hpp"
#include "bootstrap.hpp"
#include "bootstrap_stl.hpp"

namespace chaiscript
{
    /*
    class TokenType { public: enum Type { File, Whitespace, Identifier, Integer, Operator, Parens_Open, Parens_Close,
        Square_Open, Square_Close, Curly_Open, Curly_Close, Comma, Quoted_String, Single_Quoted_String, Carriage_Return, Semicolon,
        Function_Def, Lambda_Def, Scoped_Block, Statement, Equation, Return, Expression, Term, Factor, Negate, Not, Comment,
        Value, Fun_Call, Method_Call, Comparison, If_Block, While_Block, Boolean, Real_Number, Array_Call, Variable_Decl, Array_Init, Map_Init,
        For_Block, Prefix, Break, Map_Pair}; };

    const char *tokentype_to_string(int tokentype) {
        const char *token_types[] = {"File", "Whitespace", "Identifier", "Integer", "Operator", "Parens_Open", "Parens_Close",
            "Square_Open", "Square_Close", "Curly_Open", "Curly_Close", "Comma", "Quoted_String", "Single_Quoted_String", "Carriage_Return", "Semicolon",
            "Function_Def", "Lambda_Def", "Scoped_Block", "Statement", "Equation", "Return", "Expression", "Term", "Factor", "Negate", "Not", "Comment",
            "Value", "Fun_Call", "Method_Call", "Comparison", "If_Block", "While_Block", "Boolean", "Real Number", "Array_Call", "Variable_Decl", "Array_Init", "Map_Init",
            "For_Block", "Prefix", "Break", "Map_Pair"};

        return token_types[tokentype];
    }
    */
    class Token_Type { public: enum Type { Error, Int, Float, Id, Char, Str, Eol, Fun_Call, Arg_List, Variable, Equation, Var_Decl,
        Expression, Comparison, Additive, Multiplicative, Negate, Not, Array_Call, Dot_Access, Quoted_String, Single_Quoted_String,
        Lambda, Block, Def, While, If, For, Inline_Array, Inline_Map, Return, File, Prefix, Break, Map_Pair }; };

    const char *token_type_to_string(int tokentype) {
        const char *token_types[] = { "Internal Parser Error", "Int", "Float", "Id", "Char", "Str", "Eol", "Fun_Call", "Arg_List", "Variable", "Equation", "Var_Decl",
            "Expression", "Comparison", "Additive", "Multiplicative", "Negate", "Not", "Array_Call", "Dot_Access", "Quoted_String", "Single_Quoted_String",
            "Lambda", "Block", "Def", "While", "If", "For", "Inline_Array", "Inline_Map", "Return", "File", "Prefix", "Break", "Map_Pair" };

        return token_types[tokentype];
    }

    struct File_Position {
        int line;
        int column;
       // std::string::iterator text_pos;

        File_Position(int file_line, int file_column)
            : line(file_line), column(file_column) { }

        File_Position() : line(0), column(0) { }
    };

    typedef std::tr1::shared_ptr<struct Token> TokenPtr;

    struct Token {
        std::string text;
        int identifier;
        const char *filename;
        File_Position start, end;

        std::vector<TokenPtr> children;

        Token(const std::string &token_text, int id, const char *fname) : text(token_text), identifier(id), filename(fname) { }

        Token(const std::string &token_text, int id, const char *fname, int start_line, int start_col, int end_line, int end_col) :
            text(token_text), identifier(id), filename(fname) {

            start.line = start_line;
            start.column = start_col;
            end.line = end_line;
            end.column = end_col;
        }
    };

    struct Parse_Error {
        std::string reason;
        File_Position position;
        const char *filename;

        Parse_Error(const std::string &why, const File_Position &where, const char *fname) :
            reason(why), position(where), filename(fname) { }

        Parse_Error(const std::string &why, const TokenPtr &where) : reason(why) {
            filename = where->filename;
            position = where->start;
        }

        virtual ~Parse_Error() throw() {}
    };

    struct ParserError {
        std::string reason;
        TokenPtr location;

        ParserError(const std::string &why, const TokenPtr where) : reason(why), location(where){ }
    };

    struct EvalError : public std::runtime_error {
        std::string reason;
        TokenPtr location;

        EvalError(const std::string &why, const TokenPtr where)
          : std::runtime_error("Eval error: \"" + why + "\" in '"
              + where->filename + "' line: " + boost::lexical_cast<std::string>(where->start.line+1)),
            reason(why), location(where) { }

        virtual ~EvalError() throw() {}
    };

    struct ReturnValue {
        dispatchkit::Boxed_Value retval;
        TokenPtr location;

        ReturnValue(const dispatchkit::Boxed_Value &return_value, const TokenPtr where) : retval(return_value), location(where) { }
    };

    struct BreakLoop {
        TokenPtr location;

        BreakLoop(const TokenPtr where) : location(where) { }
    };
}

#include "chaiscript_eval.hpp"
#include "chaiscript_engine.hpp"

#endif /* CHAISCRIPT_HPP_ */
