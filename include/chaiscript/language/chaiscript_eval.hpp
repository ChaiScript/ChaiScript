// This file is distributed under the BSD License.
// See "license.txt" for details.
// Copyright 2009-2012, Jonathan Turner (jonathan@emptycrate.com)
// Copyright 2009-2014, Jason Turner (jason@emptycrate.com)
// http://www.chaiscript.com

#ifndef CHAISCRIPT_EVAL_HPP_
#define CHAISCRIPT_EVAL_HPP_

#include <map>

#include <chaiscript/language/chaiscript_common.hpp>

namespace chaiscript
{
  /// \brief Classes and functions that are part of the runtime eval system
  namespace eval
  {
    namespace detail
    {
      /**
       * Helper function that will set up the scope around a function call, including handling the named function parameters
       */
      static const Boxed_Value eval_function(chaiscript::detail::Dispatch_Engine &t_ss, const AST_NodePtr &t_node, const std::vector<std::string> &t_param_names, const std::vector<Boxed_Value> &t_vals) {
        chaiscript::eval::detail::Scope_Push_Pop spp(t_ss);

        for (unsigned int i = 0; i < t_param_names.size(); ++i) {
          t_ss.add_object(t_param_names[i], t_vals[i]);
        }

        try {
          return t_node->eval(t_ss);
        } catch (const detail::Return_Value &rv) {
          return rv.retval;
        } 
      }
    }

    struct Binary_Operator_AST_Node : public AST_Node {
      public:
        Binary_Operator_AST_Node(const std::string &t_ast_node_text, int t_id, const boost::shared_ptr<std::string> &t_fname=boost::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          AST_Node(t_ast_node_text, t_id, t_fname, t_start_line, t_start_col, t_end_line, t_end_col)
      { }

        virtual ~Binary_Operator_AST_Node() {}
        virtual Boxed_Value eval_internal(chaiscript::detail::Dispatch_Engine &t_ss) {
          return do_oper(t_ss, Operators::to_operator(children[1]->text), children[1]->text,
              this->children[0]->eval(t_ss),
              this->children[2]->eval(t_ss));
        }

        virtual std::string pretty_print() const 
        {
          if (children.size() == 3)
          {
            return "(" + this->children[0]->pretty_print() + " " + this->children[1]->text + " " + this->children[2]->pretty_print() + ")";
          } else {
            return "(" + this->children[0]->pretty_print() + " " + text + " " + this->children[1]->pretty_print() + ")";
          }
        }

      protected:
        Boxed_Value do_oper(chaiscript::detail::Dispatch_Engine &t_ss, 
            Operators::Opers t_oper, const std::string &t_oper_string, const Boxed_Value &t_lhs, const Boxed_Value &t_rhs)
        {
          try {
            chaiscript::eval::detail::Function_Push_Pop fpp(t_ss);
            std::vector<Boxed_Value> params(2);
            params.push_back(t_lhs);
            params.push_back(t_rhs);
            fpp.save_params(params);
           
            if (t_oper != Operators::invalid && t_lhs.get_type_info().is_arithmetic() && t_rhs.get_type_info().is_arithmetic())
            {
              // If it's an arithmetic operation we want to short circuit dispatch
              try{
                return Boxed_Number::do_oper(t_oper, t_lhs, t_rhs);
              } catch (...) {
                throw exception::eval_error("Error with numeric operator calling: " + t_oper_string);
              }

            } else {
              chaiscript::eval::detail::Stack_Push_Pop spp(t_ss);
              return t_ss.call_function(t_oper_string, t_lhs, t_rhs);
            }
          }
          catch(const exception::dispatch_error &e){
            throw exception::eval_error("Can not find appropriate '" + t_oper_string + "' operator.", e.parameters, e.functions, false, t_ss);
          }
        }
    };

    struct Error_AST_Node : public AST_Node {
      public:
        Error_AST_Node(const std::string &t_ast_node_text = "", const boost::shared_ptr<std::string> &t_fname=boost::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          AST_Node(t_ast_node_text, AST_Node_Type::Error, t_fname, t_start_line, t_start_col, t_end_line, t_end_col) { }

        virtual ~Error_AST_Node() {}
    };

    struct Int_AST_Node : public AST_Node {
      public:
        Int_AST_Node(const std::string &t_ast_node_text = "", const boost::shared_ptr<std::string> &t_fname=boost::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          AST_Node(t_ast_node_text, AST_Node_Type::Int, t_fname, t_start_line, t_start_col, t_end_line, t_end_col), 
          m_value(const_var(int(atoi(t_ast_node_text.c_str())))) { }
        Int_AST_Node(const std::string &t_ast_node_text, const Boxed_Value &t_bv, const boost::shared_ptr<std::string> &t_fname=boost::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          AST_Node(t_ast_node_text, AST_Node_Type::Int, t_fname, t_start_line, t_start_col, t_end_line, t_end_col), 
          m_value(t_bv) { }
        virtual ~Int_AST_Node() {}
        virtual Boxed_Value eval_internal(chaiscript::detail::Dispatch_Engine &){
          return m_value;
        }

      private:
        Boxed_Value m_value;

    };

    struct Float_AST_Node : public AST_Node {
      public:
        Float_AST_Node(const std::string &t_ast_node_text = "", const boost::shared_ptr<std::string> &t_fname=boost::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          AST_Node(t_ast_node_text, AST_Node_Type::Float, t_fname, t_start_line, t_start_col, t_end_line, t_end_col),
          m_value(const_var(double(atof(t_ast_node_text.c_str())))) { }
        Float_AST_Node(const std::string &t_ast_node_text, const Boxed_Value &t_bv, const boost::shared_ptr<std::string> &t_fname=boost::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          AST_Node(t_ast_node_text, AST_Node_Type::Float, t_fname, t_start_line, t_start_col, t_end_line, t_end_col),
          m_value(t_bv) { }
        virtual ~Float_AST_Node() {}
        virtual Boxed_Value eval_internal(chaiscript::detail::Dispatch_Engine &){
          return m_value;
        }

      private:
        Boxed_Value m_value;

    };

    struct Id_AST_Node : public AST_Node {
      public:
        Id_AST_Node(const std::string &t_ast_node_text = "", const boost::shared_ptr<std::string> &t_fname=boost::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          AST_Node(t_ast_node_text, AST_Node_Type::Id, t_fname, t_start_line, t_start_col, t_end_line, t_end_col),
          m_value(get_value(t_ast_node_text))
      { }

        virtual ~Id_AST_Node() {}
        virtual Boxed_Value eval_internal(chaiscript::detail::Dispatch_Engine &t_ss) {
          if (!m_value.is_undef())
          {
            return m_value;
          } else {
            try {
              return t_ss.get_object(this->text);
            }
            catch (std::exception &) {
              throw exception::eval_error("Can not find object: " + this->text);
            }
          }
        }

      private:
        static Boxed_Value get_value(const std::string &t_text)
        {
          if (t_text == "true") {
            return const_var(true);
          }
          else if (t_text == "false") {
            return const_var(false);
          }
          else if (t_text == "Infinity") {
            return const_var(std::numeric_limits<double>::infinity());
          }
          else if (t_text == "NaN") {
            return const_var(std::numeric_limits<double>::quiet_NaN());
          } else {
            return Boxed_Value();
          }
        }

        Boxed_Value m_value;
    };

    struct Char_AST_Node : public AST_Node {
      public:
        Char_AST_Node(const std::string &t_ast_node_text = "", const boost::shared_ptr<std::string> &t_fname=boost::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          AST_Node(t_ast_node_text, AST_Node_Type::Char, t_fname, t_start_line, t_start_col, t_end_line, t_end_col) { }
        virtual ~Char_AST_Node() {}
    };

    struct Str_AST_Node : public AST_Node {
      public:
        Str_AST_Node(const std::string &t_ast_node_text = "", const boost::shared_ptr<std::string> &t_fname=boost::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          AST_Node(t_ast_node_text, AST_Node_Type::Str, t_fname, t_start_line, t_start_col, t_end_line, t_end_col) { }
        virtual ~Str_AST_Node() {}
    };

    struct Eol_AST_Node : public AST_Node {
      public:
        Eol_AST_Node(const std::string &t_ast_node_text = "", const boost::shared_ptr<std::string> &t_fname=boost::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          AST_Node(t_ast_node_text, AST_Node_Type::Eol, t_fname, t_start_line, t_start_col, t_end_line, t_end_col) { }
        virtual ~Eol_AST_Node() {}

        virtual std::string pretty_print() const 
        {
          return "\n";
        }
    };

    struct Fun_Call_AST_Node : public AST_Node {
      public:
        Fun_Call_AST_Node(const std::string &t_ast_node_text = "", const boost::shared_ptr<std::string> &t_fname=boost::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          AST_Node(t_ast_node_text, AST_Node_Type::Fun_Call, t_fname, t_start_line, t_start_col, t_end_line, t_end_col) { }
        virtual ~Fun_Call_AST_Node() {}
        virtual Boxed_Value eval_internal(chaiscript::detail::Dispatch_Engine &t_ss){
          chaiscript::eval::detail::Function_Push_Pop fpp(t_ss);
          dispatch::Param_List_Builder plb;

          if ((this->children.size() > 1) && (this->children[1]->identifier == AST_Node_Type::Arg_List)) {
            for (size_t i = 0; i < this->children[1]->children.size(); ++i) {
              plb << this->children[1]->children[i]->eval(t_ss);
            }
          }

          fpp.save_params(plb.objects);

          Boxed_Value fn = this->children[0]->eval(t_ss);

          try {
            chaiscript::eval::detail::Stack_Push_Pop spp(t_ss);
            const Boxed_Value &retval = (*t_ss.boxed_cast<const Const_Proxy_Function &>(fn))(plb, t_ss.conversions());
            return retval;
          }
          catch(const exception::dispatch_error &e){
            throw exception::eval_error(std::string(e.what()) + " with function '" + this->children[0]->text + "'", e.parameters, e.functions, false, t_ss);
          }
          catch(const exception::bad_boxed_cast &){
            try {
              Const_Proxy_Function f = t_ss.boxed_cast<const Const_Proxy_Function &>(fn);
              // handle the case where there is only 1 function to try to call and dispatch fails on it
              std::vector<Const_Proxy_Function> funcs;
              funcs.push_back(f);
              throw exception::eval_error("Error calling function '" + this->children[0]->text + "'", plb.objects, funcs, false, t_ss);
            } catch (const exception::bad_boxed_cast &) {
              throw exception::eval_error("'" + this->children[0]->pretty_print() + "' does not evaluate to a function.");
            }
          }
          catch(const exception::arity_error &e){
            throw exception::eval_error(std::string(e.what()) + " with function '" + this->children[0]->text + "'");
          }
          catch(const exception::guard_error &e){
            throw exception::eval_error(std::string(e.what()) + " with function '" + this->children[0]->text + "'");
          }
          catch(detail::Return_Value &rv) {
            return rv.retval;
          }
        }

        virtual std::string pretty_print() const 
        {
          std::ostringstream oss;

          for (unsigned int j = 0; j < this->children.size(); ++j) {
            oss << this->children[j]->pretty_print();

            if (j == 0)
            {
              oss << "(";
            }
          }

          oss << ")";

          return oss.str();
        }

    };

    struct Inplace_Fun_Call_AST_Node : public AST_Node {
      public:
        Inplace_Fun_Call_AST_Node(const std::string &t_ast_node_text = "", const boost::shared_ptr<std::string> &t_fname=boost::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          AST_Node(t_ast_node_text, AST_Node_Type::Inplace_Fun_Call, t_fname, t_start_line, t_start_col, t_end_line, t_end_col) { }
        virtual ~Inplace_Fun_Call_AST_Node() {}
        virtual Boxed_Value eval_internal(chaiscript::detail::Dispatch_Engine &t_ss){
          dispatch::Param_List_Builder plb;

          if ((this->children.size() > 1) && (this->children[1]->identifier == AST_Node_Type::Arg_List)) {
            for (size_t i = 0; i < this->children[1]->children.size(); ++i) {
              plb << this->children[1]->children[i]->eval(t_ss);
            }
          }

          Boxed_Value bv;
          Const_Proxy_Function fn;

          try {
            bv = this->children[0]->eval(t_ss);
            try {
              fn = t_ss.boxed_cast<const Const_Proxy_Function &>(bv);
            } catch (const exception::bad_boxed_cast &) {
              throw exception::eval_error("'" + this->children[0]->pretty_print() + "' does not evaluate to a function.");
            }
            return (*fn)(plb, t_ss.conversions());
          }
          catch(const exception::dispatch_error &e){
            throw exception::eval_error(std::string(e.what()) + " with function '" + this->children[0]->text + "'", e.parameters, e.functions, false, t_ss);
          }
          catch(const exception::bad_boxed_cast &){
            // handle the case where there is only 1 function to try to call and dispatch fails on it
            std::vector<Const_Proxy_Function> funcs;
            funcs.push_back(fn);
            throw exception::eval_error("Error calling function '" + this->children[0]->text + "'", plb.objects, funcs, false, t_ss);
          }
          catch(const exception::arity_error &e){
            throw exception::eval_error(std::string(e.what()) + " with function '" + this->children[0]->text + "'");
          }
          catch(const exception::guard_error &e){
            throw exception::eval_error(std::string(e.what()) + " with function '" + this->children[0]->text + "'");
          }
          catch(detail::Return_Value &rv) {
            return rv.retval;
          }
        }

        virtual std::string pretty_print() const 
        {
          std::ostringstream oss;
          for (unsigned int j = 0; j < this->children.size(); ++j) {
            oss << this->children[j]->pretty_print();

            if (j == 0)
            {
              oss << "(";
            }
          }

          oss << ")";

          return oss.str();
        }

    };

    struct Arg_List_AST_Node : public AST_Node {
      public:
        Arg_List_AST_Node(const std::string &t_ast_node_text = "", const boost::shared_ptr<std::string> &t_fname=boost::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          AST_Node(t_ast_node_text, AST_Node_Type::Arg_List, t_fname, t_start_line, t_start_col, t_end_line, t_end_col) { }
        virtual ~Arg_List_AST_Node() {}

        virtual std::string pretty_print() const 
        {
          std::ostringstream oss;
          for (unsigned int j = 0; j < this->children.size(); ++j) {
            if (j != 0)
            {
              oss << ", ";
            }

            oss << this->children[j]->pretty_print();
          }

          return oss.str();
        }
    };

    struct Variable_AST_Node : public AST_Node {
      public:
        Variable_AST_Node(const std::string &t_ast_node_text = "", const boost::shared_ptr<std::string> &t_fname=boost::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          AST_Node(t_ast_node_text, AST_Node_Type::Variable, t_fname, t_start_line, t_start_col, t_end_line, t_end_col) { }
        virtual ~Variable_AST_Node() {}
    };

    struct Equation_AST_Node : public AST_Node {
      public:
        Equation_AST_Node(const std::string &t_ast_node_text = "", const boost::shared_ptr<std::string> &t_fname=boost::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          AST_Node(t_ast_node_text, AST_Node_Type::Equation, t_fname, t_start_line, t_start_col, t_end_line, t_end_col)
        {}

        virtual ~Equation_AST_Node() {}
        virtual Boxed_Value eval_internal(chaiscript::detail::Dispatch_Engine &t_ss) {
          Boxed_Value retval = this->children.back()->eval(t_ss); 


          if (this->children.size() > 1) {
            Boxed_Value lhs = this->children[0]->eval(t_ss);

            Operators::Opers oper = Operators::to_operator(this->children[1]->text);

            if (oper != Operators::invalid && lhs.get_type_info().is_arithmetic() &&
                retval.get_type_info().is_arithmetic())
            {
              try {
                retval = Boxed_Number::do_oper(oper, lhs, retval);
              } catch (const std::exception &) {
                throw exception::eval_error("Error with unsupported arithmetic assignment operation");
              }
            } else if (this->children[1]->text == "=") {
              try {
                if (lhs.is_undef()) {
                  retval = t_ss.call_function("clone", retval);
                }

                try {
                  retval = t_ss.call_function(this->children[1]->text, lhs, retval);
                }
                catch(const exception::dispatch_error &e){
                  throw exception::eval_error("Unable to find appropriate'" + this->children[1]->text + "' operator.", e.parameters, e.functions, false, t_ss);
                }
              }
              catch(const exception::dispatch_error &e){
                throw exception::eval_error("Missing clone or copy constructor for right hand side of equation", e.parameters, e.functions, false, t_ss);
              }
            }
            else if (this->children[1]->text == ":=") {
              if (lhs.is_undef() || type_match(lhs, retval)) {
                lhs.assign(retval);
              } else {
                throw exception::eval_error("Mismatched types in equation");
              }
            }
            else {
              try {
                retval = t_ss.call_function(this->children[1]->text, lhs, retval);
              } catch(const exception::dispatch_error &e){
                  throw exception::eval_error("Unable to find appropriate'" + this->children[1]->text + "' operator.", e.parameters, e.functions, false, t_ss);
              }
            }
          }
          return retval;
        }
    };

    struct Var_Decl_AST_Node : public AST_Node {
      public:
        Var_Decl_AST_Node(const std::string &t_ast_node_text = "", const boost::shared_ptr<std::string> &t_fname=boost::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          AST_Node(t_ast_node_text, AST_Node_Type::Var_Decl, t_fname, t_start_line, t_start_col, t_end_line, t_end_col) { }
        virtual ~Var_Decl_AST_Node() {}
        virtual Boxed_Value eval_internal(chaiscript::detail::Dispatch_Engine &t_ss){
          try {
            t_ss.add_object(this->children[0]->text, Boxed_Value());
          }
          catch (const exception::reserved_word_error &) {
            throw exception::eval_error("Reserved word used as variable '" + this->children[0]->text + "'");
          } catch (const exception::name_conflict_error &e) {
            throw exception::eval_error("Variable redefined '" + e.name() + "'");
          }
          return t_ss.get_object(this->children[0]->text);
        }

        virtual std::string pretty_print() const 
        {
          return "var " + this->children[0]->text;
        }

    };

    struct Comparison_AST_Node : public Binary_Operator_AST_Node {
      public:
        Comparison_AST_Node(const std::string &t_ast_node_text = "", const boost::shared_ptr<std::string> &t_fname=boost::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          Binary_Operator_AST_Node(t_ast_node_text, AST_Node_Type::Comparison, t_fname, t_start_line, t_start_col, t_end_line, t_end_col) { }
        virtual ~Comparison_AST_Node() {}
    };

    struct Addition_AST_Node : public Binary_Operator_AST_Node {
      public:
        Addition_AST_Node(const std::string &t_ast_node_text = "+",
            const boost::shared_ptr<std::string> &t_fname=boost::shared_ptr<std::string>(),
            int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          Binary_Operator_AST_Node(t_ast_node_text, AST_Node_Type::Addition, t_fname, t_start_line, t_start_col, t_end_line, t_end_col) { }
        virtual ~Addition_AST_Node() {}
        virtual Boxed_Value eval_internal(chaiscript::detail::Dispatch_Engine &t_ss) {
          return do_oper(t_ss, Operators::sum, "+", this->children[0]->eval(t_ss), this->children[1]->eval(t_ss));
        }
    };

    struct Subtraction_AST_Node : public Binary_Operator_AST_Node {
      public:
        Subtraction_AST_Node(const std::string &t_ast_node_text = "-",
            const boost::shared_ptr<std::string> &t_fname=boost::shared_ptr<std::string>(), int t_start_line = 0,
            int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          Binary_Operator_AST_Node(t_ast_node_text, AST_Node_Type::Subtraction, t_fname, t_start_line, t_start_col, t_end_line, t_end_col) { }
        virtual ~Subtraction_AST_Node() {}
        virtual Boxed_Value eval_internal(chaiscript::detail::Dispatch_Engine &t_ss) {
          return do_oper(t_ss, Operators::difference, "-", this->children[0]->eval(t_ss), this->children[1]->eval(t_ss));
        }
    };

    struct Multiplication_AST_Node : public Binary_Operator_AST_Node {
      public:
        Multiplication_AST_Node(const std::string &t_ast_node_text = "*",
            const boost::shared_ptr<std::string> &t_fname=boost::shared_ptr<std::string>(), int t_start_line = 0,
            int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          Binary_Operator_AST_Node(t_ast_node_text, AST_Node_Type::Multiplication, t_fname, t_start_line, t_start_col, t_end_line, t_end_col) { }
        virtual ~Multiplication_AST_Node() {}
        virtual Boxed_Value eval_internal(chaiscript::detail::Dispatch_Engine &t_ss) {
          return do_oper(t_ss, Operators::product, "*", this->children[0]->eval(t_ss), this->children[1]->eval(t_ss));
        }
    };

    struct Division_AST_Node : public Binary_Operator_AST_Node {
      public:
        Division_AST_Node(const std::string &t_ast_node_text = "/",
            const boost::shared_ptr<std::string> &t_fname=boost::shared_ptr<std::string>(), int t_start_line = 0,
            int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          Binary_Operator_AST_Node(t_ast_node_text, AST_Node_Type::Division, t_fname, t_start_line, t_start_col, t_end_line, t_end_col) { }
        virtual ~Division_AST_Node() {}
        virtual Boxed_Value eval_internal(chaiscript::detail::Dispatch_Engine &t_ss) {
          return do_oper(t_ss, Operators::quotient, "/", this->children[0]->eval(t_ss), this->children[1]->eval(t_ss));
        }
    };

    struct Modulus_AST_Node : public Binary_Operator_AST_Node {
      public:
        Modulus_AST_Node(const std::string &t_ast_node_text = "%",
            const boost::shared_ptr<std::string> &t_fname=boost::shared_ptr<std::string>(), int t_start_line = 0,
            int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          Binary_Operator_AST_Node(t_ast_node_text, AST_Node_Type::Modulus, t_fname, t_start_line, t_start_col, t_end_line, t_end_col) { }
        virtual ~Modulus_AST_Node() {}
        virtual Boxed_Value eval_internal(chaiscript::detail::Dispatch_Engine &t_ss) {
          return do_oper(t_ss, Operators::remainder, "%", this->children[0]->eval(t_ss), this->children[1]->eval(t_ss));
        }
    };

    struct Array_Call_AST_Node : public AST_Node {
      public:
        Array_Call_AST_Node(const std::string &t_ast_node_text = "", const boost::shared_ptr<std::string> &t_fname=boost::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          AST_Node(t_ast_node_text, AST_Node_Type::Array_Call, t_fname, t_start_line, t_start_col, t_end_line, t_end_col) { }
        virtual ~Array_Call_AST_Node() {}
        virtual Boxed_Value eval_internal(chaiscript::detail::Dispatch_Engine &t_ss){
          chaiscript::eval::detail::Function_Push_Pop fpp(t_ss);
          Boxed_Value retval = this->children[0]->eval(t_ss);
          std::vector<Boxed_Value> params;
          params.push_back(retval);

          for (size_t i = 1; i < this->children.size(); ++i) {
            try {
              Boxed_Value p1(this->children[i]->eval(t_ss));

              chaiscript::eval::detail::Stack_Push_Pop spp(t_ss);
              params.push_back(p1);
              fpp.save_params(params);
              params.clear();
              retval = t_ss.call_function("[]", retval, p1);
            }
            catch(std::out_of_range &) {
              throw exception::eval_error("Out of bounds exception");
            }
            catch(const exception::dispatch_error &e){
              throw exception::eval_error("Can not find appropriate array lookup operator '[]'.", e.parameters, e.functions, false, t_ss );
            }
          }

          return retval;
        }

        virtual std::string pretty_print() const 
        {
          std::ostringstream oss;
          oss << this->children[0]->pretty_print();

          for (size_t i = 1; i < this->children.size(); ++i)
          {
            oss << "[";
            oss << this->children[i]->pretty_print();
            oss << "]";
          }

          return oss.str();
        }
    };

    struct Dot_Access_AST_Node : public AST_Node {
      public:
        Dot_Access_AST_Node(const std::string &t_ast_node_text = "", const boost::shared_ptr<std::string> &t_fname=boost::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          AST_Node(t_ast_node_text, AST_Node_Type::Dot_Access, t_fname, t_start_line, t_start_col, t_end_line, t_end_col) { }
        virtual ~Dot_Access_AST_Node() {}
        virtual Boxed_Value eval_internal(chaiscript::detail::Dispatch_Engine &t_ss){
          Boxed_Value retval = this->children[0]->eval(t_ss);

          if (this->children.size() > 1) {
            for (size_t i = 2; i < this->children.size(); i+=2) {
              chaiscript::eval::detail::Function_Push_Pop fpp(t_ss);
              dispatch::Param_List_Builder plb;
              plb << retval;

              if (this->children[i]->children.size() > 1) {
                for (size_t j = 0; j < this->children[i]->children[1]->children.size(); ++j) {
                  plb << this->children[i]->children[1]->children[j]->eval(t_ss);
                }
              }

              fpp.save_params(plb.objects);

              std::string fun_name;
              if ((this->children[i]->identifier == AST_Node_Type::Fun_Call) || (this->children[i]->identifier == AST_Node_Type::Array_Call)) {
                fun_name = this->children[i]->children[0]->text;
              }
              else {
                fun_name = this->children[i]->text;
              }

              try {
                chaiscript::eval::detail::Stack_Push_Pop spp(t_ss);
                retval = t_ss.call_function(fun_name, plb);
              }
              catch(const exception::dispatch_error &e){
                if (e.functions.empty())
                {
                  throw exception::eval_error("'" + fun_name + "' is not a function.");
                } else {
                  throw exception::eval_error(std::string(e.what()) + " for function '" + fun_name + "'", e.parameters, e.functions, true, t_ss);
                }
              }
              catch(detail::Return_Value &rv) {
                retval = rv.retval;
              }

              if (this->children[i]->identifier == AST_Node_Type::Array_Call) {
                for (size_t j = 1; j < this->children[i]->children.size(); ++j) {
                  try {
                    retval = t_ss.call_function("[]", retval, this->children[i]->children[j]->eval(t_ss));
                  }
                  catch(std::out_of_range &) {
                    throw exception::eval_error("Out of bounds exception");
                  }
                  catch(const exception::dispatch_error &e){
                    throw exception::eval_error("Can not find appropriate array lookup operator '[]'.", e.parameters, e.functions, true, t_ss);
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
        Quoted_String_AST_Node(const std::string &t_ast_node_text = "", const boost::shared_ptr<std::string> &t_fname=boost::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          AST_Node(t_ast_node_text, AST_Node_Type::Quoted_String, t_fname, t_start_line, t_start_col, t_end_line, t_end_col),
          m_value(const_var(t_ast_node_text)) { }
        virtual ~Quoted_String_AST_Node() {}
        virtual Boxed_Value eval_internal(chaiscript::detail::Dispatch_Engine &) {
          return m_value;
        }

        virtual std::string pretty_print() const 
        {
          return "\"" + text + "\"";
        }

      private:
        Boxed_Value m_value;

    };

    struct Single_Quoted_String_AST_Node : public AST_Node {
      public:
        Single_Quoted_String_AST_Node(const std::string &t_ast_node_text = "", const boost::shared_ptr<std::string> &t_fname=boost::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          AST_Node(t_ast_node_text, AST_Node_Type::Single_Quoted_String, t_fname, t_start_line, t_start_col, t_end_line, t_end_col),
          m_value(const_var(char(t_ast_node_text.at(0)))) { }
        virtual ~Single_Quoted_String_AST_Node() {}
        virtual Boxed_Value eval_internal(chaiscript::detail::Dispatch_Engine &){
          return m_value;
        }

        virtual std::string pretty_print() const 
        {
          return "'" + text + "'";
        }

      private:
        Boxed_Value m_value;
    };

    struct Lambda_AST_Node : public AST_Node {
      public:
        Lambda_AST_Node(const std::string &t_ast_node_text = "", const boost::shared_ptr<std::string> &t_fname=boost::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          AST_Node(t_ast_node_text, AST_Node_Type::Lambda, t_fname, t_start_line, t_start_col, t_end_line, t_end_col) { }
        virtual ~Lambda_AST_Node() {}
        virtual Boxed_Value eval_internal(chaiscript::detail::Dispatch_Engine &t_ss){
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
                (boost::bind(chaiscript::eval::detail::eval_function, boost::ref(t_ss), this->children.back(), t_param_names, _1),
                 static_cast<int>(numparams), this->children.back())));
        }

    };

    struct Block_AST_Node : public AST_Node {
      public:
        Block_AST_Node(const std::string &t_ast_node_text = "", const boost::shared_ptr<std::string> &t_fname=boost::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          AST_Node(t_ast_node_text, AST_Node_Type::Block, t_fname, t_start_line, t_start_col, t_end_line, t_end_col) { }
        virtual ~Block_AST_Node() {}
        virtual Boxed_Value eval_internal(chaiscript::detail::Dispatch_Engine &t_ss){
          const size_t num_children = this->children.size();

          chaiscript::eval::detail::Scope_Push_Pop spp(t_ss);

          for (size_t i = 0; i < num_children; ++i) {
            try {
              const Boxed_Value &retval = this->children[i]->eval(t_ss);

              if (i + 1 == num_children)
              {
                return retval;
              }
            }
            catch (const chaiscript::eval::detail::Return_Value &) {
              throw;
            }
          }

          return Boxed_Value();
        }

    };

    struct Def_AST_Node : public AST_Node {
      public:
        Def_AST_Node(const std::string &t_ast_node_text = "", const boost::shared_ptr<std::string> &t_fname=boost::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          AST_Node(t_ast_node_text, AST_Node_Type::Def, t_fname, t_start_line, t_start_col, t_end_line, t_end_col) { }
        virtual ~Def_AST_Node() {}
        virtual Boxed_Value eval_internal(chaiscript::detail::Dispatch_Engine &t_ss){
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
              (new dispatch::Dynamic_Proxy_Function(boost::bind(chaiscript::eval::detail::eval_function,
                                                                boost::ref(t_ss), guardnode,
                                                                t_param_names, _1), static_cast<int>(numparams), guardnode));
          }

          try {
            const std::string & l_function_name = this->children[0]->text;
            const std::string & l_annotation = this->annotation?this->annotation->text:"";
            t_ss.add(Proxy_Function
                (new dispatch::Dynamic_Proxy_Function(boost::bind(chaiscript::eval::detail::eval_function,
                                                                  boost::ref(t_ss), this->children.back(),
                                                                  t_param_names, _1), static_cast<int>(numparams), this->children.back(),
                                                      l_annotation, guard)), l_function_name);
          }
          catch (const exception::reserved_word_error &e) {
            throw exception::eval_error("Reserved word used as function name '" + e.word() + "'");
          } catch (const exception::name_conflict_error &e) {
            throw exception::eval_error("Function redefined '" + e.name() + "'");
          }
          return Boxed_Value();
        }

    };

    struct While_AST_Node : public AST_Node {
      public:
        While_AST_Node(const std::string &t_ast_node_text = "", const boost::shared_ptr<std::string> &t_fname=boost::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          AST_Node(t_ast_node_text, AST_Node_Type::While, t_fname, t_start_line, t_start_col, t_end_line, t_end_col) { }
        virtual ~While_AST_Node() {}
        virtual Boxed_Value eval_internal(chaiscript::detail::Dispatch_Engine &t_ss) {
          chaiscript::eval::detail::Scope_Push_Pop spp(t_ss);

          try {
            while (boxed_cast<bool>(this->children[0]->eval(t_ss))) {
              try {
                this->children[1]->eval(t_ss);
              } catch (detail::Continue_Loop &) {
                // we got a continue exception, which means all of the remaining 
                // loop implementation is skipped and we just need to continue to
                // the next condition test
              }
            } 
          } catch (const exception::bad_boxed_cast &) {
            throw exception::eval_error("While condition not boolean");
          } catch (detail::Break_Loop &) {
            // loop was broken intentionally
          }

          return Boxed_Value();
        }

    };

    struct Ternary_Cond_AST_Node : public AST_Node {
      public:
        Ternary_Cond_AST_Node(const std::string &t_ast_node_text = "", const boost::shared_ptr<std::string> &t_fname=boost::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          AST_Node(t_ast_node_text, AST_Node_Type::If, t_fname, t_start_line, t_start_col, t_end_line, t_end_col) { }
        virtual ~Ternary_Cond_AST_Node() {}
        virtual Boxed_Value eval_internal(chaiscript::detail::Dispatch_Engine &t_ss){
          bool cond;
          try {
            cond = boxed_cast<bool>(this->children[0]->eval(t_ss));
          }
          catch (const exception::bad_boxed_cast &) {
            throw exception::eval_error("Ternary if condition not boolean");
          }

          if (cond) {
            return this->children[1]->eval(t_ss);
          }
          else {
            return this->children[2]->eval(t_ss);
          }
        }
    };

    struct If_AST_Node : public AST_Node {
      public:
        If_AST_Node(const std::string &t_ast_node_text = "", const boost::shared_ptr<std::string> &t_fname=boost::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          AST_Node(t_ast_node_text, AST_Node_Type::If, t_fname, t_start_line, t_start_col, t_end_line, t_end_col) { }
        virtual ~If_AST_Node() {}
        virtual Boxed_Value eval_internal(chaiscript::detail::Dispatch_Engine &t_ss){
          bool cond;
          try {
            cond = boxed_cast<bool>(this->children[0]->eval(t_ss));
          }
          catch (const exception::bad_boxed_cast &) {
            throw exception::eval_error("If condition not boolean");
          }

          if (cond) {
            return this->children[1]->eval(t_ss);
          }
          else {
            if (this->children.size() > 2) {
              size_t i = 2;
              while ((!cond) && (i < this->children.size())) {
                if (this->children[i]->text == "else") {
                  return this->children[i+1]->eval(t_ss);
                }
                else if (this->children[i]->text == "else if") {
                  try {
                    cond = boxed_cast<bool>(this->children[i+1]->eval(t_ss));
                  }
                  catch (const exception::bad_boxed_cast &) {
                    throw exception::eval_error("'else if' condition not boolean");
                  }
                  if (cond) {
                    return this->children[i+2]->eval(t_ss);
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
        For_AST_Node(const std::string &t_ast_node_text = "", const boost::shared_ptr<std::string> &t_fname=boost::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          AST_Node(t_ast_node_text, AST_Node_Type::For, t_fname, t_start_line, t_start_col, t_end_line, t_end_col) { }
        virtual ~For_AST_Node() {}
        virtual Boxed_Value eval_internal(chaiscript::detail::Dispatch_Engine &t_ss){
          chaiscript::eval::detail::Scope_Push_Pop spp(t_ss);

          // initial expression
          this->children[0]->eval(t_ss);

          try {
            // while condition evals to true
            while (boxed_cast<bool>(this->children[1]->eval(t_ss))) {
              try {
                // Body of Loop
                this->children[3]->eval(t_ss);
              } catch (detail::Continue_Loop &) {
                // we got a continue exception, which means all of the remaining 
                // loop implementation is skipped and we just need to continue to
                // the next iteration step
              }

              // loop expression
              this->children[2]->eval(t_ss);
            }
          }
          catch (const exception::bad_boxed_cast &) {
            throw exception::eval_error("For condition not boolean");
          }
          catch (detail::Break_Loop &) {
            // loop broken
          }

          return Boxed_Value();
        }

    };

    struct Switch_AST_Node : public AST_Node {
      public:
        Switch_AST_Node(const std::string &t_ast_node_text = "", const boost::shared_ptr<std::string> &t_fname=boost::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          AST_Node(t_ast_node_text, AST_Node_Type::Switch, t_fname, t_start_line, t_start_col, t_end_line, t_end_col) { }
        virtual ~Switch_AST_Node() {}
        virtual Boxed_Value eval_internal(chaiscript::detail::Dispatch_Engine &t_ss) {
          Boxed_Value match_value;
          bool breaking = false;
          size_t currentCase = 1;
          bool hasMatched = false;

          chaiscript::eval::detail::Scope_Push_Pop spp(t_ss);

          match_value = this->children[0]->eval(t_ss);

          while (!breaking && (currentCase < this->children.size())) {
            try {
              if (this->children[currentCase]->identifier == AST_Node_Type::Case) {
                //This is a little odd, but because want to see both the switch and the case simultaneously, I do a downcast here.
                try {
                  if (hasMatched || boxed_cast<bool>(t_ss.call_function("==", match_value, this->children[currentCase]->children[0]->eval(t_ss)))) {
                    this->children[currentCase]->eval(t_ss);
                    hasMatched = true;
                  }
                }
                catch (const exception::bad_boxed_cast &) {
                  throw exception::eval_error("Internal error: case guard evaluation not boolean");
                }
              }
              else if (this->children[currentCase]->identifier == AST_Node_Type::Default) {
                this->children[currentCase]->eval(t_ss);
                breaking = true;
              }
            }
            catch (detail::Break_Loop &) {
              breaking = true;
            }
            ++currentCase;
          }
          return Boxed_Value();
        }
    };

    struct Case_AST_Node : public AST_Node {
      public:
        Case_AST_Node(const std::string &t_ast_node_text = "", const boost::shared_ptr<std::string> &t_fname=boost::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          AST_Node(t_ast_node_text, AST_Node_Type::Case, t_fname, t_start_line, t_start_col, t_end_line, t_end_col) { }
        virtual ~Case_AST_Node() {}
        virtual Boxed_Value eval_internal(chaiscript::detail::Dispatch_Engine &t_ss) {
          chaiscript::eval::detail::Scope_Push_Pop spp(t_ss);

          this->children[1]->eval(t_ss);
          
          return Boxed_Value();
        }
    };
   
    struct Default_AST_Node : public AST_Node {
      public:
        Default_AST_Node(const std::string &t_ast_node_text = "", const boost::shared_ptr<std::string> &t_fname=boost::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          AST_Node(t_ast_node_text, AST_Node_Type::Default, t_fname, t_start_line, t_start_col, t_end_line, t_end_col) { }
        virtual ~Default_AST_Node() {}
        virtual Boxed_Value eval_internal(chaiscript::detail::Dispatch_Engine &t_ss) {
          chaiscript::eval::detail::Scope_Push_Pop spp(t_ss);

          this->children[0]->eval(t_ss);

          return Boxed_Value();
        }
    };
       
    struct Inline_Array_AST_Node : public AST_Node {
      public:
        Inline_Array_AST_Node(const std::string &t_ast_node_text = "", const boost::shared_ptr<std::string> &t_fname=boost::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          AST_Node(t_ast_node_text, AST_Node_Type::Inline_Array, t_fname, t_start_line, t_start_col, t_end_line, t_end_col) { }
        virtual ~Inline_Array_AST_Node() {}
        virtual Boxed_Value eval_internal(chaiscript::detail::Dispatch_Engine &t_ss){
          try {
            std::vector<Boxed_Value> vec;
            if (this->children.size() > 0) {
              for (size_t i = 0; i < this->children[0]->children.size(); ++i) {
                Boxed_Value bv = t_ss.call_function("clone", this->children[0]->children[i]->eval(t_ss));
                vec.push_back(bv);
              }
            }
            return const_var(vec);
          }
          catch (const exception::dispatch_error &) {
            throw exception::eval_error("Can not find appropriate 'clone' or copy constructor for vector elements");
          }

        }

        virtual std::string pretty_print() const
        {
          return "[" + AST_Node::pretty_print() + "]";
        }
    };

    struct Inline_Map_AST_Node : public AST_Node {
      public:
        Inline_Map_AST_Node(const std::string &t_ast_node_text = "", const boost::shared_ptr<std::string> &t_fname=boost::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          AST_Node(t_ast_node_text, AST_Node_Type::Inline_Map, t_fname, t_start_line, t_start_col, t_end_line, t_end_col) { }
        virtual ~Inline_Map_AST_Node() {}
        virtual Boxed_Value eval_internal(chaiscript::detail::Dispatch_Engine &t_ss){
          try {
            std::map<std::string, Boxed_Value> retval;
            for (size_t i = 0; i < this->children[0]->children.size(); ++i) {
              Boxed_Value bv = t_ss.call_function("clone", this->children[0]->children[i]->children[1]->eval(t_ss));
              retval[t_ss.boxed_cast<std::string>(this->children[0]->children[i]->children[0]->eval(t_ss))] 
                = bv;
            }
            return const_var(retval);
          }
          catch (const exception::dispatch_error &e) {
            throw exception::eval_error("Can not find appropriate copy constructor or 'clone' while inserting into Map.", e.parameters, e.functions, false, t_ss);
          }
        }

    };

    struct Return_AST_Node : public AST_Node {
      public:
        Return_AST_Node(const std::string &t_ast_node_text = "", const boost::shared_ptr<std::string> &t_fname=boost::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          AST_Node(t_ast_node_text, AST_Node_Type::Return, t_fname, t_start_line, t_start_col, t_end_line, t_end_col) { }
        virtual ~Return_AST_Node() {}
        virtual Boxed_Value eval_internal(chaiscript::detail::Dispatch_Engine &t_ss){
          if (this->children.size() > 0) {
            throw detail::Return_Value(this->children[0]->eval(t_ss));
          }
          else {
            throw detail::Return_Value(Boxed_Value());
          }
        }

    };

    struct File_AST_Node : public AST_Node {
      public:
        File_AST_Node(const std::string &t_ast_node_text = "", const boost::shared_ptr<std::string> &t_fname=boost::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          AST_Node(t_ast_node_text, AST_Node_Type::File, t_fname, t_start_line, t_start_col, t_end_line, t_end_col) { }
        virtual ~File_AST_Node() {}
        virtual Boxed_Value eval_internal(chaiscript::detail::Dispatch_Engine &t_ss) {
          const size_t size = this->children.size(); 
          for (size_t i = 0; i < size; ++i) {
            const Boxed_Value &retval = this->children[i]->eval(t_ss);
            if (i + 1 == size) {
              return retval;
            }
          }
          return Boxed_Value();
        }
    };

    struct Prefix_AST_Node : public AST_Node {
      public:
        Prefix_AST_Node(const std::string &t_ast_node_text = "", const boost::shared_ptr<std::string> &t_fname=boost::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          AST_Node(t_ast_node_text, AST_Node_Type::Prefix, t_fname, t_start_line, t_start_col, t_end_line, t_end_col)
        { }

        virtual ~Prefix_AST_Node() {}
        virtual Boxed_Value eval_internal(chaiscript::detail::Dispatch_Engine &t_ss){
          chaiscript::eval::detail::Function_Push_Pop fpp(t_ss);
          Boxed_Value bv(this->children[1]->eval(t_ss));

          Operators::Opers oper = Operators::to_operator(children[0]->text, true);
          try {
            // short circuit arithmetic operations
            if (bv.get_type_info().is_arithmetic() && oper != Operators::invalid)
            {
              return Boxed_Number::do_oper(oper, bv);
            } else {
              chaiscript::eval::detail::Stack_Push_Pop spp(t_ss);
              std::vector<Boxed_Value> params;
              params.push_back(bv);
              fpp.save_params(params);
              return t_ss.call_function(this->children[0]->text, bv);
            }
          } catch (const exception::dispatch_error &e) {
            throw exception::eval_error("Error with prefix operator evaluation: '" + children[0]->text + "'", e.parameters, e.functions, false, t_ss);
          }
        }

    };

    struct Break_AST_Node : public AST_Node {
      public:
        Break_AST_Node(const std::string &t_ast_node_text = "", const boost::shared_ptr<std::string> &t_fname=boost::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          AST_Node(t_ast_node_text, AST_Node_Type::Break, t_fname, t_start_line, t_start_col, t_end_line, t_end_col) { }
        virtual ~Break_AST_Node() {}
        virtual Boxed_Value eval_internal(chaiscript::detail::Dispatch_Engine &){
          throw detail::Break_Loop();
        }
    };

    struct Continue_AST_Node : public AST_Node {
      public:
        Continue_AST_Node(const std::string &t_ast_node_text = "", const boost::shared_ptr<std::string> &t_fname=boost::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          AST_Node(t_ast_node_text, AST_Node_Type::Continue, t_fname, t_start_line, t_start_col, t_end_line, t_end_col) { }
        virtual ~Continue_AST_Node() {}
        virtual Boxed_Value eval_internal(chaiscript::detail::Dispatch_Engine &){
          throw detail::Continue_Loop();
        }
    };

    struct Noop_AST_Node : public AST_Node {
      public:
        Noop_AST_Node(const std::string &t_ast_node_text = "", const boost::shared_ptr<std::string> &t_fname=boost::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          AST_Node(t_ast_node_text, AST_Node_Type::Noop, t_fname, t_start_line, t_start_col, t_end_line, t_end_col),
          m_value(const_var(true))
        { }

        virtual ~Noop_AST_Node() {}
        virtual Boxed_Value eval_internal(chaiscript::detail::Dispatch_Engine &){
          // It's a no-op, that evaluates to "true"
          return m_value;
        }

      private:
        Boxed_Value m_value;
    };

    struct Map_Pair_AST_Node : public AST_Node {
      public:
        Map_Pair_AST_Node(const std::string &t_ast_node_text = "", const boost::shared_ptr<std::string> &t_fname=boost::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          AST_Node(t_ast_node_text, AST_Node_Type::Map_Pair, t_fname, t_start_line, t_start_col, t_end_line, t_end_col) { }
        virtual ~Map_Pair_AST_Node() {}
    };

    struct Value_Range_AST_Node : public AST_Node {
      public:
        Value_Range_AST_Node(const std::string &t_ast_node_text = "", const boost::shared_ptr<std::string> &t_fname=boost::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          AST_Node(t_ast_node_text, AST_Node_Type::Value_Range, t_fname, t_start_line, t_start_col, t_end_line, t_end_col) { }
        virtual ~Value_Range_AST_Node() {}
    };

    struct Inline_Range_AST_Node : public AST_Node {
      public:
        Inline_Range_AST_Node(const std::string &t_ast_node_text = "", const boost::shared_ptr<std::string> &t_fname=boost::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          AST_Node(t_ast_node_text, AST_Node_Type::Inline_Range, t_fname, t_start_line, t_start_col, t_end_line, t_end_col) { }
        virtual ~Inline_Range_AST_Node() {}
        virtual Boxed_Value eval_internal(chaiscript::detail::Dispatch_Engine &t_ss){
          try {
            return t_ss.call_function("generate_range",
                this->children[0]->children[0]->children[0]->eval(t_ss),
                this->children[0]->children[0]->children[1]->eval(t_ss));
          }
          catch (const exception::dispatch_error &e) {
            throw exception::eval_error("Unable to generate range vector, while calling 'generate_range'", e.parameters, e.functions, false, t_ss);
          }
        }

    };

    struct Annotation_AST_Node : public AST_Node {
      public:
        Annotation_AST_Node(const std::string &t_ast_node_text = "", const boost::shared_ptr<std::string> &t_fname=boost::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          AST_Node(t_ast_node_text, AST_Node_Type::Annotation, t_fname, t_start_line, t_start_col, t_end_line, t_end_col) { }
        virtual ~Annotation_AST_Node() {}
    };

    struct Try_AST_Node : public AST_Node {
      public:
        Try_AST_Node(const std::string &t_ast_node_text = "", const boost::shared_ptr<std::string> &t_fname=boost::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          AST_Node(t_ast_node_text, AST_Node_Type::Try, t_fname, t_start_line, t_start_col, t_end_line, t_end_col) { }
        virtual ~Try_AST_Node() {}
        virtual Boxed_Value eval_internal(chaiscript::detail::Dispatch_Engine &t_ss){
          Boxed_Value retval;        

          chaiscript::eval::detail::Scope_Push_Pop spp(t_ss);

          try {
            retval = this->children[0]->eval(t_ss);
          }
          catch (exception::eval_error &) {
            if (this->children.back()->identifier == AST_Node_Type::Finally) {
              this->children.back()->children[0]->eval(t_ss);
            }
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
              chaiscript::eval::detail::Scope_Push_Pop catchscope(t_ss);
              AST_NodePtr catch_block = this->children[i];

              if (catch_block->children.size() == 1) {
                //No variable capture, no guards
                retval = catch_block->children[0]->eval(t_ss);
                break;
              }
              else if (catch_block->children.size() == 2) {
                //Variable capture, no guards
                t_ss.add_object(catch_block->children[0]->text, except);
                retval = catch_block->children[1]->eval(t_ss);

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
                    this->children.back()->children[0]->eval(t_ss);
                  }
                  throw exception::eval_error("Guard condition not boolean");
                }
                if (guard) {
                  retval = catch_block->children[2]->eval(t_ss);
                  break;
                }
              }
              else {
                if (this->children.back()->identifier == AST_Node_Type::Finally) {
                  this->children.back()->children[0]->eval(t_ss);
                }
                throw exception::eval_error("Internal error: catch block size unrecognized");
              }
            }
          }
          catch (Boxed_Value &except) {
            for (size_t i = 1; i < this->children.size(); ++i) {
              chaiscript::eval::detail::Scope_Push_Pop catchscope(t_ss);
              AST_NodePtr catch_block = this->children[i];

              if (catch_block->children.size() == 1) {
                //No variable capture, no guards
                retval = catch_block->children[0]->eval(t_ss);
                break;
              }
              else if (catch_block->children.size() == 2) {
                //Variable capture, no guards
                t_ss.add_object(catch_block->children[0]->text, except);
                retval = catch_block->children[1]->eval(t_ss);
                break;
              }
              else if (catch_block->children.size() == 3) {
                //Variable capture, guards
                t_ss.add_object(catch_block->children[0]->text, except);

                bool guard;
                try {
                  guard = boxed_cast<bool>(catch_block->children[1]->eval(t_ss));
                }
                catch (const exception::bad_boxed_cast &) {
                  if (this->children.back()->identifier == AST_Node_Type::Finally) {
                    this->children.back()->children[0]->eval(t_ss);
                  }

                  throw exception::eval_error("Guard condition not boolean");
                }
                if (guard) {
                  retval = catch_block->children[2]->eval(t_ss);
                  break;
                }
              }
              else {
                if (this->children.back()->identifier == AST_Node_Type::Finally) {
                  this->children.back()->children[0]->eval(t_ss);
                }
                throw exception::eval_error("Internal error: catch block size unrecognized");
              }
            }
          }
          catch (...) {
            if (this->children.back()->identifier == AST_Node_Type::Finally) {
              this->children.back()->children[0]->eval(t_ss);
            }
            throw;
          }

          if (this->children.back()->identifier == AST_Node_Type::Finally) {
            retval = this->children.back()->children[0]->eval(t_ss);
          }

          return retval;
        }

    };

    struct Catch_AST_Node : public AST_Node {
      public:
        Catch_AST_Node(const std::string &t_ast_node_text = "", const boost::shared_ptr<std::string> &t_fname=boost::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          AST_Node(t_ast_node_text, AST_Node_Type::Catch, t_fname, t_start_line, t_start_col, t_end_line, t_end_col) { }
        virtual ~Catch_AST_Node() {}
    };

    struct Finally_AST_Node : public AST_Node {
      public:
        Finally_AST_Node(const std::string &t_ast_node_text = "", const boost::shared_ptr<std::string> &t_fname=boost::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          AST_Node(t_ast_node_text, AST_Node_Type::Finally, t_fname, t_start_line, t_start_col, t_end_line, t_end_col) { }
        virtual ~Finally_AST_Node() {}
    };

    struct Method_AST_Node : public AST_Node {
      public:
        Method_AST_Node(const std::string &t_ast_node_text = "", const boost::shared_ptr<std::string> &t_fname=boost::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          AST_Node(t_ast_node_text, AST_Node_Type::Method, t_fname, t_start_line, t_start_col, t_end_line, t_end_col) { }
        virtual ~Method_AST_Node() {}
        virtual Boxed_Value eval_internal(chaiscript::detail::Dispatch_Engine &t_ss){

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
              (new dispatch::Dynamic_Proxy_Function(boost::bind(chaiscript::eval::detail::eval_function,
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
                                                                    (new dispatch::Dynamic_Proxy_Function(boost::bind(chaiscript::eval::detail::eval_function,
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
                                                                 (new dispatch::Dynamic_Proxy_Function(boost::bind(chaiscript::eval::detail::eval_function,
                                                                                                                   boost::ref(t_ss), this->children.back(),
                                                                                                                   t_param_names, _1), static_cast<int>(numparams), this->children.back(),
                                                                                                       l_annotation, guard)), ti)), function_name);

            }
          }
          catch (const exception::reserved_word_error &e) {
            throw exception::eval_error("Reserved word used as method name '" + e.word() + "'");
          } catch (const exception::name_conflict_error &e) {
            throw exception::eval_error("Method redefined '" + e.name() + "'");
          }
          return Boxed_Value();
        }

    };

    struct Attr_Decl_AST_Node : public AST_Node {
      public:
        Attr_Decl_AST_Node(const std::string &t_ast_node_text = "", const boost::shared_ptr<std::string> &t_fname=boost::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          AST_Node(t_ast_node_text, AST_Node_Type::Attr_Decl, t_fname, t_start_line, t_start_col, t_end_line, t_end_col) { }
        virtual ~Attr_Decl_AST_Node() {}
        virtual Boxed_Value eval_internal(chaiscript::detail::Dispatch_Engine &t_ss){
          try {

            t_ss.add(Proxy_Function
                (new dispatch::detail::Dynamic_Object_Function(
                     this->children[0]->text,
                     fun(boost::function<Boxed_Value (dispatch::Dynamic_Object &)>(boost::bind(&dispatch::Dynamic_Object::get_attr, 
                                                                                   _1,
                                                                                   this->children[1]->text
                                                                                   )))
                     )
                ), this->children[1]->text);

          }
          catch (const exception::reserved_word_error &) {
            throw exception::eval_error("Reserved word used as attribute '" + this->children[1]->text + "'");
          } catch (const exception::name_conflict_error &e) {
            throw exception::eval_error("Attribute redefined '" + e.name() + "'");
          }
          return Boxed_Value();
        }

    };

    struct Shift_AST_Node : public Binary_Operator_AST_Node {
      public:
        Shift_AST_Node(const std::string &t_ast_node_text = "", const boost::shared_ptr<std::string> &t_fname=boost::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          Binary_Operator_AST_Node(t_ast_node_text, AST_Node_Type::Shift, t_fname, t_start_line, t_start_col, t_end_line, t_end_col) { }
        virtual ~Shift_AST_Node() {}
    };

    struct Equality_AST_Node : public Binary_Operator_AST_Node {
      public:
        Equality_AST_Node(const std::string &t_ast_node_text = "", const boost::shared_ptr<std::string> &t_fname=boost::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          Binary_Operator_AST_Node(t_ast_node_text, AST_Node_Type::Equality, t_fname, t_start_line, t_start_col, t_end_line, t_end_col) { }
        virtual ~Equality_AST_Node() {}
    };

    struct Bitwise_And_AST_Node : public Binary_Operator_AST_Node {
      public:
        Bitwise_And_AST_Node(const std::string &t_ast_node_text = "", const boost::shared_ptr<std::string> &t_fname=boost::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          Binary_Operator_AST_Node(t_ast_node_text, AST_Node_Type::Bitwise_And, t_fname, t_start_line, t_start_col, t_end_line, t_end_col) { }
        virtual ~Bitwise_And_AST_Node() {}
    };

    struct Bitwise_Xor_AST_Node : public Binary_Operator_AST_Node {
      public:
        Bitwise_Xor_AST_Node(const std::string &t_ast_node_text = "", const boost::shared_ptr<std::string> &t_fname=boost::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          Binary_Operator_AST_Node(t_ast_node_text, AST_Node_Type::Bitwise_Xor, t_fname, t_start_line, t_start_col, t_end_line, t_end_col) { }
        virtual ~Bitwise_Xor_AST_Node() {}
    };

    struct Bitwise_Or_AST_Node : public Binary_Operator_AST_Node {
      public:
        Bitwise_Or_AST_Node(const std::string &t_ast_node_text = "", const boost::shared_ptr<std::string> &t_fname=boost::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          Binary_Operator_AST_Node(t_ast_node_text, AST_Node_Type::Bitwise_Or, t_fname, t_start_line, t_start_col, t_end_line, t_end_col) { }
        virtual ~Bitwise_Or_AST_Node() {}
    };

    struct Logical_And_AST_Node : public AST_Node {
      public:
        Logical_And_AST_Node(const std::string &t_ast_node_text = "", const boost::shared_ptr<std::string> &t_fname=boost::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          AST_Node(t_ast_node_text, AST_Node_Type::Logical_And, t_fname, t_start_line, t_start_col, t_end_line, t_end_col) { }
        virtual ~Logical_And_AST_Node() {}
        virtual Boxed_Value eval_internal(chaiscript::detail::Dispatch_Engine &t_ss){
          Boxed_Value retval = this->children[0]->eval(t_ss);

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
                retval = this->children[i+1]->eval(t_ss);
              }
              else {
                retval = Boxed_Value(false);
              }
            }
          }
          return retval;
        }

        virtual std::string pretty_print() const
        {
          return "(" + AST_Node::pretty_print() + ")";
        }
    };

    struct Logical_Or_AST_Node : public AST_Node {
      public:
        Logical_Or_AST_Node(const std::string &t_ast_node_text = "", const boost::shared_ptr<std::string> &t_fname=boost::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          AST_Node(t_ast_node_text, AST_Node_Type::Logical_Or, t_fname, t_start_line, t_start_col, t_end_line, t_end_col) { }
        virtual ~Logical_Or_AST_Node() {}
        virtual Boxed_Value eval_internal(chaiscript::detail::Dispatch_Engine &t_ss){
          Boxed_Value retval;

          retval = this->children[0]->eval(t_ss);

          if (this->children.size() > 1) {
            for (size_t i = 1; i < this->children.size(); i += 2) {
              bool lhs = boxed_cast<bool>(retval);

              if (lhs) {
                retval = Boxed_Value(true);
              }
              else {
                retval = this->children[i+1]->eval(t_ss);
              }
            }
          }
          return retval;
        }

        virtual std::string pretty_print() const
        {
          return "(" + AST_Node::pretty_print() + ")";
        }

    };
  }


}
#endif /* CHAISCRIPT_EVAL_HPP_ */
