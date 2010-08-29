// This file is distributed under the BSD License.
// See "license.txt" for details.
// Copyright 2009-2010, Jonathan Turner (jonathan@emptycrate.com)
// and Jason Turner (jason@emptycrate.com)
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
  class AST_Node_Type {
  public:
    enum Type { Error, Int, Float, Id, Char, Str, Eol, Fun_Call, Inplace_Fun_Call, Arg_List, Variable, Equation, Var_Decl,
                Comparison, Additive, Multiplicative, Array_Call, Dot_Access, Quoted_String, Single_Quoted_String,
                Lambda, Block, Def, While, If, For, Inline_Array, Inline_Map, Return, File, Prefix, Break, Map_Pair, Value_Range,
                Inline_Range, Annotation, Try, Catch, Finally, Method, Attr_Decl, Shift, Equality, Bitwise_And, Bitwise_Xor, Bitwise_Or, 
                Logical_And, Logical_Or
    };
  };

  namespace
  {
    /**
     * Helper lookup to get the name of each node type
     */
    const char *ast_node_type_to_string(int ast_node_type) {
      const char *ast_node_types[] = { "Internal Parser Error", "Int", "Float", "Id", "Char", "Str", "Eol", "Fun_Call", "Inplace_Fun_Call", "Arg_List", "Variable", "Equation", "Var_Decl",
                                    "Comparison", "Additive", "Multiplicative", "Array_Call", "Dot_Access", "Quoted_String", "Single_Quoted_String",
                                    "Lambda", "Block", "Def", "While", "If", "For", "Inline_Array", "Inline_Map", "Return", "File", "Prefix", "Break", "Map_Pair", "Value_Range",
                                    "Inline_Range", "Annotation", "Try", "Catch", "Finally", "Method", "Attr_Decl", "Shift", "Equality", "Bitwise_And", "Bitwise_Xor", "Bitwise_Or", 
                                    "Logical_And", "Logical_Or"};

      return ast_node_types[ast_node_type];
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

  typedef boost::shared_ptr<struct AST_Node> AST_NodePtr;

  /**
   * The struct that doubles as both a parser ast_node and an AST node
   */
  struct AST_Node {
    std::string text;
    int identifier;
    const char *filename;
    File_Position start, end;
    /*
    bool is_cached;
    Boxed_Value cached_value;
    */
    std::vector<AST_NodePtr> children;
    AST_NodePtr annotation;

    AST_Node(const std::string &ast_node_text, int id, const char *fname, int start_line, int start_col, int end_line, int end_col) :
      text(ast_node_text), identifier(id), filename(fname)/*, is_cached(false)*/ {

      start.line = start_line;
      start.column = start_col;
      end.line = end_line;
      end.column = end_col;
    }
    AST_Node(const std::string &ast_node_text, int id, const char *fname) :
      text(ast_node_text), identifier(id), filename(fname)/*, is_cached(false)*/ { }

    /*
    void cache_const(const Boxed_Value &value) {
      this->cached_value = value;
      this->is_cached = true;
    }
    */

    virtual Boxed_Value eval(Dispatch_Engine &) {
      Boxed_Value bv;
      throw std::runtime_error("Undispatched ast_node (internal error)");
      return bv;
    }
  };

  struct Error_AST_Node : public AST_Node {
  public:
    Error_AST_Node(const std::string &ast_node_text = "", int id = AST_Node_Type::Error, const char *fname = NULL, int start_line = 0, int start_col = 0, int end_line = 0, int end_col = 0) :
      AST_Node(ast_node_text, id, fname, start_line, start_col, end_line, end_col) { }
  };
  struct Int_AST_Node : public AST_Node {
  public:
    Int_AST_Node(const std::string &ast_node_text = "", int id = AST_Node_Type::Int, const char *fname = NULL, int start_line = 0, int start_col = 0, int end_line = 0, int end_col = 0) :
      AST_Node(ast_node_text, id, fname, start_line, start_col, end_line, end_col) { }
    Boxed_Value eval(Dispatch_Engine &ss);
  };
  struct Float_AST_Node : public AST_Node {
  public:
    Float_AST_Node(const std::string &ast_node_text = "", int id = AST_Node_Type::Float, const char *fname = NULL, int start_line = 0, int start_col = 0, int end_line = 0, int end_col = 0) :
      AST_Node(ast_node_text, id, fname, start_line, start_col, end_line, end_col) { }
    Boxed_Value eval(Dispatch_Engine &ss);
  };
  struct Id_AST_Node : public AST_Node {
  public:
    Id_AST_Node(const std::string &ast_node_text = "", int id = AST_Node_Type::Id, const char *fname = NULL, int start_line = 0, int start_col = 0, int end_line = 0, int end_col = 0) :
      AST_Node(ast_node_text, id, fname, start_line, start_col, end_line, end_col) { }
    Boxed_Value eval(Dispatch_Engine &ss);
  };
  struct Char_AST_Node : public AST_Node {
  public:
    Char_AST_Node(const std::string &ast_node_text = "", int id = AST_Node_Type::Char, const char *fname = NULL, int start_line = 0, int start_col = 0, int end_line = 0, int end_col = 0) :
      AST_Node(ast_node_text, id, fname, start_line, start_col, end_line, end_col) { }
  };
  struct Str_AST_Node : public AST_Node {
  public:
    Str_AST_Node(const std::string &ast_node_text = "", int id = AST_Node_Type::Str, const char *fname = NULL, int start_line = 0, int start_col = 0, int end_line = 0, int end_col = 0) :
      AST_Node(ast_node_text, id, fname, start_line, start_col, end_line, end_col) { }
  };
  struct Eol_AST_Node : public AST_Node {
  public:
    Eol_AST_Node(const std::string &ast_node_text = "", int id = AST_Node_Type::Eol, const char *fname = NULL, int start_line = 0, int start_col = 0, int end_line = 0, int end_col = 0) :
      AST_Node(ast_node_text, id, fname, start_line, start_col, end_line, end_col) { }
  };
  struct Fun_Call_AST_Node : public AST_Node {
  public:
    Fun_Call_AST_Node(const std::string &ast_node_text = "", int id = AST_Node_Type::Fun_Call, const char *fname = NULL, int start_line = 0, int start_col = 0, int end_line = 0, int end_col = 0) :
      AST_Node(ast_node_text, id, fname, start_line, start_col, end_line, end_col) { }
    Boxed_Value eval(Dispatch_Engine &ss);
  };
  struct Inplace_Fun_Call_AST_Node : public AST_Node {
  public:
    Inplace_Fun_Call_AST_Node(const std::string &ast_node_text = "", int id = AST_Node_Type::Inplace_Fun_Call, const char *fname = NULL, int start_line = 0, int start_col = 0, int end_line = 0, int end_col = 0) :
      AST_Node(ast_node_text, id, fname, start_line, start_col, end_line, end_col) { }
    Boxed_Value eval(Dispatch_Engine &ss);
  };
  struct Arg_List_AST_Node : public AST_Node {
  public:
    Arg_List_AST_Node(const std::string &ast_node_text = "", int id = AST_Node_Type::Arg_List, const char *fname = NULL, int start_line = 0, int start_col = 0, int end_line = 0, int end_col = 0) :
      AST_Node(ast_node_text, id, fname, start_line, start_col, end_line, end_col) { }
  };
  struct Variable_AST_Node : public AST_Node {
  public:
    Variable_AST_Node(const std::string &ast_node_text = "", int id = AST_Node_Type::Variable, const char *fname = NULL, int start_line = 0, int start_col = 0, int end_line = 0, int end_col = 0) :
      AST_Node(ast_node_text, id, fname, start_line, start_col, end_line, end_col) { }
  };
  struct Equation_AST_Node : public AST_Node {
  public:
    Equation_AST_Node(const std::string &ast_node_text = "", int id = AST_Node_Type::Equation, const char *fname = NULL, int start_line = 0, int start_col = 0, int end_line = 0, int end_col = 0) :
      AST_Node(ast_node_text, id, fname, start_line, start_col, end_line, end_col) { }
    Boxed_Value eval(Dispatch_Engine &ss);
  };
  struct Var_Decl_AST_Node : public AST_Node {
  public:
    Var_Decl_AST_Node(const std::string &ast_node_text = "", int id = AST_Node_Type::Var_Decl, const char *fname = NULL, int start_line = 0, int start_col = 0, int end_line = 0, int end_col = 0) :
      AST_Node(ast_node_text, id, fname, start_line, start_col, end_line, end_col) { }
    Boxed_Value eval(Dispatch_Engine &ss);
  };
  struct Comparison_AST_Node : public AST_Node {
  public:
    Comparison_AST_Node(const std::string &ast_node_text = "", int id = AST_Node_Type::Comparison, const char *fname = NULL, int start_line = 0, int start_col = 0, int end_line = 0, int end_col = 0) :
      AST_Node(ast_node_text, id, fname, start_line, start_col, end_line, end_col) { }
    Boxed_Value eval(Dispatch_Engine &ss);
  };
  struct Additive_AST_Node : public AST_Node {
  public:
    Additive_AST_Node(const std::string &ast_node_text = "", int id = AST_Node_Type::Additive, const char *fname = NULL, int start_line = 0, int start_col = 0, int end_line = 0, int end_col = 0) :
      AST_Node(ast_node_text, id, fname, start_line, start_col, end_line, end_col) { }
    Boxed_Value eval(Dispatch_Engine &ss);
  };
  struct Multiplicative_AST_Node : public AST_Node {
  public:
    Multiplicative_AST_Node(const std::string &ast_node_text = "", int id = AST_Node_Type::Multiplicative, const char *fname = NULL, int start_line = 0, int start_col = 0, int end_line = 0, int end_col = 0) :
      AST_Node(ast_node_text, id, fname, start_line, start_col, end_line, end_col) { }
    Boxed_Value eval(Dispatch_Engine &ss);
  };
  struct Array_Call_AST_Node : public AST_Node {
  public:
    Array_Call_AST_Node(const std::string &ast_node_text = "", int id = AST_Node_Type::Array_Call, const char *fname = NULL, int start_line = 0, int start_col = 0, int end_line = 0, int end_col = 0) :
      AST_Node(ast_node_text, id, fname, start_line, start_col, end_line, end_col) { }
    Boxed_Value eval(Dispatch_Engine &ss);
  };
  struct Dot_Access_AST_Node : public AST_Node {
  public:
    Dot_Access_AST_Node(const std::string &ast_node_text = "", int id = AST_Node_Type::Dot_Access, const char *fname = NULL, int start_line = 0, int start_col = 0, int end_line = 0, int end_col = 0) :
      AST_Node(ast_node_text, id, fname, start_line, start_col, end_line, end_col) { }
    Boxed_Value eval(Dispatch_Engine &ss);
  };
  struct Quoted_String_AST_Node : public AST_Node {
  public:
    Quoted_String_AST_Node(const std::string &ast_node_text = "", int id = AST_Node_Type::Quoted_String, const char *fname = NULL, int start_line = 0, int start_col = 0, int end_line = 0, int end_col = 0) :
      AST_Node(ast_node_text, id, fname, start_line, start_col, end_line, end_col) { }
    Boxed_Value eval(Dispatch_Engine &ss);
  };
  struct Single_Quoted_String_AST_Node : public AST_Node {
  public:
    Single_Quoted_String_AST_Node(const std::string &ast_node_text = "", int id = AST_Node_Type::Single_Quoted_String, const char *fname = NULL, int start_line = 0, int start_col = 0, int end_line = 0, int end_col = 0) :
      AST_Node(ast_node_text, id, fname, start_line, start_col, end_line, end_col) { }
    Boxed_Value eval(Dispatch_Engine &ss);
  };
  struct Lambda_AST_Node : public AST_Node {
  public:
    Lambda_AST_Node(const std::string &ast_node_text = "", int id = AST_Node_Type::Lambda, const char *fname = NULL, int start_line = 0, int start_col = 0, int end_line = 0, int end_col = 0) :
      AST_Node(ast_node_text, id, fname, start_line, start_col, end_line, end_col) { }
    Boxed_Value eval(Dispatch_Engine &ss);
  };
  struct Block_AST_Node : public AST_Node {
  public:
    Block_AST_Node(const std::string &ast_node_text = "", int id = AST_Node_Type::Block, const char *fname = NULL, int start_line = 0, int start_col = 0, int end_line = 0, int end_col = 0) :
      AST_Node(ast_node_text, id, fname, start_line, start_col, end_line, end_col) { }
    Boxed_Value eval(Dispatch_Engine &ss);
  };
  struct Def_AST_Node : public AST_Node {
  public:
    Def_AST_Node(const std::string &ast_node_text = "", int id = AST_Node_Type::Def, const char *fname = NULL, int start_line = 0, int start_col = 0, int end_line = 0, int end_col = 0) :
      AST_Node(ast_node_text, id, fname, start_line, start_col, end_line, end_col) { }
    Boxed_Value eval(Dispatch_Engine &ss);
  };
  struct While_AST_Node : public AST_Node {
  public:
    While_AST_Node(const std::string &ast_node_text = "", int id = AST_Node_Type::While, const char *fname = NULL, int start_line = 0, int start_col = 0, int end_line = 0, int end_col = 0) :
      AST_Node(ast_node_text, id, fname, start_line, start_col, end_line, end_col) { }
    Boxed_Value eval(Dispatch_Engine &ss);
  };
  struct If_AST_Node : public AST_Node {
  public:
    If_AST_Node(const std::string &ast_node_text = "", int id = AST_Node_Type::If, const char *fname = NULL, int start_line = 0, int start_col = 0, int end_line = 0, int end_col = 0) :
      AST_Node(ast_node_text, id, fname, start_line, start_col, end_line, end_col) { }
    Boxed_Value eval(Dispatch_Engine &ss);
  };
  struct For_AST_Node : public AST_Node {
  public:
    For_AST_Node(const std::string &ast_node_text = "", int id = AST_Node_Type::For, const char *fname = NULL, int start_line = 0, int start_col = 0, int end_line = 0, int end_col = 0) :
      AST_Node(ast_node_text, id, fname, start_line, start_col, end_line, end_col) { }
    Boxed_Value eval(Dispatch_Engine &ss);
  };
  struct Inline_Array_AST_Node : public AST_Node {
  public:
    Inline_Array_AST_Node(const std::string &ast_node_text = "", int id = AST_Node_Type::Inline_Array, const char *fname = NULL, int start_line = 0, int start_col = 0, int end_line = 0, int end_col = 0) :
      AST_Node(ast_node_text, id, fname, start_line, start_col, end_line, end_col) { }
    Boxed_Value eval(Dispatch_Engine &ss);
  };
  struct Inline_Map_AST_Node : public AST_Node {
  public:
    Inline_Map_AST_Node(const std::string &ast_node_text = "", int id = AST_Node_Type::Inline_Map, const char *fname = NULL, int start_line = 0, int start_col = 0, int end_line = 0, int end_col = 0) :
      AST_Node(ast_node_text, id, fname, start_line, start_col, end_line, end_col) { }
    Boxed_Value eval(Dispatch_Engine &ss);
  };
  struct Return_AST_Node : public AST_Node {
  public:
    Return_AST_Node(const std::string &ast_node_text = "", int id = AST_Node_Type::Return, const char *fname = NULL, int start_line = 0, int start_col = 0, int end_line = 0, int end_col = 0) :
      AST_Node(ast_node_text, id, fname, start_line, start_col, end_line, end_col) { }
    Boxed_Value eval(Dispatch_Engine &ss);
  };
  struct File_AST_Node : public AST_Node {
  public:
    File_AST_Node(const std::string &ast_node_text = "", int id = AST_Node_Type::File, const char *fname = NULL, int start_line = 0, int start_col = 0, int end_line = 0, int end_col = 0) :
      AST_Node(ast_node_text, id, fname, start_line, start_col, end_line, end_col) { }
    Boxed_Value eval(Dispatch_Engine &ss);
  };
  struct Prefix_AST_Node : public AST_Node {
  public:
    Prefix_AST_Node(const std::string &ast_node_text = "", int id = AST_Node_Type::Prefix, const char *fname = NULL, int start_line = 0, int start_col = 0, int end_line = 0, int end_col = 0) :
      AST_Node(ast_node_text, id, fname, start_line, start_col, end_line, end_col) { }
    Boxed_Value eval(Dispatch_Engine &ss);
  };
  struct Break_AST_Node : public AST_Node {
  public:
    Break_AST_Node(const std::string &ast_node_text = "", int id = AST_Node_Type::Break, const char *fname = NULL, int start_line = 0, int start_col = 0, int end_line = 0, int end_col = 0) :
      AST_Node(ast_node_text, id, fname, start_line, start_col, end_line, end_col) { }
    Boxed_Value eval(Dispatch_Engine &ss);
  };
  struct Map_Pair_AST_Node : public AST_Node {
  public:
    Map_Pair_AST_Node(const std::string &ast_node_text = "", int id = AST_Node_Type::Map_Pair, const char *fname = NULL, int start_line = 0, int start_col = 0, int end_line = 0, int end_col = 0) :
      AST_Node(ast_node_text, id, fname, start_line, start_col, end_line, end_col) { }
  };
  struct Value_Range_AST_Node : public AST_Node {
  public:
    Value_Range_AST_Node(const std::string &ast_node_text = "", int id = AST_Node_Type::Value_Range, const char *fname = NULL, int start_line = 0, int start_col = 0, int end_line = 0, int end_col = 0) :
      AST_Node(ast_node_text, id, fname, start_line, start_col, end_line, end_col) { }
  };
  struct Inline_Range_AST_Node : public AST_Node {
  public:
    Inline_Range_AST_Node(const std::string &ast_node_text = "", int id = AST_Node_Type::Inline_Range, const char *fname = NULL, int start_line = 0, int start_col = 0, int end_line = 0, int end_col = 0) :
      AST_Node(ast_node_text, id, fname, start_line, start_col, end_line, end_col) { }
    Boxed_Value eval(Dispatch_Engine &ss);
  };
  struct Annotation_AST_Node : public AST_Node {
  public:
    Annotation_AST_Node(const std::string &ast_node_text = "", int id = AST_Node_Type::Annotation, const char *fname = NULL, int start_line = 0, int start_col = 0, int end_line = 0, int end_col = 0) :
      AST_Node(ast_node_text, id, fname, start_line, start_col, end_line, end_col) { }
  };
  struct Try_AST_Node : public AST_Node {
  public:
    Try_AST_Node(const std::string &ast_node_text = "", int id = AST_Node_Type::Try, const char *fname = NULL, int start_line = 0, int start_col = 0, int end_line = 0, int end_col = 0) :
      AST_Node(ast_node_text, id, fname, start_line, start_col, end_line, end_col) { }
    Boxed_Value eval(Dispatch_Engine &ss);
  };
  struct Catch_AST_Node : public AST_Node {
  public:
    Catch_AST_Node(const std::string &ast_node_text = "", int id = AST_Node_Type::Catch, const char *fname = NULL, int start_line = 0, int start_col = 0, int end_line = 0, int end_col = 0) :
      AST_Node(ast_node_text, id, fname, start_line, start_col, end_line, end_col) { }
  };
  struct Finally_AST_Node : public AST_Node {
  public:
    Finally_AST_Node(const std::string &ast_node_text = "", int id = AST_Node_Type::Finally, const char *fname = NULL, int start_line = 0, int start_col = 0, int end_line = 0, int end_col = 0) :
      AST_Node(ast_node_text, id, fname, start_line, start_col, end_line, end_col) { }
  };
  struct Method_AST_Node : public AST_Node {
  public:
    Method_AST_Node(const std::string &ast_node_text = "", int id = AST_Node_Type::Method, const char *fname = NULL, int start_line = 0, int start_col = 0, int end_line = 0, int end_col = 0) :
      AST_Node(ast_node_text, id, fname, start_line, start_col, end_line, end_col) { }
    Boxed_Value eval(Dispatch_Engine &ss);
  };
  struct Attr_Decl_AST_Node : public AST_Node {
  public:
    Attr_Decl_AST_Node(const std::string &ast_node_text = "", int id = AST_Node_Type::Attr_Decl, const char *fname = NULL, int start_line = 0, int start_col = 0, int end_line = 0, int end_col = 0) :
      AST_Node(ast_node_text, id, fname, start_line, start_col, end_line, end_col) { }
    Boxed_Value eval(Dispatch_Engine &ss);
  };
  struct Shift_AST_Node : public AST_Node {
  public:
    Shift_AST_Node(const std::string &ast_node_text = "", int id = AST_Node_Type::Shift, const char *fname = NULL, int start_line = 0, int start_col = 0, int end_line = 0, int end_col = 0) :
      AST_Node(ast_node_text, id, fname, start_line, start_col, end_line, end_col) { }
    Boxed_Value eval(Dispatch_Engine &ss);
  };
  struct Equality_AST_Node : public AST_Node {
  public:
    Equality_AST_Node(const std::string &ast_node_text = "", int id = AST_Node_Type::Equality, const char *fname = NULL, int start_line = 0, int start_col = 0, int end_line = 0, int end_col = 0) :
      AST_Node(ast_node_text, id, fname, start_line, start_col, end_line, end_col) { }
    Boxed_Value eval(Dispatch_Engine &ss);
  };
  struct Bitwise_And_AST_Node : public AST_Node {
  public:
    Bitwise_And_AST_Node(const std::string &ast_node_text = "", int id = AST_Node_Type::Bitwise_And, const char *fname = NULL, int start_line = 0, int start_col = 0, int end_line = 0, int end_col = 0) :
      AST_Node(ast_node_text, id, fname, start_line, start_col, end_line, end_col) { }
    Boxed_Value eval(Dispatch_Engine &ss);
  };
  struct Bitwise_Xor_AST_Node : public AST_Node {
  public:
    Bitwise_Xor_AST_Node(const std::string &ast_node_text = "", int id = AST_Node_Type::Bitwise_Xor, const char *fname = NULL, int start_line = 0, int start_col = 0, int end_line = 0, int end_col = 0) :
      AST_Node(ast_node_text, id, fname, start_line, start_col, end_line, end_col) { }
    Boxed_Value eval(Dispatch_Engine &ss);
  };
  struct Bitwise_Or_AST_Node : public AST_Node {
  public:
    Bitwise_Or_AST_Node(const std::string &ast_node_text = "", int id = AST_Node_Type::Bitwise_Or, const char *fname = NULL, int start_line = 0, int start_col = 0, int end_line = 0, int end_col = 0) :
      AST_Node(ast_node_text, id, fname, start_line, start_col, end_line, end_col) { }
    Boxed_Value eval(Dispatch_Engine &ss);
  };
  struct Logical_And_AST_Node : public AST_Node {
  public:
    Logical_And_AST_Node(const std::string &ast_node_text = "", int id = AST_Node_Type::Logical_And, const char *fname = NULL, int start_line = 0, int start_col = 0, int end_line = 0, int end_col = 0) :
      AST_Node(ast_node_text, id, fname, start_line, start_col, end_line, end_col) { }
    Boxed_Value eval(Dispatch_Engine &ss);
  };
  struct Logical_Or_AST_Node : public AST_Node {
  public:
    Logical_Or_AST_Node(const std::string &ast_node_text = "", int id = AST_Node_Type::Logical_Or, const char *fname = NULL, int start_line = 0, int start_col = 0, int end_line = 0, int end_col = 0) :
      AST_Node(ast_node_text, id, fname, start_line, start_col, end_line, end_col) { }
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
    std::vector<AST_NodePtr> call_stack;

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

