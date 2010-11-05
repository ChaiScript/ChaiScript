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
    boost::shared_ptr<std::string> filename;
    File_Position start, end;
    std::vector<AST_NodePtr> children;
    AST_NodePtr annotation;

    AST_Node(const std::string &ast_node_text, int id, const boost::shared_ptr<std::string> &fname, int start_line, int start_col, int end_line, int end_col) :
      text(ast_node_text), identifier(id), filename(fname),
      start(start_line, start_col), end(end_line, end_col)
    {
    }

    AST_Node(const std::string &ast_node_text, int id, const boost::shared_ptr<std::string> &fname) :
      text(ast_node_text), identifier(id), filename(fname) {}

    virtual ~AST_Node() {}

    /**
     * Prints the contents of an AST node, including its children, recursively
     */
    std::string to_string(std::string prepend = "") {
      std::ostringstream oss;

      oss << prepend << "(" << ast_node_type_to_string(this->identifier) << ") "
          << this->text << " : " << this->start.line << ", " << this->start.column << std::endl;
      
      for (unsigned int j = 0; j < this->children.size(); ++j) {
        oss << this->children[j]->to_string(prepend + "  ");
      }
      return oss.str();
    }

    std::string internal_to_string() {
      return to_string();
    }

    virtual Boxed_Value eval(Dispatch_Engine &) {
      Boxed_Value bv;
      throw std::runtime_error("Undispatched ast_node (internal error)");
    }
  };


  

  /**
   * Errors generated during parsing or evaluation
   */
  struct Eval_Error : public std::runtime_error {
    std::string reason;
    File_Position start_position;
    File_Position end_position;
    std::string filename;
    std::vector<AST_NodePtr> call_stack;

    Eval_Error(const std::string &why, const File_Position &where, const std::string &fname) :
      std::runtime_error("Error: \"" + why + "\" " +
                         (std::string(fname) != "__EVAL__" ? ("in '" + fname + "' ") : "during evaluation ") +
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

