// This file is distributed under the BSD License.
// See "license.txt" for details.
// Copyright 2009-2012, Jonathan Turner (jonathan@emptycrate.com)
// Copyright 2009-2015, Jason Turner (jason@emptycrate.com)
// http://www.chaiscript.com

#ifndef CHAISCRIPT_COMMON_HPP_
#define CHAISCRIPT_COMMON_HPP_

#include <algorithm>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "../chaiscript_defines.hpp"
#include "../dispatchkit/boxed_value.hpp"
#include "../dispatchkit/dispatchkit.hpp"
#include "../dispatchkit/proxy_functions.hpp"
#include "../dispatchkit/type_info.hpp"

namespace chaiscript {
struct AST_Node;
}  // namespace chaiscript

namespace chaiscript
{

  /// Signature of module entry point that all binary loadable modules must implement.
  typedef ModulePtr (*Create_Module_Func)();


  /// Types of AST nodes available to the parser and eval
  class AST_Node_Type {
  public:
    enum Type { Error, Int, Float, Id, Char, Str, Eol, Fun_Call, Inplace_Fun_Call, Arg_List, Variable, Equation, Var_Decl,
                Comparison, Addition, Subtraction, Multiplication, Division, Modulus, Array_Call, Dot_Access, Quoted_String, Single_Quoted_String,
                Lambda, Block, Def, While, If, For, Inline_Array, Inline_Map, Return, File, Prefix, Break, Continue, Map_Pair, Value_Range,
                Inline_Range, Annotation, Try, Catch, Finally, Method, Attr_Decl, Shift, Equality, Bitwise_And, Bitwise_Xor, Bitwise_Or, 
                Logical_And, Logical_Or, Reference, Switch, Case, Default, Ternary_Cond, Noop, Class, Binary, Arg, Global_Decl
    };
  };

  namespace
  {

    /// Helper lookup to get the name of each node type
    const char *ast_node_type_to_string(int ast_node_type) {
      const char *ast_node_types[] = { "Internal Parser Error", "Int", "Float", "Id", "Char", "Str", "Eol", "Fun_Call", "Inplace_Fun_Call", "Arg_List", "Variable", "Equation", "Var_Decl",
                                    "Comparison", "Addition", "Subtraction", "Multiplication", "Division", "Modulus", "Array_Call", "Dot_Access", "Quoted_String", "Single_Quoted_String",
                                    "Lambda", "Block", "Def", "While", "If", "For", "Inline_Array", "Inline_Map", "Return", "File", "Prefix", "Break", "Continue", "Map_Pair", "Value_Range",
                                    "Inline_Range", "Annotation", "Try", "Catch", "Finally", "Method", "Attr_Decl", "Shift", "Equality", "Bitwise_And", "Bitwise_Xor", "Bitwise_Or", 
                                    "Logical_And", "Logical_Or", "Reference", "Switch", "Case", "Default", "Ternary Condition", "Noop", "Class", "Binary", "Arg"};

      return ast_node_types[ast_node_type];
    }
  }

  /// \brief Convenience type for file positions
  struct File_Position {
    int line;
    int column;

    File_Position(int t_file_line, int t_file_column)
      : line(t_file_line), column(t_file_column) { }

    File_Position() : line(0), column(0) { }
  };

  struct Parse_Location {
    Parse_Location(std::string t_fname="", const int t_start_line=0, const int t_start_col=0,
        const int t_end_line=0, const int t_end_col=0)
      : start(t_start_line, t_start_col), 
        end(t_end_line, t_end_col),
        filename(std::make_shared<std::string>(std::move(t_fname)))
    {
    }

    Parse_Location(std::shared_ptr<std::string> t_fname, const int t_start_line=0, const int t_start_col=0,
        const int t_end_line=0, const int t_end_col=0)
      : start(t_start_line, t_start_col), 
        end(t_end_line, t_end_col),
        filename(std::move(t_fname))
    {
    }



    File_Position start;
    File_Position end;
    std::shared_ptr<std::string> filename;
  };


  /// \brief Typedef for pointers to AST_Node objects. Used in building of the AST_Node tree
  typedef std::shared_ptr<AST_Node> AST_NodePtr;
  typedef std::shared_ptr<const AST_Node> AST_NodePtr_Const;


  /// \brief Classes which may be thrown during error cases when ChaiScript is executing.
  namespace exception
  {

    /// Errors generated during parsing or evaluation
    struct eval_error : std::runtime_error {
      std::string reason;
      File_Position start_position;
      std::string filename;
      std::string detail;
      std::vector<AST_NodePtr_Const> call_stack;

      eval_error(const std::string &t_why, const File_Position &t_where, const std::string &t_fname,
          const std::vector<Boxed_Value> &t_parameters, const std::vector<chaiscript::Const_Proxy_Function> &t_functions,
          bool t_dot_notation,
          const chaiscript::detail::Dispatch_Engine &t_ss) CHAISCRIPT_NOEXCEPT :
        std::runtime_error(format(t_why, t_where, t_fname, t_parameters, t_dot_notation, t_ss)),
        reason(t_why), start_position(t_where), filename(t_fname), detail(format_detail(t_functions, t_dot_notation, t_ss)) 
      {}

      eval_error(const std::string &t_why, 
           const std::vector<Boxed_Value> &t_parameters, const std::vector<chaiscript::Const_Proxy_Function> &t_functions,
           bool t_dot_notation,
           const chaiscript::detail::Dispatch_Engine &t_ss) CHAISCRIPT_NOEXCEPT :
        std::runtime_error(format(t_why, t_parameters, t_dot_notation, t_ss)),
        reason(t_why), detail(format_detail(t_functions, t_dot_notation, t_ss))
      {}


      eval_error(const std::string &t_why, const File_Position &t_where, const std::string &t_fname) CHAISCRIPT_NOEXCEPT :
        std::runtime_error(format(t_why, t_where, t_fname)),
        reason(t_why), start_position(t_where), filename(t_fname)
      {}

      eval_error(const std::string &t_why) CHAISCRIPT_NOEXCEPT
        : std::runtime_error("Error: \"" + t_why + "\" "),
        reason(t_why) 
      {}

      eval_error(const eval_error &) = default;

      std::string pretty_print() const
      {
        std::ostringstream ss;

        ss << what();
        if (call_stack.size() > 0) {
          ss << "during evaluation at (" << fname(call_stack[0]) << " " << startpos(call_stack[0]) << ")\n";
          ss << '\n' << detail << '\n';
          ss << "  " << fname(call_stack[0]) << " (" << startpos(call_stack[0]) << ") '" << pretty(call_stack[0]) << "'";
          for (size_t j = 1; j < call_stack.size(); ++j) {
            if (id(call_stack[j]) != chaiscript::AST_Node_Type::Block
                && id(call_stack[j]) != chaiscript::AST_Node_Type::File)
            {
              ss << '\n';
              ss << "  from " << fname(call_stack[j]) << " (" << startpos(call_stack[j]) << ") '" << pretty(call_stack[j]) << "'";
            }
          }
        }
        ss << '\n';
        return ss.str();
      }

      virtual ~eval_error() CHAISCRIPT_NOEXCEPT {}

    private:

      template<typename T>
        static int id(const T& t)
        {
          return t->identifier;
        }

      template<typename T>
        static std::string pretty(const T& t)
        {
          return t->pretty_print();
        }

      template<typename T>
        static const std::string &fname(const T& t)
        {
          return t->filename();
        }

      template<typename T>
        static std::string startpos(const T& t)
        {
          std::ostringstream oss;
          oss << t->start().line << ", " << t->start().column;
          return oss.str();
        }

      static std::string format_why(const std::string &t_why)
      {
        return "Error: \"" + t_why + "\"";
      }

      static std::string format_types(const Const_Proxy_Function &t_func,
          bool t_dot_notation,
          const chaiscript::detail::Dispatch_Engine &t_ss)
      {
        int arity = t_func->get_arity();
        std::vector<Type_Info> types = t_func->get_param_types();

        std::string retval;
        if (arity == -1)
        {
          retval = "(...)";
          if (t_dot_notation)
          {
            retval = "(Object)." + retval;
          }
        } else if (types.size() <= 1) {
          retval = "()";
        } else {
          std::stringstream ss;
          ss << "(";

          std::string paramstr;

          for (size_t index = 1;
               index != types.size();
               ++index)
          {
            paramstr += (types[index].is_const()?"const ":"");
            paramstr += t_ss.get_type_name(types[index]);

            if (index == 1 && t_dot_notation)
            {
              paramstr += ").(";
              if (types.size() == 2)
              {
                paramstr += ", ";
              }
            } else {
              paramstr += ", ";
            }
          }

          ss << paramstr.substr(0, paramstr.size() - 2);

          ss << ")";
          retval = ss.str();
        }


        std::shared_ptr<const dispatch::Dynamic_Proxy_Function> dynfun 
          = std::dynamic_pointer_cast<const dispatch::Dynamic_Proxy_Function>(t_func);

        if (dynfun)
        {
          Proxy_Function f = dynfun->get_guard();

          if (f)
          {
            auto dynfunguard = std::dynamic_pointer_cast<const dispatch::Dynamic_Proxy_Function>(f);
            if (dynfunguard)
            {
              retval += " : " + format_guard(dynfunguard->get_parse_tree());
            }
          }

          retval += "\n          Defined at " + format_location(dynfun->get_parse_tree());        
        }

        return retval;
      }

      template<typename T>
        static std::string format_guard(const T &t)
        {
          return t->pretty_print();
        }

      template<typename T>
        static std::string format_location(const T &t)
        {
          if (t) {
            std::ostringstream oss;
            oss << "(" << t->filename() << " " << t->start().line << ", " << t->start().column << ")"; 
            return oss.str();
          } else {
            return "(internal)";
          }

        }

      static std::string format_detail(const std::vector<chaiscript::Const_Proxy_Function> &t_functions,
          bool t_dot_notation,
          const chaiscript::detail::Dispatch_Engine &t_ss)
      {
        std::stringstream ss;
        if (t_functions.size() == 1)
        {
          ss << "  Expected: " << format_types(t_functions[0], t_dot_notation, t_ss) << '\n';
        } else {
          ss << "  " << t_functions.size() << " overloads available:\n";

          for (const auto & t_function : t_functions)
          {
            ss << "      " << format_types((t_function), t_dot_notation, t_ss) << '\n';
          }

        }

        return ss.str();

      }

      static std::string format_parameters(const std::vector<Boxed_Value> &t_parameters,
          bool t_dot_notation,
          const chaiscript::detail::Dispatch_Engine &t_ss)
      {
        std::stringstream ss;
        ss << "(";

        if (!t_parameters.empty())
        {
          std::string paramstr;

          for (auto itr = t_parameters.begin();
               itr != t_parameters.end();
               ++itr)
          {
            paramstr += (itr->is_const()?"const ":"");
            paramstr += t_ss.type_name(*itr);

            if (itr == t_parameters.begin() && t_dot_notation)
            {
              paramstr += ").(";
              if (t_parameters.size() == 1)
              {
                paramstr += ", ";
              }
            } else {
              paramstr += ", ";
            }
          }

          ss << paramstr.substr(0, paramstr.size() - 2);
        }
        ss << ")";

        return ss.str();
      }

      static std::string format_filename(const std::string &t_fname)
      {
        std::stringstream ss;

        if (t_fname != "__EVAL__")
        {
          ss << "in '" << t_fname << "' ";
        } else {
          ss << "during evaluation ";
        }

        return ss.str();
      }

      static std::string format_location(const File_Position &t_where)
      {
        std::stringstream ss;
        ss << "at (" << t_where.line << ", " << t_where.column << ")";
        return ss.str();
      }

      static std::string format(const std::string &t_why, const File_Position &t_where, const std::string &t_fname,
          const std::vector<Boxed_Value> &t_parameters, bool t_dot_notation, const chaiscript::detail::Dispatch_Engine &t_ss)
      {
        std::stringstream ss;

        ss << format_why(t_why);
        ss << " ";

        ss << "With parameters: " << format_parameters(t_parameters, t_dot_notation, t_ss);
        ss << " ";

        ss << format_filename(t_fname);
        ss << " ";

        ss << format_location(t_where);

        return ss.str();
      }

      static std::string format(const std::string &t_why, 
          const std::vector<Boxed_Value> &t_parameters, 
          bool t_dot_notation,
          const chaiscript::detail::Dispatch_Engine &t_ss)
      {
        std::stringstream ss;

        ss << format_why(t_why);
        ss << " ";

        ss << "With parameters: " << format_parameters(t_parameters, t_dot_notation, t_ss);
        ss << " ";

        return ss.str();
      }

      static std::string format(const std::string &t_why, const File_Position &t_where, const std::string &t_fname)
      {
        std::stringstream ss;

        ss << format_why(t_why);
        ss << " ";

        ss << format_filename(t_fname);
        ss << " ";

        ss << format_location(t_where);

        return ss.str();
      }
    };


    /// Errors generated when loading a file
    struct file_not_found_error : std::runtime_error {
      file_not_found_error(const std::string &t_filename) CHAISCRIPT_NOEXCEPT
        : std::runtime_error("File Not Found: " + t_filename)
      { }

      file_not_found_error(const file_not_found_error &) = default;
      virtual ~file_not_found_error() CHAISCRIPT_NOEXCEPT {}
    };

  }

 
  /// \brief Struct that doubles as both a parser ast_node and an AST node.
  struct AST_Node : std::enable_shared_from_this<AST_Node> {
    public:
      const int identifier; //< \todo shouldn't this be a strongly typed enum value?
      const std::string text;
      Parse_Location location;
      std::vector<AST_NodePtr> children;
      AST_NodePtr annotation;

      const std::string &filename() const {
        return *location.filename;
      }

      const File_Position &start() const {
        return location.start;
      }

      const File_Position &end() const {
        return location.end;
      }

      virtual std::string pretty_print() const
      {
        std::ostringstream oss;

        oss << text;

        for (auto & elem : this->children) {
          oss << elem->pretty_print();
        }

        return oss.str();
      }


      /// Prints the contents of an AST node, including its children, recursively
      std::string to_string(const std::string &t_prepend = "") const {
        std::ostringstream oss;

        oss << t_prepend << "(" << ast_node_type_to_string(this->identifier) << ") "
            << this->text << " : " << this->location.start.line << ", " << this->location.start.column << '\n';

        for (auto & elem : this->children) {
          oss << elem->to_string(t_prepend + "  ");
        }
        return oss.str();
      }

      Boxed_Value eval(const chaiscript::detail::Dispatch_State &t_e) const
      {
        try {
          return eval_internal(t_e);
        } catch (exception::eval_error &ee) {
          ee.call_stack.push_back(shared_from_this());
          throw;
        }
      }

      static bool get_bool_condition(const Boxed_Value &t_bv) {
        try {
          return boxed_cast<bool>(t_bv);
        }
        catch (const exception::bad_boxed_cast &) {
          throw exception::eval_error("Condition not boolean");
        }
      }


      void replace_child(const AST_NodePtr &t_child, const AST_NodePtr &t_new_child)
      {
        std::replace(children.begin(), children.end(), t_child, t_new_child);
      }

      virtual ~AST_Node() {}

    protected:
      AST_Node(std::string t_ast_node_text, int t_id, Parse_Location t_loc, 
               std::vector<AST_NodePtr> t_children = std::vector<AST_NodePtr>()) :
        identifier(t_id), text(std::move(t_ast_node_text)),
        location(std::move(t_loc)),
        children(std::move(t_children))
      {
      }

      virtual Boxed_Value eval_internal(const chaiscript::detail::Dispatch_State &) const
      {
        throw std::runtime_error("Undispatched ast_node (internal error)");
      }

    private:
      // Copy and assignment explicitly unimplemented
      AST_Node(const AST_Node &) = delete;
      AST_Node& operator=(const AST_Node &) = delete;
  };


  namespace eval
  {
    namespace detail
    {
      /// Special type for returned values
      struct Return_Value {
        Boxed_Value retval;

        Return_Value(Boxed_Value t_return_value) : retval(std::move(t_return_value)) { }
      };


      /// Special type indicating a call to 'break'
      struct Break_Loop {
        Break_Loop() { }
      };


      /// Special type indicating a call to 'continue'
      struct Continue_Loop {
        Continue_Loop() { }
      };


      /// Creates a new scope then pops it on destruction
      struct Scope_Push_Pop
      {
        Scope_Push_Pop(const Scope_Push_Pop &) = delete;
        Scope_Push_Pop& operator=(const Scope_Push_Pop &) = delete;

        Scope_Push_Pop(const chaiscript::detail::Dispatch_State &t_ds)
          : m_ds(t_ds)
        {
          m_ds.get()->new_scope(m_ds.get().stack_holder());
        }

        ~Scope_Push_Pop()
        {
          m_ds.get()->pop_scope(m_ds.get().stack_holder());
        }


        private:
        std::reference_wrapper<const chaiscript::detail::Dispatch_State> m_ds;
      };

      /// Creates a new function call and pops it on destruction
      struct Function_Push_Pop
      {
        Function_Push_Pop(const Function_Push_Pop &) = delete;
        Function_Push_Pop& operator=(const Function_Push_Pop &) = delete;

        Function_Push_Pop(const chaiscript::detail::Dispatch_State &t_ds)
          : m_ds(t_ds)
        {
          m_ds.get()->new_function_call(m_ds.get().stack_holder());
        }

        ~Function_Push_Pop()
        {
          m_ds.get()->pop_function_call(m_ds.get().stack_holder());
        }

        void save_params(const std::vector<Boxed_Value> &t_params)
        {
          m_ds.get()->save_function_params(t_params);
        }

        void save_params(std::initializer_list<Boxed_Value> t_params)
        {
          m_ds.get()->save_function_params(std::move(t_params));
        }


        private:
          std::reference_wrapper<const chaiscript::detail::Dispatch_State> m_ds;
      };

      /// Creates a new scope then pops it on destruction
      struct Stack_Push_Pop
      {
        Stack_Push_Pop(const Stack_Push_Pop &) = delete;
        Stack_Push_Pop& operator=(const Stack_Push_Pop &) = delete;

        Stack_Push_Pop(const chaiscript::detail::Dispatch_State &t_ds)
          : m_ds(t_ds)
        {
          m_ds.get()->new_stack(m_ds.get().stack_holder());
        }

        ~Stack_Push_Pop()
        {
          m_ds.get()->pop_stack(m_ds.get().stack_holder());
        }


        private:
          std::reference_wrapper<const chaiscript::detail::Dispatch_State> m_ds;
      };
    }
  }
}

#endif /* _CHAISCRIPT_COMMON_HPP */

