// This file is distributed under the BSD License.
// See "license.txt" for details.
// Copyright 2009-2010, Jonathan Turner (jturner@minnow-lang.org)
// and Jason Turner (lefticus@gmail.com)
// http://www.chaiscript.com

#ifndef _CHAISCRIPT_COMMON_HPP
#define	_CHAISCRIPT_COMMON_HPP

#include <chaiscript/dispatchkit/dispatchkit.hpp>

namespace chaiscript
{
  typedef ModulePtr (*Create_Module_Func)();

  /**
   * Types of AST nodes available to the parser and eval
   */
  class Token_Type { public: enum Type { Error, Int, Float, Id, Char, Str, Eol, Fun_Call, Inplace_Fun_Call, Arg_List, Variable, Equation, Var_Decl,
                                         Comparison, Additive, Multiplicative, Array_Call, Dot_Access, Quoted_String, Single_Quoted_String,
                                         Lambda, Block, Def, While, If, For, Inline_Array, Inline_Map, Return, File, Prefix, Break, Map_Pair, Value_Range,
                                         Inline_Range, Annotation, Try, Catch, Finally, Method, Attr_Decl, Shift, Equality, Bitwise_And, Bitwise_Xor, Bitwise_Or, 
                                         Logical_And, Logical_Or}; };

  namespace
  {
    /**
     * Helper lookup to get the name of each node type
     */
    const char *token_type_to_string(int tokentype) {
      const char *token_types[] = { "Internal Parser Error", "Int", "Float", "Id", "Char", "Str", "Eol", "Fun_Call", "Inplace_Fun_Call", "Arg_List", "Variable", "Equation", "Var_Decl",
                                    "Comparison", "Additive", "Multiplicative", "Array_Call", "Dot_Access", "Quoted_String", "Single_Quoted_String",
                                    "Lambda", "Block", "Def", "While", "If", "For", "Inline_Array", "Inline_Map", "Return", "File", "Prefix", "Break", "Map_Pair", "Value_Range",
                                    "Inline_Range", "Annotation", "Try", "Catch", "Finally", "Method", "Attr_Decl", "Shift", "Equality", "Bitwise_And", "Bitwise_Xor", "Bitwise_Or", 
                                    "Logical_And", "Logical_Or"};

      return token_types[tokentype];
    }
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

    Token(const std::string &token_text, int id, const char *fname, int start_line, int start_col, int end_line, int end_col) :
      text(token_text), identifier(id), filename(fname), is_cached(false) {

      start.line = start_line;
      start.column = start_col;
      end.line = end_line;
      end.column = end_col;
    }
    Token(const std::string &token_text, int id, const char *fname) :
      text(token_text), identifier(id), filename(fname), is_cached(false) { }


    void cache_const(const Boxed_Value &value) {
      this->cached_value = value;
      this->is_cached = true;
    }

    virtual Boxed_Value eval(Dispatch_Engine &) {
      Boxed_Value bv;
      throw std::runtime_error("Undispatched token (internal error)");
      return bv;
    }
  };

  struct Error_Token : public Token {
  public:
    Error_Token(const std::string &token_text, int id, const char *fname, int start_line, int start_col, int end_line, int end_col) :
      Token(token_text, id, fname, start_line, start_col, end_line, end_col) { }
  };
  struct Int_Token : public Token {
  public:
    Int_Token(const std::string &token_text, int id, const char *fname, int start_line, int start_col, int end_line, int end_col) :
      Token(token_text, id, fname, start_line, start_col, end_line, end_col) { }
    Boxed_Value eval(Dispatch_Engine &ss);
  };
  struct Float_Token : public Token {
  public:
    Float_Token(const std::string &token_text, int id, const char *fname, int start_line, int start_col, int end_line, int end_col) :
      Token(token_text, id, fname, start_line, start_col, end_line, end_col) { }
    Boxed_Value eval(Dispatch_Engine &ss);
  };
  struct Id_Token : public Token {
  public:
    Id_Token(const std::string &token_text, int id, const char *fname, int start_line, int start_col, int end_line, int end_col) :
      Token(token_text, id, fname, start_line, start_col, end_line, end_col) { }
    Boxed_Value eval(Dispatch_Engine &ss);
  };
  struct Char_Token : public Token {
  public:
    Char_Token(const std::string &token_text, int id, const char *fname, int start_line, int start_col, int end_line, int end_col) :
      Token(token_text, id, fname, start_line, start_col, end_line, end_col) { }
  };
  struct Str_Token : public Token {
  public:
    Str_Token(const std::string &token_text, int id, const char *fname, int start_line, int start_col, int end_line, int end_col) :
      Token(token_text, id, fname, start_line, start_col, end_line, end_col) { }
  };
  struct Eol_Token : public Token {
  public:
    Eol_Token(const std::string &token_text, int id, const char *fname, int start_line, int start_col, int end_line, int end_col) :
      Token(token_text, id, fname, start_line, start_col, end_line, end_col) { }
  };
  struct Fun_Call_Token : public Token {
  public:
    Fun_Call_Token(const std::string &token_text, int id, const char *fname, int start_line, int start_col, int end_line, int end_col) :
      Token(token_text, id, fname, start_line, start_col, end_line, end_col) { }
    Boxed_Value eval(Dispatch_Engine &ss);
  };
  struct Inplace_Fun_Call_Token : public Token {
  public:
    Inplace_Fun_Call_Token(const std::string &token_text, int id, const char *fname, int start_line, int start_col, int end_line, int end_col) :
      Token(token_text, id, fname, start_line, start_col, end_line, end_col) { }
    Boxed_Value eval(Dispatch_Engine &ss);
  };
  struct Arg_List_Token : public Token {
  public:
    Arg_List_Token(const std::string &token_text, int id, const char *fname, int start_line, int start_col, int end_line, int end_col) :
      Token(token_text, id, fname, start_line, start_col, end_line, end_col) { }
  };
  struct Variable_Token : public Token {
  public:
    Variable_Token(const std::string &token_text, int id, const char *fname, int start_line, int start_col, int end_line, int end_col) :
      Token(token_text, id, fname, start_line, start_col, end_line, end_col) { }
  };
  struct Equation_Token : public Token {
  public:
    Equation_Token(const std::string &token_text, int id, const char *fname, int start_line, int start_col, int end_line, int end_col) :
      Token(token_text, id, fname, start_line, start_col, end_line, end_col) { }
    Boxed_Value eval(Dispatch_Engine &ss);
  };
  struct Var_Decl_Token : public Token {
  public:
    Var_Decl_Token(const std::string &token_text, int id, const char *fname, int start_line, int start_col, int end_line, int end_col) :
      Token(token_text, id, fname, start_line, start_col, end_line, end_col) { }
    Boxed_Value eval(Dispatch_Engine &ss);
  };
  struct Comparison_Token : public Token {
  public:
    Comparison_Token(const std::string &token_text, int id, const char *fname, int start_line, int start_col, int end_line, int end_col) :
      Token(token_text, id, fname, start_line, start_col, end_line, end_col) { }
    Boxed_Value eval(Dispatch_Engine &ss);
  };
  struct Additive_Token : public Token {
  public:
    Additive_Token(const std::string &token_text, int id, const char *fname, int start_line, int start_col, int end_line, int end_col) :
      Token(token_text, id, fname, start_line, start_col, end_line, end_col) { }
    Boxed_Value eval(Dispatch_Engine &ss);
  };
  struct Multiplicative_Token : public Token {
  public:
   Multiplicative_Token(const std::string &token_text, int id, const char *fname, int start_line, int start_col, int end_line, int end_col) :
      Token(token_text, id, fname, start_line, start_col, end_line, end_col) { }
    Boxed_Value eval(Dispatch_Engine &ss);
  };
  struct Array_Call_Token : public Token {
  public:
    Array_Call_Token(const std::string &token_text, int id, const char *fname, int start_line, int start_col, int end_line, int end_col) :
      Token(token_text, id, fname, start_line, start_col, end_line, end_col) { }
    Boxed_Value eval(Dispatch_Engine &ss);
  };
  struct Dot_Access_Token : public Token {
  public:
    Dot_Access_Token(const std::string &token_text, int id, const char *fname, int start_line, int start_col, int end_line, int end_col) :
      Token(token_text, id, fname, start_line, start_col, end_line, end_col) { }
    Boxed_Value eval(Dispatch_Engine &ss);
  };
  struct Quoted_String_Token : public Token {
  public:
    Quoted_String_Token(const std::string &token_text, int id, const char *fname, int start_line, int start_col, int end_line, int end_col) :
      Token(token_text, id, fname, start_line, start_col, end_line, end_col) { }
    Boxed_Value eval(Dispatch_Engine &ss);
  };
  struct Single_Quoted_String_Token : public Token {
  public:
    Single_Quoted_String_Token(const std::string &token_text, int id, const char *fname, int start_line, int start_col, int end_line, int end_col) :
      Token(token_text, id, fname, start_line, start_col, end_line, end_col) { }
    Boxed_Value eval(Dispatch_Engine &ss);
  };
  struct Lambda_Token : public Token {
  public:
    Lambda_Token(const std::string &token_text, int id, const char *fname, int start_line, int start_col, int end_line, int end_col) :
      Token(token_text, id, fname, start_line, start_col, end_line, end_col) { }
    Boxed_Value eval(Dispatch_Engine &ss);
  };
  struct Block_Token : public Token {
  public:
    Block_Token(const std::string &token_text, int id, const char *fname, int start_line, int start_col, int end_line, int end_col) :
      Token(token_text, id, fname, start_line, start_col, end_line, end_col) { }
    Boxed_Value eval(Dispatch_Engine &ss);
  };
  struct Def_Token : public Token {
  public:
    Def_Token(const std::string &token_text, int id, const char *fname, int start_line, int start_col, int end_line, int end_col) :
      Token(token_text, id, fname, start_line, start_col, end_line, end_col) { }
    Boxed_Value eval(Dispatch_Engine &ss);
  };
  struct While_Token : public Token {
  public:
    While_Token(const std::string &token_text, int id, const char *fname, int start_line, int start_col, int end_line, int end_col) :
      Token(token_text, id, fname, start_line, start_col, end_line, end_col) { }
    Boxed_Value eval(Dispatch_Engine &ss);
  };
  struct If_Token : public Token {
  public:
    If_Token(const std::string &token_text, int id, const char *fname, int start_line, int start_col, int end_line, int end_col) :
      Token(token_text, id, fname, start_line, start_col, end_line, end_col) { }
    Boxed_Value eval(Dispatch_Engine &ss);
  };
  struct For_Token : public Token {
  public:
    For_Token(const std::string &token_text, int id, const char *fname, int start_line, int start_col, int end_line, int end_col) :
      Token(token_text, id, fname, start_line, start_col, end_line, end_col) { }
    Boxed_Value eval(Dispatch_Engine &ss);
  };
  struct Inline_Array_Token : public Token {
  public:
    Inline_Array_Token(const std::string &token_text, int id, const char *fname, int start_line, int start_col, int end_line, int end_col) :
      Token(token_text, id, fname, start_line, start_col, end_line, end_col) { }
    Boxed_Value eval(Dispatch_Engine &ss);
  };
  struct Inline_Map_Token : public Token {
  public:
    Inline_Map_Token(const std::string &token_text, int id, const char *fname, int start_line, int start_col, int end_line, int end_col) :
      Token(token_text, id, fname, start_line, start_col, end_line, end_col) { }
    Boxed_Value eval(Dispatch_Engine &ss);
  };
  struct Return_Token : public Token {
  public:
    Return_Token(const std::string &token_text, int id, const char *fname, int start_line, int start_col, int end_line, int end_col) :
      Token(token_text, id, fname, start_line, start_col, end_line, end_col) { }
    Boxed_Value eval(Dispatch_Engine &ss);
  };
  struct File_Token : public Token {
  public:
    File_Token(const std::string &token_text, int id, const char *fname, int start_line, int start_col, int end_line, int end_col) :
      Token(token_text, id, fname, start_line, start_col, end_line, end_col) { }
    Boxed_Value eval(Dispatch_Engine &ss);
  };
  struct Prefix_Token : public Token {
  public:
    Prefix_Token(const std::string &token_text, int id, const char *fname, int start_line, int start_col, int end_line, int end_col) :
      Token(token_text, id, fname, start_line, start_col, end_line, end_col) { }
    Boxed_Value eval(Dispatch_Engine &ss);
  };
  struct Break_Token : public Token {
  public:
    Break_Token(const std::string &token_text, int id, const char *fname, int start_line, int start_col, int end_line, int end_col) :
      Token(token_text, id, fname, start_line, start_col, end_line, end_col) { }
    Boxed_Value eval(Dispatch_Engine &ss);
  };
  struct Map_Pair_Token : public Token {
  public:
    Map_Pair_Token(const std::string &token_text, int id, const char *fname, int start_line, int start_col, int end_line, int end_col) :
      Token(token_text, id, fname, start_line, start_col, end_line, end_col) { }
  };
  struct Value_Range_Token : public Token {
  public:
    Value_Range_Token(const std::string &token_text, int id, const char *fname, int start_line, int start_col, int end_line, int end_col) :
      Token(token_text, id, fname, start_line, start_col, end_line, end_col) { }
  };
  struct Inline_Range_Token : public Token {
  public:
    Inline_Range_Token(const std::string &token_text, int id, const char *fname, int start_line, int start_col, int end_line, int end_col) :
      Token(token_text, id, fname, start_line, start_col, end_line, end_col) { }
    Boxed_Value eval(Dispatch_Engine &ss);
  };
  struct Annotation_Token : public Token {
  public:
    Annotation_Token(const std::string &token_text, int id, const char *fname, int start_line, int start_col, int end_line, int end_col) :
      Token(token_text, id, fname, start_line, start_col, end_line, end_col) { }
  };
  struct Try_Token : public Token {
  public:
    Try_Token(const std::string &token_text, int id, const char *fname, int start_line, int start_col, int end_line, int end_col) :
      Token(token_text, id, fname, start_line, start_col, end_line, end_col) { }
    Boxed_Value eval(Dispatch_Engine &ss);
  };
  struct Catch_Token : public Token {
  public:
    Catch_Token(const std::string &token_text, int id, const char *fname, int start_line, int start_col, int end_line, int end_col) :
      Token(token_text, id, fname, start_line, start_col, end_line, end_col) { }
  };
  struct Finally_Token : public Token {
  public:
    Finally_Token(const std::string &token_text, int id, const char *fname, int start_line, int start_col, int end_line, int end_col) :
      Token(token_text, id, fname, start_line, start_col, end_line, end_col) { }
  };
  struct Method_Token : public Token {
  public:
    Method_Token(const std::string &token_text, int id, const char *fname, int start_line, int start_col, int end_line, int end_col) :
      Token(token_text, id, fname, start_line, start_col, end_line, end_col) { }
    Boxed_Value eval(Dispatch_Engine &ss);
  };
  struct Attr_Decl_Token : public Token {
  public:
    Attr_Decl_Token(const std::string &token_text, int id, const char *fname, int start_line, int start_col, int end_line, int end_col) :
      Token(token_text, id, fname, start_line, start_col, end_line, end_col) { }
    Boxed_Value eval(Dispatch_Engine &ss);
  };
  struct Shift_Token : public Token {
  public:
    Shift_Token(const std::string &token_text, int id, const char *fname, int start_line, int start_col, int end_line, int end_col) :
      Token(token_text, id, fname, start_line, start_col, end_line, end_col) { }
    Boxed_Value eval(Dispatch_Engine &ss);
  };
  struct Equality_Token : public Token {
  public:
    Equality_Token(const std::string &token_text, int id, const char *fname, int start_line, int start_col, int end_line, int end_col) :
      Token(token_text, id, fname, start_line, start_col, end_line, end_col) { }
    Boxed_Value eval(Dispatch_Engine &ss);
  };
  struct Bitwise_And_Token : public Token {
  public:
    Bitwise_And_Token(const std::string &token_text, int id, const char *fname, int start_line, int start_col, int end_line, int end_col) :
      Token(token_text, id, fname, start_line, start_col, end_line, end_col) { }
    Boxed_Value eval(Dispatch_Engine &ss);
  };
  struct Bitwise_Xor_Token : public Token {
  public:
    Bitwise_Xor_Token(const std::string &token_text, int id, const char *fname, int start_line, int start_col, int end_line, int end_col) :
      Token(token_text, id, fname, start_line, start_col, end_line, end_col) { }
    Boxed_Value eval(Dispatch_Engine &ss);
  };
  struct Bitwise_Or_Token : public Token {
  public:
    Bitwise_Or_Token(const std::string &token_text, int id, const char *fname, int start_line, int start_col, int end_line, int end_col) :
      Token(token_text, id, fname, start_line, start_col, end_line, end_col) { }
    Boxed_Value eval(Dispatch_Engine &ss);
  };
  struct Logical_And_Token : public Token {
  public:
    Logical_And_Token(const std::string &token_text, int id, const char *fname, int start_line, int start_col, int end_line, int end_col) :
      Token(token_text, id, fname, start_line, start_col, end_line, end_col) { }
    Boxed_Value eval(Dispatch_Engine &ss);
  };
  struct Logical_Or_Token : public Token {
  public:
    Logical_Or_Token(const std::string &token_text, int id, const char *fname, int start_line, int start_col, int end_line, int end_col) :
      Token(token_text, id, fname, start_line, start_col, end_line, end_col) { }
    Boxed_Value eval(Dispatch_Engine &ss);
  };

  /**
   * Errors generated during parsing or evaluation
   */
  struct Eval_Error : public std::runtime_error {
    std::string reason;
    File_Position start_position;
    File_Position end_position;
    const char *filename;
    std::vector<TokenPtr> call_stack;

    Eval_Error(const std::string &why, const File_Position &where, const char *fname) :
      std::runtime_error("Error: \"" + why + "\" " +
                         (std::string(fname) != "__EVAL__" ? ("in '" + std::string(fname) + "' ") : "during evaluation ") +
                         + "at (" + boost::lexical_cast<std::string>(where.line) + ", " +
                         boost::lexical_cast<std::string>(where.column) + ")"),
      reason(why), start_position(where), end_position(where), filename(fname)
    { }

    Eval_Error(const std::string &why)
      : std::runtime_error("Error: \"" + why + "\" "),
        reason(why) {}

    virtual ~Eval_Error() throw() {}
  };

  /**
   * Errors generated when loading a file
   */
  struct File_Not_Found_Error : public std::runtime_error {
    File_Not_Found_Error(const std::string &filename)
      : std::runtime_error("File Not Found: " + filename)
    { }

    virtual ~File_Not_Found_Error() throw() {}
  };


  /**
   * Special type for returned values
   */
  struct Return_Value {
    Boxed_Value retval;

    Return_Value(const Boxed_Value &return_value) : retval(return_value) { }
  };

  /**
   * Special type indicating a call to 'break'
   */
  struct Break_Loop {
    Break_Loop() { }
  };
}

#endif	/* _CHAISCRIPT_COMMON_HPP */

