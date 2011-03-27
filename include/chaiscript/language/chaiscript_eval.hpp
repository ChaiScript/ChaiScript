// This file is distributed under the BSD License.
// See "license.txt" for details.
// Copyright 2009-2011, Jonathan Turner (jonathan@emptycrate.com)
// and Jason Turner (jason@emptycrate.com)
// http://www.chaiscript.com

#ifndef CHAISCRIPT_EVAL_HPP_
#define CHAISCRIPT_EVAL_HPP_

#include <map>

#include <chaiscript/language/chaiscript_common.hpp>

namespace chaiscript
{
  /**
   * Helper function that will set up the scope around a function call, including handling the named function parameters
   */
  template <typename Eval_System>
    const Boxed_Value eval_function (Eval_System &t_ss, const AST_NodePtr &t_node, const std::vector<std::string> &t_param_names, const std::vector<Boxed_Value> &t_vals) {
      t_ss.new_scope();

      for (unsigned int i = 0; i < t_param_names.size(); ++i) {
        t_ss.add_object(t_param_names[i], t_vals[i]);
      }

      try {
        Boxed_Value retval(t_node->eval(t_ss));
        t_ss.pop_scope();
        return retval;
      } catch (const detail::Return_Value &rv) {
        t_ss.pop_scope();
        return rv.retval;
      } catch (exception::eval_error &ee) {
        ee.call_stack.push_back(t_node);
        t_ss.pop_scope();
        throw;
      } catch (...) {
        t_ss.pop_scope();
        throw;
      }
    }


  namespace eval
  {
    struct Binary_Operator_AST_Node : public AST_Node {
      public:
        Binary_Operator_AST_Node(const std::string &t_ast_node_text = "", int t_id = AST_Node_Type::Bitwise_Xor, const boost::shared_ptr<std::string> &t_fname=boost::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          AST_Node(t_ast_node_text, t_id, t_fname, t_start_line, t_start_col, t_end_line, t_end_col) { }
        virtual ~Binary_Operator_AST_Node() {}
        virtual Boxed_Value eval(chaiscript::detail::Dispatch_Engine &t_ss){
          Boxed_Value retval;

          try {
            retval = this->children[0]->eval(t_ss);
          }
          catch (exception::eval_error &ee) {
            ee.call_stack.push_back(this->children[0]);
            throw;
          }

          for (size_t i = 1; i < this->children.size(); i += 2) {
            try {
              retval = t_ss.call_function(this->children[i]->text, retval, this->children[i+1]->eval(t_ss));
            }
            catch(const exception::dispatch_error &){
              throw exception::eval_error("Can not find appropriate '" + this->children[i]->text + "'");
            }
            catch(exception::eval_error &ee) {
              ee.call_stack.push_back(this->children[i+1]);
              throw;
            }
          }

          return retval;
        }

    };

    struct Error_AST_Node : public AST_Node {
      public:
        Error_AST_Node(const std::string &t_ast_node_text = "", int t_id = AST_Node_Type::Error, const boost::shared_ptr<std::string> &t_fname=boost::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          AST_Node(t_ast_node_text, t_id, t_fname, t_start_line, t_start_col, t_end_line, t_end_col) { }

        virtual ~Error_AST_Node() {}
    };

    struct Int_AST_Node : public AST_Node {
      public:
        Int_AST_Node(const std::string &t_ast_node_text = "", int t_id = AST_Node_Type::Int, const boost::shared_ptr<std::string> &t_fname=boost::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          AST_Node(t_ast_node_text, t_id, t_fname, t_start_line, t_start_col, t_end_line, t_end_col), m_value(const_var(int(atoi(this->text.c_str())))) { }
        virtual ~Int_AST_Node() {}
        virtual Boxed_Value eval(chaiscript::detail::Dispatch_Engine &){
          return m_value;
        }

      private:
        Boxed_Value m_value;

    };

    struct Float_AST_Node : public AST_Node {
      public:
        Float_AST_Node(const std::string &t_ast_node_text = "", int t_id = AST_Node_Type::Float, const boost::shared_ptr<std::string> &t_fname=boost::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          AST_Node(t_ast_node_text, t_id, t_fname, t_start_line, t_start_col, t_end_line, t_end_col),
          m_value(const_var(double(atof(this->text.c_str())))) { }
        virtual ~Float_AST_Node() {}
        virtual Boxed_Value eval(chaiscript::detail::Dispatch_Engine &){
          return m_value;
        }

      private:
        Boxed_Value m_value;

    };

    struct Id_AST_Node : public AST_Node {
      public:
        Id_AST_Node(const std::string &t_ast_node_text = "", int t_id = AST_Node_Type::Id, const boost::shared_ptr<std::string> &t_fname=boost::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          AST_Node(t_ast_node_text, t_id, t_fname, t_start_line, t_start_col, t_end_line, t_end_col) { }
        virtual ~Id_AST_Node() {}
        virtual Boxed_Value eval(chaiscript::detail::Dispatch_Engine &t_ss){
          if (this->text == "true") {
            return const_var(true);
          }
          else if (this->text == "false") {
            return const_var(false);
          }
          else if (this->text == "Infinity") {
            return const_var(std::numeric_limits<double>::infinity());
          }
          else if (this->text == "NaN") {
            return const_var(std::numeric_limits<double>::quiet_NaN());
          }
          else {
            try {
              return t_ss.get_object(this->text);
            }
            catch (std::exception &) {
              throw exception::eval_error("Can not find object: " + this->text);
            }
          }
        }
    };

    struct Char_AST_Node : public AST_Node {
      public:
        Char_AST_Node(const std::string &t_ast_node_text = "", int t_id = AST_Node_Type::Char, const boost::shared_ptr<std::string> &t_fname=boost::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          AST_Node(t_ast_node_text, t_id, t_fname, t_start_line, t_start_col, t_end_line, t_end_col) { }
        virtual ~Char_AST_Node() {}
    };

    struct Str_AST_Node : public AST_Node {
      public:
        Str_AST_Node(const std::string &t_ast_node_text = "", int t_id = AST_Node_Type::Str, const boost::shared_ptr<std::string> &t_fname=boost::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          AST_Node(t_ast_node_text, t_id, t_fname, t_start_line, t_start_col, t_end_line, t_end_col) { }
        virtual ~Str_AST_Node() {}
    };

    struct Eol_AST_Node : public AST_Node {
      public:
        Eol_AST_Node(const std::string &t_ast_node_text = "", int t_id = AST_Node_Type::Eol, const boost::shared_ptr<std::string> &t_fname=boost::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          AST_Node(t_ast_node_text, t_id, t_fname, t_start_line, t_start_col, t_end_line, t_end_col) { }
        virtual ~Eol_AST_Node() {}
    };

    struct Fun_Call_AST_Node : public AST_Node {
      public:
        Fun_Call_AST_Node(const std::string &t_ast_node_text = "", int t_id = AST_Node_Type::Fun_Call, const boost::shared_ptr<std::string> &t_fname=boost::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          AST_Node(t_ast_node_text, t_id, t_fname, t_start_line, t_start_col, t_end_line, t_end_col) { }
        virtual ~Fun_Call_AST_Node() {}
        virtual Boxed_Value eval(chaiscript::detail::Dispatch_Engine &t_ss){
          dispatch::Param_List_Builder plb;

          if ((this->children.size() > 1) && (this->children[1]->identifier == AST_Node_Type::Arg_List)) {
            for (size_t i = 0; i < this->children[1]->children.size(); ++i) {
              try {
                plb << this->children[1]->children[i]->eval(t_ss);
              }
              catch(exception::eval_error &ee) {
                ee.call_stack.push_back(this->children[1]->children[i]);
                throw;
              }
            }
          }

          chaiscript::detail::Dispatch_Engine::Stack prev_stack = t_ss.get_stack();
          chaiscript::detail::Dispatch_Engine::Stack new_stack = t_ss.new_stack();

          try {
            Boxed_Value fn = this->children[0]->eval(t_ss);

            try {
              t_ss.set_stack(new_stack);
              const Boxed_Value &retval = (*boxed_cast<Const_Proxy_Function>(fn))(plb);
              t_ss.set_stack(prev_stack);
              return retval;
            }
            catch(const exception::dispatch_error &e){
              t_ss.set_stack(prev_stack);
              throw exception::eval_error(std::string(e.what()) + " with function '" + this->children[0]->text + "'");
            }
            catch(detail::Return_Value &rv) {
              t_ss.set_stack(prev_stack);
              return rv.retval;
            }
            catch(...) {
              t_ss.set_stack(prev_stack);
              throw;
            }
          }
          catch(exception::eval_error &ee) {
            ee.call_stack.push_back(this->children[0]);
            t_ss.set_stack(prev_stack);
            throw exception::eval_error(ee.reason);
          }

        }

    };

    struct Inplace_Fun_Call_AST_Node : public AST_Node {
      public:
        Inplace_Fun_Call_AST_Node(const std::string &t_ast_node_text = "", int t_id = AST_Node_Type::Inplace_Fun_Call, const boost::shared_ptr<std::string> &t_fname=boost::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          AST_Node(t_ast_node_text, t_id, t_fname, t_start_line, t_start_col, t_end_line, t_end_col) { }
        virtual ~Inplace_Fun_Call_AST_Node() {}
        virtual Boxed_Value eval(chaiscript::detail::Dispatch_Engine &t_ss){
          dispatch::Param_List_Builder plb;

          if ((this->children.size() > 1) && (this->children[1]->identifier == AST_Node_Type::Arg_List)) {
            for (size_t i = 0; i < this->children[1]->children.size(); ++i) {
              try {
                plb << this->children[1]->children[i]->eval(t_ss);
              }
              catch (exception::eval_error &ee) {
                ee.call_stack.push_back(this->children[1]->children[i]);
                throw;
              }
            }
          }

          try {
            Boxed_Value fn = this->children[0]->eval(t_ss);

            try {
              return (*boxed_cast<Const_Proxy_Function >(fn))(plb);
            }
            catch(const exception::dispatch_error &e){
              throw exception::eval_error(std::string(e.what()) + " with function '" + this->children[0]->text + "'");
            }
            catch(detail::Return_Value &rv) {
              return rv.retval;
            }
            catch(...) {
              throw;
            }
          }
          catch(exception::eval_error &ee) {
            ee.call_stack.push_back(this->children[0]);
            throw exception::eval_error(ee.reason);
          }

        }

    };

    struct Arg_List_AST_Node : public AST_Node {
      public:
        Arg_List_AST_Node(const std::string &t_ast_node_text = "", int t_id = AST_Node_Type::Arg_List, const boost::shared_ptr<std::string> &t_fname=boost::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          AST_Node(t_ast_node_text, t_id, t_fname, t_start_line, t_start_col, t_end_line, t_end_col) { }
        virtual ~Arg_List_AST_Node() {}
    };

    struct Variable_AST_Node : public AST_Node {
      public:
        Variable_AST_Node(const std::string &t_ast_node_text = "", int t_id = AST_Node_Type::Variable, const boost::shared_ptr<std::string> &t_fname=boost::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          AST_Node(t_ast_node_text, t_id, t_fname, t_start_line, t_start_col, t_end_line, t_end_col) { }
        virtual ~Variable_AST_Node() {}
    };

    struct Equation_AST_Node : public AST_Node {
      public:
        Equation_AST_Node(const std::string &t_ast_node_text = "", int t_id = AST_Node_Type::Equation, const boost::shared_ptr<std::string> &t_fname=boost::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          AST_Node(t_ast_node_text, t_id, t_fname, t_start_line, t_start_col, t_end_line, t_end_col) { }
        virtual ~Equation_AST_Node() {}
        virtual Boxed_Value eval(chaiscript::detail::Dispatch_Engine &t_ss){
          Boxed_Value retval;
          try {
            retval = this->children.back()->eval(t_ss); 
          }
          catch (exception::eval_error &ee) {
            ee.call_stack.push_back(this->children.back());
            throw;
          }

          if (this->children.size() > 1) {
            for (int i = static_cast<int>(this->children.size())-3; i >= 0; i -= 2) {
              if (this->children[i+1]->text == "=") {
                try {
                  Boxed_Value lhs = this->children[i]->eval(t_ss);

                  try {
                    if (lhs.is_undef()) {
                      retval = t_ss.call_function("clone", retval);
                      retval.clear_dependencies();
                    }

                    try {
                      retval = t_ss.call_function(this->children[i+1]->text, lhs, retval);
                    }
                    catch(const exception::dispatch_error &){
                      throw exception::eval_error(std::string("Mismatched types in equation") + (lhs.is_const()?", lhs is const.":"."));
                    }
                  }
                  catch(const exception::dispatch_error &){
                    throw exception::eval_error("Can not clone right hand side of equation");
                  }
                }
                catch(exception::eval_error &ee) {
                  ee.call_stack.push_back(this->children[i]);
                  throw;
                }
              }
              else if (this->children[i+1]->text == ":=") {
                try {
                  Boxed_Value lhs = this->children[i]->eval(t_ss);
                  if (lhs.is_undef() || type_match(lhs, retval)) {
                    lhs.assign(retval);
                  }
                  else {
                    throw exception::eval_error("Mismatched types in equation");
                  }
                }
                catch (exception::eval_error &ee) {
                  ee.call_stack.push_back(this->children[i]);
                  throw;
                }
              }
              else {
                try {
                  retval = t_ss.call_function(this->children[i+1]->text, this->children[i]->eval(t_ss), retval);
                }
                catch(const exception::dispatch_error &){
                  throw exception::eval_error("Can not find appropriate '" + this->children[i+1]->text + "'");
                }
                catch(exception::eval_error &ee) {
                  ee.call_stack.push_back(this->children[i]);
                  throw;
                }
              }
            }
          }
          return retval;
        }
    };

    struct Var_Decl_AST_Node : public AST_Node {
      public:
        Var_Decl_AST_Node(const std::string &t_ast_node_text = "", int t_id = AST_Node_Type::Var_Decl, const boost::shared_ptr<std::string> &t_fname=boost::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          AST_Node(t_ast_node_text, t_id, t_fname, t_start_line, t_start_col, t_end_line, t_end_col) { }
        virtual ~Var_Decl_AST_Node() {}
        virtual Boxed_Value eval(chaiscript::detail::Dispatch_Engine &t_ss){
          try {
            t_ss.add_object(this->children[0]->text, Boxed_Value());
          }
          catch (const exception::reserved_word_error &) {
            throw exception::eval_error("Reserved word used as variable '" + this->children[0]->text + "'");
          }
          return t_ss.get_object(this->children[0]->text);
        }

    };

    struct Comparison_AST_Node : public Binary_Operator_AST_Node {
      public:
        Comparison_AST_Node(const std::string &t_ast_node_text = "", int t_id = AST_Node_Type::Comparison, const boost::shared_ptr<std::string> &t_fname=boost::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          Binary_Operator_AST_Node(t_ast_node_text, t_id, t_fname, t_start_line, t_start_col, t_end_line, t_end_col) { }
        virtual ~Comparison_AST_Node() {}
    };

    struct Additive_AST_Node : public Binary_Operator_AST_Node {
      public:
        Additive_AST_Node(const std::string &t_ast_node_text = "", int t_id = AST_Node_Type::Additive, const boost::shared_ptr<std::string> &t_fname=boost::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          Binary_Operator_AST_Node(t_ast_node_text, t_id, t_fname, t_start_line, t_start_col, t_end_line, t_end_col) { }
        virtual ~Additive_AST_Node() {}
    };

    struct Multiplicative_AST_Node : public Binary_Operator_AST_Node {
      public:
        Multiplicative_AST_Node(const std::string &t_ast_node_text = "", int t_id = AST_Node_Type::Multiplicative, const boost::shared_ptr<std::string> &t_fname=boost::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          Binary_Operator_AST_Node(t_ast_node_text, t_id, t_fname, t_start_line, t_start_col, t_end_line, t_end_col) { }
        virtual ~Multiplicative_AST_Node() {}
    };

    struct Array_Call_AST_Node : public AST_Node {
      public:
        Array_Call_AST_Node(const std::string &t_ast_node_text = "", int t_id = AST_Node_Type::Array_Call, const boost::shared_ptr<std::string> &t_fname=boost::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          AST_Node(t_ast_node_text, t_id, t_fname, t_start_line, t_start_col, t_end_line, t_end_col) { }
        virtual ~Array_Call_AST_Node() {}
        virtual Boxed_Value eval(chaiscript::detail::Dispatch_Engine &t_ss){
          Boxed_Value retval;

          try {
            retval = this->children[0]->eval(t_ss);
          }
          catch (exception::eval_error &ee) {
            ee.call_stack.push_back(this->children[0]);
            throw;
          }

          for (size_t i = 1; i < this->children.size(); ++i) {
            try {
              retval = t_ss.call_function("[]", retval, this->children[i]->eval(t_ss));
            }
            catch(std::out_of_range &) {
              throw exception::eval_error("Out of bounds exception");
            }
            catch(const exception::dispatch_error &){
              throw exception::eval_error("Can not find appropriate array lookup '[]' ");
            }
            catch(exception::eval_error &ee) {
              ee.call_stack.push_back(this->children[i]);
              throw;
            }
          }

          return retval;
        }
    };

    struct Dot_Access_AST_Node : public AST_Node {
      public:
        Dot_Access_AST_Node(const std::string &t_ast_node_text = "", int t_id = AST_Node_Type::Dot_Access, const boost::shared_ptr<std::string> &t_fname=boost::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          AST_Node(t_ast_node_text, t_id, t_fname, t_start_line, t_start_col, t_end_line, t_end_col) { }
        virtual ~Dot_Access_AST_Node() {}
        virtual Boxed_Value eval(chaiscript::detail::Dispatch_Engine &t_ss){
          Boxed_Value retval;
          try {
            retval = this->children[0]->eval(t_ss);
          }
          catch (exception::eval_error &ee) {
            ee.call_stack.push_back(this->children[0]);
            throw;
          }

          if (this->children.size() > 1) {
            for (size_t i = 2; i < this->children.size(); i+=2) {
              dispatch::Param_List_Builder plb;
              plb << retval;

              if (this->children[i]->children.size() > 1) {
                for (size_t j = 0; j < this->children[i]->children[1]->children.size(); ++j) {
                  try {
                    plb << this->children[i]->children[1]->children[j]->eval(t_ss);
                  }
                  catch (exception::eval_error &ee) {
                    ee.call_stack.push_back(this->children[i]->children[1]->children[j]);
                    throw;
                  }
                }
              }

              std::string fun_name;
              if ((this->children[i]->identifier == AST_Node_Type::Fun_Call) || (this->children[i]->identifier == AST_Node_Type::Array_Call)) {
                fun_name = this->children[i]->children[0]->text;
              }
              else {
                fun_name = this->children[i]->text;
              }

              chaiscript::detail::Dispatch_Engine::Stack prev_stack = t_ss.get_stack();
              chaiscript::detail::Dispatch_Engine::Stack new_stack = t_ss.new_stack();

              try {
                t_ss.set_stack(new_stack);
                retval = t_ss.call_function(fun_name, plb);
                t_ss.set_stack(prev_stack);
              }
              catch(const exception::dispatch_error &e){
                t_ss.set_stack(prev_stack);
                throw exception::eval_error(std::string(e.what()) + " for function: " + fun_name);
              }
              catch(detail::Return_Value &rv) {
                t_ss.set_stack(prev_stack);
                retval = rv.retval;
              }
              catch(...) {
                t_ss.set_stack(prev_stack);
                throw;
              }
              if (this->children[i]->identifier == AST_Node_Type::Array_Call) {
                for (size_t j = 1; j < this->children[i]->children.size(); ++j) {
                  try {
                    retval = t_ss.call_function("[]", retval, this->children[i]->children[j]->eval(t_ss));
                  }
                  catch(std::out_of_range &) {
                    throw exception::eval_error("Out of bounds exception");
                  }
                  catch(const exception::dispatch_error &){
                    throw exception::eval_error("Can not find appropriate array lookup '[]' ");
                  }
                  catch(exception::eval_error &ee) {
                    ee.call_stack.push_back(this->children[i]->children[j]);
                    throw;
                  }
                }
              }
            }
          }

          return retval;
        }

    };

    struct Quoted_String_AST_Node : public AST_Node {
      public:
        Quoted_String_AST_Node(const std::string &t_ast_node_text = "", int t_id = AST_Node_Type::Quoted_String, const boost::shared_ptr<std::string> &t_fname=boost::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          AST_Node(t_ast_node_text, t_id, t_fname, t_start_line, t_start_col, t_end_line, t_end_col),
          m_value(const_var(this->text)) { }
        virtual ~Quoted_String_AST_Node() {}
        virtual Boxed_Value eval(chaiscript::detail::Dispatch_Engine &) {
          return m_value;
        }

      private:
        Boxed_Value m_value;

    };

    struct Single_Quoted_String_AST_Node : public AST_Node {
      public:
        Single_Quoted_String_AST_Node(const std::string &t_ast_node_text = "", int t_id = AST_Node_Type::Single_Quoted_String, const boost::shared_ptr<std::string> &t_fname=boost::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          AST_Node(t_ast_node_text, t_id, t_fname, t_start_line, t_start_col, t_end_line, t_end_col),
          m_value(const_var(char(this->text[0]))) { }
        virtual ~Single_Quoted_String_AST_Node() {}
        virtual Boxed_Value eval(chaiscript::detail::Dispatch_Engine &){
          return m_value;
        }

      private:
        Boxed_Value m_value;
    };

    struct Lambda_AST_Node : public AST_Node {
      public:
        Lambda_AST_Node(const std::string &t_ast_node_text = "", int t_id = AST_Node_Type::Lambda, const boost::shared_ptr<std::string> &t_fname=boost::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          AST_Node(t_ast_node_text, t_id, t_fname, t_start_line, t_start_col, t_end_line, t_end_col) { }
        virtual ~Lambda_AST_Node() {}
        virtual Boxed_Value eval(chaiscript::detail::Dispatch_Engine &t_ss){
          std::vector<std::string> t_param_names;
          size_t numparams = 0;

          if ((this->children.size() > 0) && (this->children[0]->identifier == AST_Node_Type::Arg_List)) {
            numparams = this->children[0]->children.size();
            for (size_t i = 0; i < numparams; ++i) {
              t_param_names.push_back(this->children[0]->children[i]->text);
            }

          }
          else {
            //no parameters
            numparams = 0;
          }

          return Boxed_Value(Proxy_Function(new dispatch::Dynamic_Proxy_Function
                (boost::bind(&eval_function<chaiscript::detail::Dispatch_Engine>, boost::ref(t_ss), this->children.back(), t_param_names, _1),
                 static_cast<int>(numparams), this->children.back())));
        }

    };

    struct Block_AST_Node : public AST_Node {
      public:
        Block_AST_Node(const std::string &t_ast_node_text = "", int t_id = AST_Node_Type::Block, const boost::shared_ptr<std::string> &t_fname=boost::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          AST_Node(t_ast_node_text, t_id, t_fname, t_start_line, t_start_col, t_end_line, t_end_col) { }
        virtual ~Block_AST_Node() {}
        virtual Boxed_Value eval(chaiscript::detail::Dispatch_Engine &t_ss){
          size_t num_children = this->children.size();

          t_ss.new_scope();
          for (size_t i = 0; i < num_children; ++i) {
            try {
              const Boxed_Value &retval = this->children[i]->eval(t_ss);

              if (i + 1 == num_children)
              {
                t_ss.pop_scope();
                return retval;
              }
            }
            catch (const chaiscript::detail::Return_Value &) {
              t_ss.pop_scope();
              throw;
            }
            catch (exception::eval_error &ee) {
              ee.call_stack.push_back(this->children[i]);
              t_ss.pop_scope();
              throw;
            }
            catch (...) {
              t_ss.pop_scope();
              throw;
            }
          }

          t_ss.pop_scope();
          return Boxed_Value();
        }

    };

    struct Def_AST_Node : public AST_Node {
      public:
        Def_AST_Node(const std::string &t_ast_node_text = "", int t_id = AST_Node_Type::Def, const boost::shared_ptr<std::string> &t_fname=boost::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          AST_Node(t_ast_node_text, t_id, t_fname, t_start_line, t_start_col, t_end_line, t_end_col) { }
        virtual ~Def_AST_Node() {}
        virtual Boxed_Value eval(chaiscript::detail::Dispatch_Engine &t_ss){
          std::vector<std::string> t_param_names;
          size_t numparams = 0;
          AST_NodePtr guardnode;

          if ((this->children.size() > 2) && (this->children[1]->identifier == AST_Node_Type::Arg_List)) {
            numparams = this->children[1]->children.size();
            for (size_t i = 0; i < numparams; ++i) {
              t_param_names.push_back(this->children[1]->children[i]->text);
            }

            if (this->children.size() > 3) {
              guardnode = this->children[2];
            }
          }
          else {
            //no parameters
            numparams = 0;

            if (this->children.size() > 2) {
              guardnode = this->children[1];
            }
          }

          boost::shared_ptr<dispatch::Dynamic_Proxy_Function> guard;
          if (guardnode) {
            guard = boost::shared_ptr<dispatch::Dynamic_Proxy_Function>
              (new dispatch::Dynamic_Proxy_Function(boost::bind(&eval_function<chaiscript::detail::Dispatch_Engine>,
                                                      boost::ref(t_ss), guardnode,
                                                      t_param_names, _1), static_cast<int>(numparams), guardnode));
          }

          try {
            const std::string & l_function_name = this->children[0]->text;
            const std::string & l_annotation = this->annotation?this->annotation->text:"";
            t_ss.add(Proxy_Function
                (new dispatch::Dynamic_Proxy_Function(boost::bind(&eval_function<chaiscript::detail::Dispatch_Engine>,
                                                        boost::ref(t_ss), this->children.back(),
                                                        t_param_names, _1), static_cast<int>(numparams), this->children.back(),
                                            l_annotation, guard)), l_function_name);
          }
          catch (const exception::reserved_word_error &e) {
            throw exception::eval_error("Reserved word used as function name '" + e.word() + "'");
          }
          return Boxed_Value();
        }

    };

    struct While_AST_Node : public AST_Node {
      public:
        While_AST_Node(const std::string &t_ast_node_text = "", int t_id = AST_Node_Type::While, const boost::shared_ptr<std::string> &t_fname=boost::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          AST_Node(t_ast_node_text, t_id, t_fname, t_start_line, t_start_col, t_end_line, t_end_col) { }
        virtual ~While_AST_Node() {}
        virtual Boxed_Value eval(chaiscript::detail::Dispatch_Engine &t_ss) {
          bool cond;

          t_ss.new_scope();

          try {
            cond = boxed_cast<bool>(this->children[0]->eval(t_ss));
          }
          catch (const exception::bad_boxed_cast &) {
            t_ss.pop_scope();
            throw exception::eval_error("While condition not boolean");
          }
          catch (exception::eval_error &ee) {
            ee.call_stack.push_back(this->children[0]);
            t_ss.pop_scope();
            throw;
          }
          while (cond) {
            try {
              try {
                this->children[1]->eval(t_ss);
              }
              catch (exception::eval_error &ee) {
                ee.call_stack.push_back(this->children[1]);
                throw;
              }

              try {
                cond = boxed_cast<bool>(this->children[0]->eval(t_ss));
              }
              catch (const exception::bad_boxed_cast &) {
                t_ss.pop_scope();
                throw exception::eval_error("While condition not boolean");
              }
              catch (exception::eval_error &ee) {
                ee.call_stack.push_back(this->children[0]);
                t_ss.pop_scope();
                throw;
              }
            }
            catch (detail::Break_Loop &) {
              cond = false;
            }
          }
          t_ss.pop_scope();
          return Boxed_Value();
        }
        ;
    };

    struct If_AST_Node : public AST_Node {
      public:
        If_AST_Node(const std::string &t_ast_node_text = "", int t_id = AST_Node_Type::If, const boost::shared_ptr<std::string> &t_fname=boost::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          AST_Node(t_ast_node_text, t_id, t_fname, t_start_line, t_start_col, t_end_line, t_end_col) { }
        virtual ~If_AST_Node() {}
        virtual Boxed_Value eval(chaiscript::detail::Dispatch_Engine &t_ss){
          bool cond;
          try {
            cond = boxed_cast<bool>(this->children[0]->eval(t_ss));
          }
          catch (const exception::bad_boxed_cast &) {
            throw exception::eval_error("If condition not boolean");
          }
          catch (exception::eval_error &ee) {
            ee.call_stack.push_back(this->children[0]);
            throw;
          }

          if (cond) {
            try {
              return this->children[1]->eval(t_ss);
            }
            catch (exception::eval_error &ee) {
              ee.call_stack.push_back(this->children[1]);
              throw;
            }
          }
          else {
            if (this->children.size() > 2) {
              size_t i = 2;
              while ((!cond) && (i < this->children.size())) {
                if (this->children[i]->text == "else") {
                  try {
                    return this->children[i+1]->eval(t_ss);
                  }
                  catch (exception::eval_error &ee) {
                    ee.call_stack.push_back(this->children[i+1]);
                    throw;
                  }
                }
                else if (this->children[i]->text == "else if") {
                  try {
                    cond = boxed_cast<bool>(this->children[i+1]->eval(t_ss));
                  }
                  catch (const exception::bad_boxed_cast &) {
                    throw exception::eval_error("'else if' condition not boolean");
                  }
                  catch (exception::eval_error &ee) {
                    ee.call_stack.push_back(this->children[i+1]);
                    throw;
                  }
                  if (cond) {
                    try {
                      return this->children[i+2]->eval(t_ss);
                    }
                    catch (exception::eval_error &ee) {
                      ee.call_stack.push_back(this->children[i+2]);
                      throw;
                    }
                  }
                }
                i = i + 3;
              }
            }
          }

          return Boxed_Value(false);
        }

    };

    struct For_AST_Node : public AST_Node {
      public:
        For_AST_Node(const std::string &t_ast_node_text = "", int t_id = AST_Node_Type::For, const boost::shared_ptr<std::string> &t_fname=boost::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          AST_Node(t_ast_node_text, t_id, t_fname, t_start_line, t_start_col, t_end_line, t_end_col) { }
        virtual ~For_AST_Node() {}
        virtual Boxed_Value eval(chaiscript::detail::Dispatch_Engine &t_ss){
          bool cond;

          t_ss.new_scope();

          try {
            if (this->children.size() == 4) {
              try {
                this->children[0]->eval(t_ss);
              }
              catch (exception::eval_error &ee) {
                ee.call_stack.push_back(this->children[0]);
                throw;
              }

              try {
                cond = boxed_cast<bool>(this->children[1]->eval(t_ss));
              }
              catch (exception::eval_error &ee) {
                ee.call_stack.push_back(this->children[1]);
                throw;
              }
            }
            else {
              try {
                cond = boxed_cast<bool>(this->children[0]->eval(t_ss));
              }
              catch (exception::eval_error &ee) {
                ee.call_stack.push_back(this->children[0]);
                t_ss.pop_scope();
                throw;
              }
            }
          }
          catch (const exception::bad_boxed_cast &) {
            t_ss.pop_scope();
            throw exception::eval_error("For condition not boolean");
          }
          while (cond) {
            try {
              if (this->children.size() == 4) {
                try {
                  this->children[3]->eval(t_ss);
                }
                catch (exception::eval_error &ee) {
                  ee.call_stack.push_back(this->children[3]);
                  throw;
                }

                try {
                  this->children[2]->eval(t_ss);
                }
                catch (exception::eval_error &ee) {
                  ee.call_stack.push_back(this->children[2]);
                  throw;
                }

                try {
                  cond = boxed_cast<bool>(this->children[1]->eval(t_ss));
                }
                catch (exception::eval_error &ee) {
                  ee.call_stack.push_back(this->children[1]);
                  throw;
                }
              }
              else {
                try {
                  this->children[2]->eval(t_ss);
                }
                catch (exception::eval_error &ee) {
                  ee.call_stack.push_back(this->children[2]);
                  throw;
                }

                try {
                  this->children[1]->eval(t_ss);
                }
                catch (exception::eval_error &ee) {
                  ee.call_stack.push_back(this->children[1]);
                  throw;
                }

                try {
                  cond = boxed_cast<bool>(this->children[0]->eval(t_ss));
                }
                catch (exception::eval_error &ee) {
                  ee.call_stack.push_back(this->children[0]);
                  t_ss.pop_scope();
                  throw;
                }
              }
            }
            catch (const exception::bad_boxed_cast &) {
              t_ss.pop_scope();
              throw exception::eval_error("For condition not boolean");
            }
            catch (detail::Break_Loop &) {
              cond = false;
            }
          }
          t_ss.pop_scope();
          return Boxed_Value();
        }

    };

    struct Inline_Array_AST_Node : public AST_Node {
      public:
        Inline_Array_AST_Node(const std::string &t_ast_node_text = "", int t_id = AST_Node_Type::Inline_Array, const boost::shared_ptr<std::string> &t_fname=boost::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          AST_Node(t_ast_node_text, t_id, t_fname, t_start_line, t_start_col, t_end_line, t_end_col) { }
        virtual ~Inline_Array_AST_Node() {}
        virtual Boxed_Value eval(chaiscript::detail::Dispatch_Engine &t_ss){
          std::vector<Boxed_Value> vec;
          if (this->children.size() > 0) {
            for (size_t i = 0; i < this->children[0]->children.size(); ++i) {
              try {
                vec.push_back(this->children[0]->children[i]->eval(t_ss));
              }
              catch(exception::eval_error &ee) {
                ee.call_stack.push_back(this->children[0]->children[i]);
                throw;
              }
            }
          }

          return const_var(vec);
        }

    };

    struct Inline_Map_AST_Node : public AST_Node {
      public:
        Inline_Map_AST_Node(const std::string &t_ast_node_text = "", int t_id = AST_Node_Type::Inline_Map, const boost::shared_ptr<std::string> &t_fname=boost::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          AST_Node(t_ast_node_text, t_id, t_fname, t_start_line, t_start_col, t_end_line, t_end_col) { }
        virtual ~Inline_Map_AST_Node() {}
        virtual Boxed_Value eval(chaiscript::detail::Dispatch_Engine &t_ss){
          try {
            std::map<std::string, Boxed_Value> retval;
            for (size_t i = 0; i < this->children[0]->children.size(); ++i) {
              try {
                retval[boxed_cast<std::string>(this->children[0]->children[i]->children[0]->eval(t_ss))] 
                  = t_ss.call_function("clone", this->children[0]->children[i]->children[1]->eval(t_ss));
              }
              catch(exception::eval_error &ee) {
                ee.call_stack.push_back(this->children[0]->children[i]);
                throw;
              }
            }
            return const_var(retval);
          }
          catch (const exception::dispatch_error &) {
            throw exception::eval_error("Can not find appropriate 'Map()'");
          }
        }

    };

    struct Return_AST_Node : public AST_Node {
      public:
        Return_AST_Node(const std::string &t_ast_node_text = "", int t_id = AST_Node_Type::Return, const boost::shared_ptr<std::string> &t_fname=boost::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          AST_Node(t_ast_node_text, t_id, t_fname, t_start_line, t_start_col, t_end_line, t_end_col) { }
        virtual ~Return_AST_Node() {}
        virtual Boxed_Value eval(chaiscript::detail::Dispatch_Engine &t_ss){
          if (this->children.size() > 0) {
            try {
              throw detail::Return_Value(this->children[0]->eval(t_ss));
            }
            catch (exception::eval_error &ee) {
              ee.call_stack.push_back(this->children[0]);
              throw;
            }
          }
          else {
            throw detail::Return_Value(Boxed_Value());
          }
        }

    };

    struct File_AST_Node : public AST_Node {
      public:
        File_AST_Node(const std::string &t_ast_node_text = "", int t_id = AST_Node_Type::File, const boost::shared_ptr<std::string> &t_fname=boost::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          AST_Node(t_ast_node_text, t_id, t_fname, t_start_line, t_start_col, t_end_line, t_end_col) { }
        virtual ~File_AST_Node() {}
        virtual Boxed_Value eval(chaiscript::detail::Dispatch_Engine &t_ss) {
          const size_t size = this->children.size(); 
          for (size_t i = 0; i < size; ++i) {
            try {
              const Boxed_Value &retval = this->children[i]->eval(t_ss);
              if (i + 1 == size) {
                return retval;
              }
            }
            catch (exception::eval_error &ee) {
              ee.call_stack.push_back(this->children[i]);
              throw;
            }
          }
          return Boxed_Value();
        }
    };

    struct Prefix_AST_Node : public AST_Node {
      public:
        Prefix_AST_Node(const std::string &t_ast_node_text = "", int t_id = AST_Node_Type::Prefix, const boost::shared_ptr<std::string> &t_fname=boost::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          AST_Node(t_ast_node_text, t_id, t_fname, t_start_line, t_start_col, t_end_line, t_end_col) { }
        virtual ~Prefix_AST_Node() {}
        virtual Boxed_Value eval(chaiscript::detail::Dispatch_Engine &t_ss){
          try {
            return t_ss.call_function(this->children[0]->text, this->children[1]->eval(t_ss));
          }
          catch(std::exception &){
            throw exception::eval_error("Can not find appropriate unary '" + this->children[0]->text + "'");
          }
        }

    };

    struct Break_AST_Node : public AST_Node {
      public:
        Break_AST_Node(const std::string &t_ast_node_text = "", int t_id = AST_Node_Type::Break, const boost::shared_ptr<std::string> &t_fname=boost::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          AST_Node(t_ast_node_text, t_id, t_fname, t_start_line, t_start_col, t_end_line, t_end_col) { }
        virtual ~Break_AST_Node() {}
        virtual Boxed_Value eval(chaiscript::detail::Dispatch_Engine &){
          throw detail::Break_Loop();
        }
    };

    struct Map_Pair_AST_Node : public AST_Node {
      public:
        Map_Pair_AST_Node(const std::string &t_ast_node_text = "", int t_id = AST_Node_Type::Map_Pair, const boost::shared_ptr<std::string> &t_fname=boost::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          AST_Node(t_ast_node_text, t_id, t_fname, t_start_line, t_start_col, t_end_line, t_end_col) { }
        virtual ~Map_Pair_AST_Node() {}
    };

    struct Value_Range_AST_Node : public AST_Node {
      public:
        Value_Range_AST_Node(const std::string &t_ast_node_text = "", int t_id = AST_Node_Type::Value_Range, const boost::shared_ptr<std::string> &t_fname=boost::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          AST_Node(t_ast_node_text, t_id, t_fname, t_start_line, t_start_col, t_end_line, t_end_col) { }
        virtual ~Value_Range_AST_Node() {}
    };

    struct Inline_Range_AST_Node : public AST_Node {
      public:
        Inline_Range_AST_Node(const std::string &t_ast_node_text = "", int t_id = AST_Node_Type::Inline_Range, const boost::shared_ptr<std::string> &t_fname=boost::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          AST_Node(t_ast_node_text, t_id, t_fname, t_start_line, t_start_col, t_end_line, t_end_col) { }
        virtual ~Inline_Range_AST_Node() {}
        virtual Boxed_Value eval(chaiscript::detail::Dispatch_Engine &t_ss){
          try {
            return t_ss.call_function("generate_range",
                this->children[0]->children[0]->children[0]->eval(t_ss),
                this->children[0]->children[0]->children[1]->eval(t_ss));
          }
          catch (const exception::dispatch_error &) {
            throw exception::eval_error("Unable to generate range vector");
          }
          catch(exception::eval_error &ee) {
            ee.call_stack.push_back(this->children[0]->children[0]);
            throw;
          }
        }

    };

    struct Annotation_AST_Node : public AST_Node {
      public:
        Annotation_AST_Node(const std::string &t_ast_node_text = "", int t_id = AST_Node_Type::Annotation, const boost::shared_ptr<std::string> &t_fname=boost::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          AST_Node(t_ast_node_text, t_id, t_fname, t_start_line, t_start_col, t_end_line, t_end_col) { }
        virtual ~Annotation_AST_Node() {}
    };

    struct Try_AST_Node : public AST_Node {
      public:
        Try_AST_Node(const std::string &t_ast_node_text = "", int t_id = AST_Node_Type::Try, const boost::shared_ptr<std::string> &t_fname=boost::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          AST_Node(t_ast_node_text, t_id, t_fname, t_start_line, t_start_col, t_end_line, t_end_col) { }
        virtual ~Try_AST_Node() {}
        virtual Boxed_Value eval(chaiscript::detail::Dispatch_Engine &t_ss){
          Boxed_Value retval;        

          t_ss.new_scope();
          try {
            retval = this->children[0]->eval(t_ss);
          }
          catch (exception::eval_error &ee) {
            ee.call_stack.push_back(this->children[0]);
            if (this->children.back()->identifier == AST_Node_Type::Finally) {
              try {
                this->children.back()->children[0]->eval(t_ss);
              }
              catch (exception::eval_error &ee2) {
                ee2.call_stack.push_back(this->children.back()->children[0]);
                t_ss.pop_scope();
                throw;
              }
            }
            t_ss.pop_scope();
            throw;
          }
          catch (const std::exception &e) {
            Boxed_Value except = Boxed_Value(boost::ref(e));

            size_t end_point = this->children.size();
            if (this->children.back()->identifier == AST_Node_Type::Finally) {
              assert(end_point > 0);
              end_point = this->children.size() - 1;
            }
            for (unsigned int i = 1; i < end_point; ++i) {
              AST_NodePtr catch_block = this->children[i];

              if (catch_block->children.size() == 1) {
                //No variable capture, no guards
                try {
                  retval = catch_block->children[0]->eval(t_ss);
                }
                catch (exception::eval_error &ee) {
                  ee.call_stack.push_back(catch_block->children[0]);
                  throw;
                }
                break;
              }
              else if (catch_block->children.size() == 2) {
                //Variable capture, no guards
                t_ss.add_object(catch_block->children[0]->text, except);
                try {
                  retval = catch_block->children[1]->eval(t_ss);
                }
                catch (exception::eval_error &ee) {
                  ee.call_stack.push_back(catch_block->children[1]);
                  throw;
                }

                break;
              }
              else if (catch_block->children.size() == 3) {
                //Variable capture, no guards
                t_ss.add_object(catch_block->children[0]->text, except);

                bool guard;
                try {
                  guard = boxed_cast<bool>(catch_block->children[1]->eval(t_ss));
                } catch (const exception::bad_boxed_cast &) {
                  if (this->children.back()->identifier == AST_Node_Type::Finally) {
                    try {
                      this->children.back()->children[0]->eval(t_ss);
                    }
                    catch (exception::eval_error &ee) {
                      ee.call_stack.push_back(this->children.back()->children[0]);
                      t_ss.pop_scope();
                      throw;
                    }
                  }
                  t_ss.pop_scope();
                  throw exception::eval_error("Guard condition not boolean");
                }
                if (guard) {
                  try {
                    retval = catch_block->children[2]->eval(t_ss);
                  }
                  catch (exception::eval_error &ee) {
                    ee.call_stack.push_back(catch_block->children[2]);
                    throw;
                  }

                  break;
                }
              }
              else {
                if (this->children.back()->identifier == AST_Node_Type::Finally) {
                  try {
                    this->children.back()->children[0]->eval(t_ss);
                  }
                  catch (exception::eval_error &ee) {
                    ee.call_stack.push_back(this->children.back()->children[0]);
                    t_ss.pop_scope();
                    throw;
                  }
                }
                t_ss.pop_scope();
                throw exception::eval_error("Internal error: catch block size unrecognized");
              }
            }
          }
          catch (Boxed_Value &bv) {
            Boxed_Value except = bv;
            for (size_t i = 1; i < this->children.size(); ++i) {
              AST_NodePtr catch_block = this->children[i];

              if (catch_block->children.size() == 1) {
                //No variable capture, no guards
                try {
                  retval = catch_block->children[0]->eval(t_ss);
                }
                catch (exception::eval_error &ee) {
                  ee.call_stack.push_back(catch_block->children[0]);
                  throw;
                }

                break;
              }
              else if (catch_block->children.size() == 2) {
                //Variable capture, no guards
                t_ss.add_object(catch_block->children[0]->text, except);
                try {
                  retval = catch_block->children[1]->eval(t_ss);
                }
                catch (exception::eval_error &ee) {
                  ee.call_stack.push_back(catch_block->children[1]);
                  throw;
                }

                break;
              }
              else if (catch_block->children.size() == 3) {
                //Variable capture, no guards
                t_ss.add_object(catch_block->children[0]->text, except);

                bool guard;
                try {
                  guard = boxed_cast<bool>(catch_block->children[1]->eval(t_ss));
                }
                catch (const exception::bad_boxed_cast &) {
                  if (this->children.back()->identifier == AST_Node_Type::Finally) {
                    try {
                      this->children.back()->children[0]->eval(t_ss);
                    }
                    catch (exception::eval_error &ee) {
                      ee.call_stack.push_back(this->children.back()->children[0]);
                      t_ss.pop_scope();
                      throw;
                    }
                  }

                  t_ss.pop_scope();
                  throw exception::eval_error("Guard condition not boolean");
                }
                catch (exception::eval_error &ee) {
                  ee.call_stack.push_back(catch_block->children[1]);
                  throw;
                }
                if (guard) {
                  try {
                    retval = catch_block->children[2]->eval(t_ss);
                  }
                  catch (exception::eval_error &ee) {
                    ee.call_stack.push_back(catch_block->children[2]);
                    throw;
                  }
                  break;
                }
              }
              else {
                if (this->children.back()->identifier == AST_Node_Type::Finally) {
                  try {
                    this->children.back()->children[0]->eval(t_ss);
                  }
                  catch (exception::eval_error &ee) {
                    ee.call_stack.push_back(this->children.back()->children[0]);
                    t_ss.pop_scope();
                    throw;
                  }
                }
                t_ss.pop_scope();
                throw exception::eval_error("Internal error: catch block size unrecognized");
              }
            }
          }
          catch (...) {
            if (this->children.back()->identifier == AST_Node_Type::Finally) {
              try {
                this->children.back()->children[0]->eval(t_ss);
              }
              catch (exception::eval_error &ee) {
                ee.call_stack.push_back(this->children.back()->children[0]);
                t_ss.pop_scope();
                throw;
              }
            }
            t_ss.pop_scope();
            throw;
          }

          if (this->children.back()->identifier == AST_Node_Type::Finally) {
            try {
              retval = this->children.back()->children[0]->eval(t_ss);
            }
            catch (exception::eval_error &ee) {
              ee.call_stack.push_back(this->children.back()->children[0]);
              t_ss.pop_scope();
              throw;
            }
          }

          t_ss.pop_scope();

          return retval;
        }

    };

    struct Catch_AST_Node : public AST_Node {
      public:
        Catch_AST_Node(const std::string &t_ast_node_text = "", int t_id = AST_Node_Type::Catch, const boost::shared_ptr<std::string> &t_fname=boost::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          AST_Node(t_ast_node_text, t_id, t_fname, t_start_line, t_start_col, t_end_line, t_end_col) { }
        virtual ~Catch_AST_Node() {}
    };

    struct Finally_AST_Node : public AST_Node {
      public:
        Finally_AST_Node(const std::string &t_ast_node_text = "", int t_id = AST_Node_Type::Finally, const boost::shared_ptr<std::string> &t_fname=boost::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          AST_Node(t_ast_node_text, t_id, t_fname, t_start_line, t_start_col, t_end_line, t_end_col) { }
        virtual ~Finally_AST_Node() {}
    };

    struct Method_AST_Node : public AST_Node {
      public:
        Method_AST_Node(const std::string &t_ast_node_text = "", int t_id = AST_Node_Type::Method, const boost::shared_ptr<std::string> &t_fname=boost::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          AST_Node(t_ast_node_text, t_id, t_fname, t_start_line, t_start_col, t_end_line, t_end_col) { }
        virtual ~Method_AST_Node() {}
        virtual Boxed_Value eval(chaiscript::detail::Dispatch_Engine &t_ss){

          std::vector<std::string> t_param_names;
          AST_NodePtr guardnode;

          //The first param of a method is always the implied this ptr.
          t_param_names.push_back("this");

          if ((this->children.size() > 3) && (this->children[2]->identifier == AST_Node_Type::Arg_List)) {
            for (size_t i = 0; i < this->children[2]->children.size(); ++i) {
              t_param_names.push_back(this->children[2]->children[i]->text);
            }

            if (this->children.size() > 4) {
              guardnode = this->children[3];
            }
          }
          else {
            //no parameters

            if (this->children.size() > 3) {
              guardnode = this->children[2];
            }
          }

          size_t numparams = t_param_names.size();

          boost::shared_ptr<dispatch::Dynamic_Proxy_Function> guard;
          if (guardnode) {
            guard = boost::shared_ptr<dispatch::Dynamic_Proxy_Function>
              (new dispatch::Dynamic_Proxy_Function(boost::bind(&eval_function<chaiscript::detail::Dispatch_Engine>,
                                                      boost::ref(t_ss), guardnode,
                                                      t_param_names, _1), static_cast<int>(numparams), guardnode));
          }

          try {
            const std::string & l_annotation = this->annotation?this->annotation->text:"";
            const std::string & class_name = this->children[0]->text;
            const std::string & function_name = this->children[1]->text;
            if (function_name == class_name) {
              t_ss.add(Proxy_Function
                  (new dispatch::detail::Dynamic_Object_Constructor(class_name, Proxy_Function
                                                          (new dispatch::Dynamic_Proxy_Function(boost::bind(&eval_function<chaiscript::detail::Dispatch_Engine>,
                                                                                                  boost::ref(t_ss), this->children.back(),
                                                                                                  t_param_names, _1), static_cast<int>(numparams), this->children.back(),
                                                                                      l_annotation, guard)))), function_name);

            }
            else {
              boost::optional<chaiscript::Type_Info> ti;
              try {
                ti = t_ss.get_type(class_name);
              } catch (const std::range_error &) {
                // No biggie, the type name is just not known
              }
              t_ss.add(Proxy_Function
                  (new dispatch::detail::Dynamic_Object_Function(class_name, Proxy_Function
                                                       (new dispatch::Dynamic_Proxy_Function(boost::bind(&eval_function<chaiscript::detail::Dispatch_Engine>,
                                                                                               boost::ref(t_ss), this->children.back(),
                                                                                               t_param_names, _1), static_cast<int>(numparams), this->children.back(),
                                                                                   l_annotation, guard)), ti)), function_name);

            }
          }
          catch (const exception::reserved_word_error &e) {
            throw exception::eval_error("Reserved word used as method name '" + e.word() + "'");
          }
          return Boxed_Value();
        }

    };

    struct Attr_Decl_AST_Node : public AST_Node {
      public:
        Attr_Decl_AST_Node(const std::string &t_ast_node_text = "", int t_id = AST_Node_Type::Attr_Decl, const boost::shared_ptr<std::string> &t_fname=boost::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          AST_Node(t_ast_node_text, t_id, t_fname, t_start_line, t_start_col, t_end_line, t_end_col) { }
        virtual ~Attr_Decl_AST_Node() {}
        virtual Boxed_Value eval(chaiscript::detail::Dispatch_Engine &t_ss){
          try {
            t_ss.add(fun(boost::function<Boxed_Value (dispatch::Dynamic_Object &)>(boost::bind(&dispatch::detail::Dynamic_Object_Attribute::func, this->children[0]->text,
                      this->children[1]->text, _1))), this->children[1]->text);

          }
          catch (const exception::reserved_word_error &) {
            throw exception::eval_error("Reserved word used as attribute '" + this->children[1]->text + "'");
          }
          return Boxed_Value();
        }

    };

    struct Shift_AST_Node : public Binary_Operator_AST_Node {
      public:
        Shift_AST_Node(const std::string &t_ast_node_text = "", int t_id = AST_Node_Type::Shift, const boost::shared_ptr<std::string> &t_fname=boost::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          Binary_Operator_AST_Node(t_ast_node_text, t_id, t_fname, t_start_line, t_start_col, t_end_line, t_end_col) { }
        virtual ~Shift_AST_Node() {}
    };

    struct Equality_AST_Node : public Binary_Operator_AST_Node {
      public:
        Equality_AST_Node(const std::string &t_ast_node_text = "", int t_id = AST_Node_Type::Equality, const boost::shared_ptr<std::string> &t_fname=boost::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          Binary_Operator_AST_Node(t_ast_node_text, t_id, t_fname, t_start_line, t_start_col, t_end_line, t_end_col) { }
        virtual ~Equality_AST_Node() {}
    };

    struct Bitwise_And_AST_Node : public Binary_Operator_AST_Node {
      public:
        Bitwise_And_AST_Node(const std::string &t_ast_node_text = "", int t_id = AST_Node_Type::Bitwise_And, const boost::shared_ptr<std::string> &t_fname=boost::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          Binary_Operator_AST_Node(t_ast_node_text, t_id, t_fname, t_start_line, t_start_col, t_end_line, t_end_col) { }
        virtual ~Bitwise_And_AST_Node() {}
    };

    struct Bitwise_Xor_AST_Node : public Binary_Operator_AST_Node {
      public:
        Bitwise_Xor_AST_Node(const std::string &t_ast_node_text = "", int t_id = AST_Node_Type::Bitwise_Xor, const boost::shared_ptr<std::string> &t_fname=boost::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          Binary_Operator_AST_Node(t_ast_node_text, t_id, t_fname, t_start_line, t_start_col, t_end_line, t_end_col) { }
        virtual ~Bitwise_Xor_AST_Node() {}
    };

    struct Bitwise_Or_AST_Node : public Binary_Operator_AST_Node {
      public:
        Bitwise_Or_AST_Node(const std::string &t_ast_node_text = "", int t_id = AST_Node_Type::Bitwise_Or, const boost::shared_ptr<std::string> &t_fname=boost::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          Binary_Operator_AST_Node(t_ast_node_text, t_id, t_fname, t_start_line, t_start_col, t_end_line, t_end_col) { }
        virtual ~Bitwise_Or_AST_Node() {}
    };

    struct Logical_And_AST_Node : public AST_Node {
      public:
        Logical_And_AST_Node(const std::string &t_ast_node_text = "", int t_id = AST_Node_Type::Logical_And, const boost::shared_ptr<std::string> &t_fname=boost::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          AST_Node(t_ast_node_text, t_id, t_fname, t_start_line, t_start_col, t_end_line, t_end_col) { }
        virtual ~Logical_And_AST_Node() {}
        virtual Boxed_Value eval(chaiscript::detail::Dispatch_Engine &t_ss){
          Boxed_Value retval;
          try {
            retval = this->children[0]->eval(t_ss);
          }
          catch (exception::eval_error &ee) {
            ee.call_stack.push_back(this->children[0]);
            throw;
          }

          if (this->children.size() > 1) {
            for (size_t i = 1; i < this->children.size(); i += 2) {
              bool lhs;
              try {
                lhs = boxed_cast<bool>(retval);
              }
              catch (const exception::bad_boxed_cast &) {
                throw exception::eval_error("Condition not boolean");
              }
              if (lhs) {
                try {
                  retval = this->children[i+1]->eval(t_ss);
                }
                catch (exception::eval_error &ee) {
                  ee.call_stack.push_back(this->children[i+1]);
                  throw;
                }
              }
              else {
                retval = Boxed_Value(false);
              }
            }
          }
          return retval;
        }
    };

    struct Logical_Or_AST_Node : public AST_Node {
      public:
        Logical_Or_AST_Node(const std::string &t_ast_node_text = "", int t_id = AST_Node_Type::Logical_Or, const boost::shared_ptr<std::string> &t_fname=boost::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          AST_Node(t_ast_node_text, t_id, t_fname, t_start_line, t_start_col, t_end_line, t_end_col) { }
        virtual ~Logical_Or_AST_Node() {}
        virtual Boxed_Value eval(chaiscript::detail::Dispatch_Engine &t_ss){
          Boxed_Value retval;

          try {
            retval = this->children[0]->eval(t_ss);
          }
          catch (exception::eval_error &ee) {
            ee.call_stack.push_back(this->children[0]);
            throw;
          }

          if (this->children.size() > 1) {
            for (size_t i = 1; i < this->children.size(); i += 2) {
              bool lhs;
              try {
                lhs = boxed_cast<bool>(retval);
              }
              catch (const exception::bad_boxed_cast &) {
                throw exception::eval_error("Condition not boolean");
              }
              if (lhs) {
                retval = Boxed_Value(true);
              }
              else {
                try {
                  retval = this->children[i+1]->eval(t_ss);
                }
                catch (exception::eval_error &ee) {
                  ee.call_stack.push_back(this->children[i+1]);
                  throw;
                }
              }
            }
          }
          return retval;
        }

    };
  }


}
#endif /* CHAISCRIPT_EVAL_HPP_ */
