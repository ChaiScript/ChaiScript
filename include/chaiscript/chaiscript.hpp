// This file is distributed under the BSD License.
// See "license.txt" for details.
// Copyright 2009, Jonathan Turner (jturner@minnow-lang.org)
// and Jason Turner (lefticus@gmail.com)
// http://www.chaiscript.com

#ifndef CHAISCRIPT_HPP_
#define CHAISCRIPT_HPP_

#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>

#include <stdexcept>
#include <iostream>
#include <map>
#include <fstream>
#include <boost/shared_ptr.hpp>

#include "dispatchkit/dispatchkit.hpp"
#include "dispatchkit/bootstrap.hpp"
#include "dispatchkit/bootstrap_stl.hpp"
#include "dispatchkit/function_call.hpp"


#ifdef  BOOST_HAS_DECLSPEC
#define CHAISCRIPT_MODULE_EXPORT extern "C" __declspec(dllexport)
#else
#define CHAISCRIPT_MODULE_EXPORT extern "C" 
#endif



namespace chaiscript
{
    typedef ModulePtr (*Create_Module_Func)();

    /**
     * Types of AST nodes available to the parser and eval
     */
    class Token_Type { public: enum Type { Error, Int, Float, Id, Char, Str, Eol, Fun_Call, Inplace_Fun_Call, Arg_List, Variable, Equation, Var_Decl,
        Expression, Comparison, Additive, Multiplicative, Negate, Not, Array_Call, Dot_Access, Quoted_String, Single_Quoted_String,
        Lambda, Block, Def, While, If, For, Inline_Array, Inline_Map, Return, File, Prefix, Break, Map_Pair, Value_Range,
        Inline_Range, Annotation }; };

    /**
     * Helper lookup to get the name of each node type
     */
    const char *token_type_to_string(int tokentype) {
        const char *token_types[] = { "Internal Parser Error", "Int", "Float", "Id", "Char", "Str", "Eol", "Fun_Call", "Inplace_Fun_Call", "Arg_List", "Variable", "Equation", "Var_Decl",
            "Expression", "Comparison", "Additive", "Multiplicative", "Negate", "Not", "Array_Call", "Dot_Access", "Quoted_String", "Single_Quoted_String",
            "Lambda", "Block", "Def", "While", "If", "For", "Inline_Array", "Inline_Map", "Return", "File", "Prefix", "Break", "Map_Pair", "Value_Range",
            "Inline_Range", "Annotation"};

        return token_types[tokentype];
    }

    /**
     * Convenience type for file positions
     */
    struct File_Position {
        int line;
        int column;

        File_Position(int file_line, int file_column)
            : line(file_line), column(file_column) { }

        File_Position() : line(0), column(0) { }
    };

    typedef boost::shared_ptr<struct Token> TokenPtr;

    /**
     * The struct that doubles as both a parser token and an AST node
     */
    struct Token {
        std::string text;
        int identifier;
        const char *filename;
        File_Position start, end;
        bool is_cached;
        Boxed_Value cached_value;

        std::vector<TokenPtr> children;
        TokenPtr annotation;

        Token(const std::string &token_text, int id, const char *fname) :
            text(token_text), identifier(id), filename(fname), is_cached(false) { }

        Token(const std::string &token_text, int id, const char *fname, int start_line, int start_col, int end_line, int end_col) :
            text(token_text), identifier(id), filename(fname), is_cached(false) {

            start.line = start_line;
            start.column = start_col;
            end.line = end_line;
            end.column = end_col;
        }
    };

    /**
     * Errors generated during parsing or evaluation
     */
    struct Eval_Error : public std::runtime_error {
        std::string reason;
        File_Position start_position;
        File_Position end_position;
        const char *filename;

        Eval_Error(const std::string &why, const File_Position &where, const char *fname) :
            std::runtime_error("Error: \"" + why + "\" " +
                (std::string(fname) != "__EVAL__" ? ("in '" + std::string(fname) + "' ") : "during evaluation ") +
                + "at (" + boost::lexical_cast<std::string>(where.line) + ", " +
                boost::lexical_cast<std::string>(where.column) + ")"),
            reason(why), start_position(where), end_position(where), filename(fname)
        { }

        Eval_Error(const std::string &why, const TokenPtr &where)
            : std::runtime_error("Error: \"" + why + "\" " +
                (std::string(where->filename) != "__EVAL__" ? ("in '" + std::string(where->filename) + "' ") : "during evaluation ") +
                "at (" + boost::lexical_cast<std::string>(where->start.line) + ", " +
                boost::lexical_cast<std::string>(where->start.column) + ")"),
              reason(why), start_position(where->start), end_position(where->end), filename(where->filename) {
        }

        virtual ~Eval_Error() throw() {}
    };

    /**
     * Special type for returned values
     */
    struct Return_Value {
        Boxed_Value retval;
        TokenPtr location;

        Return_Value(const Boxed_Value &return_value, const TokenPtr where) : retval(return_value), location(where) { }
    };

    /**
     * Special type indicating a call to 'break'
     */
    struct Break_Loop {
        TokenPtr location;

        Break_Loop(const TokenPtr where) : location(where) { }
    };
}

#include "language/chaiscript_eval.hpp"
#include "language/chaiscript_engine.hpp"

#endif /* CHAISCRIPT_HPP_ */
