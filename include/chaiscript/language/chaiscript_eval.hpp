// This file is distributed under the BSD License.
// See "license.txt" for details.
// Copyright 2009-2012, Jonathan Turner (jonathan@emptycrate.com)
// Copyright 2009-2015, Jason Turner (jason@emptycrate.com)
// http://www.chaiscript.com

#ifndef CHAISCRIPT_EVAL_HPP_
#define CHAISCRIPT_EVAL_HPP_

#include <assert.h>
#include <cstdlib>
#include <exception>
#include <functional>
#include <limits>
#include <map>
#include <memory>
#include <ostream>
#include <stdexcept>
#include <string>
#include <vector>

#include "../chaiscript_defines.hpp"
#include "../dispatchkit/boxed_cast.hpp"
#include "../dispatchkit/boxed_cast_helper.hpp"
#include "../dispatchkit/boxed_number.hpp"
#include "../dispatchkit/boxed_value.hpp"
#include "../dispatchkit/dispatchkit.hpp"
#include "../dispatchkit/dynamic_object_detail.hpp"
#include "../dispatchkit/proxy_functions.hpp"
#include "../dispatchkit/proxy_functions_detail.hpp"
#include "../dispatchkit/register_function.hpp"
#include "../dispatchkit/type_info.hpp"
#include "chaiscript_algebraic.hpp"
#include "chaiscript_common.hpp"

namespace chaiscript {
namespace exception {
class bad_boxed_cast;
}  // namespace exception
}  // namespace chaiscript

namespace chaiscript
{
  /// \brief Classes and functions that are part of the runtime eval system
  namespace eval
  {
    namespace detail
    {
      /// Helper function that will set up the scope around a function call, including handling the named function parameters
      static Boxed_Value eval_function(chaiscript::detail::Dispatch_Engine &t_ss, const AST_NodePtr &t_node, const std::vector<std::string> &t_param_names, const std::vector<Boxed_Value> &t_vals) {
        chaiscript::eval::detail::Scope_Push_Pop spp(t_ss);

        for (size_t i = 0; i < t_param_names.size(); ++i) {
          t_ss.add_object(t_param_names[i], t_vals[i]);
        }

        try {
          return t_node->eval(t_ss);
        } catch (detail::Return_Value &rv) {
          return rv.retval;
        } 
      }
    }

    struct Binary_Operator_AST_Node : public AST_Node {
      public:
        Binary_Operator_AST_Node(const std::string &t_oper) :
          AST_Node(t_oper, AST_Node_Type::Binary, std::make_shared<std::string>(""), 0, 0, 0, 0),
          m_oper(Operators::to_operator(t_oper))
      { }

        virtual ~Binary_Operator_AST_Node() {}
        virtual Boxed_Value eval_internal(chaiscript::detail::Dispatch_Engine &t_ss) const CHAISCRIPT_OVERRIDE {
          return do_oper(t_ss, m_oper, text,
              this->children[0]->eval(t_ss),
              this->children[1]->eval(t_ss));
        }

        virtual std::string pretty_print() const CHAISCRIPT_OVERRIDE 
        {
          return "(" + this->children[0]->pretty_print() + " " + text + " " + this->children[1]->pretty_print() + ")";
        }

      protected:
        Boxed_Value do_oper(chaiscript::detail::Dispatch_Engine &t_ss, 
            Operators::Opers t_oper, const std::string &t_oper_string, const Boxed_Value &t_lhs, const Boxed_Value &t_rhs) const
        {
          try {
            chaiscript::eval::detail::Function_Push_Pop fpp(t_ss);
            fpp.save_params({t_lhs, t_rhs});

            if (t_oper != Operators::invalid && t_lhs.get_type_info().is_arithmetic() && t_rhs.get_type_info().is_arithmetic())
            {
              // If it's an arithmetic operation we want to short circuit dispatch
              try{
                return Boxed_Number::do_oper(t_oper, t_lhs, t_rhs);
              } catch (const chaiscript::exception::arithmetic_error &) {
                throw;
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

      private:
        Operators::Opers m_oper;
    };

    struct Int_AST_Node : public AST_Node {
      public:
        Int_AST_Node(std::string t_ast_node_text, Boxed_Value t_bv, const std::shared_ptr<std::string> &t_fname, int t_start_line, int t_start_col, int t_end_line, int t_end_col) :
          AST_Node(std::move(t_ast_node_text), AST_Node_Type::Int, t_fname, t_start_line, t_start_col, t_end_line, t_end_col), 
          m_value(std::move(t_bv)) { }
        virtual ~Int_AST_Node() {}
        virtual Boxed_Value eval_internal(chaiscript::detail::Dispatch_Engine &) const CHAISCRIPT_OVERRIDE{
          return m_value;
        }

      private:
        Boxed_Value m_value;

    };

    struct Float_AST_Node : public AST_Node {
      public:
        Float_AST_Node(std::string t_ast_node_text, Boxed_Value t_bv, const std::shared_ptr<std::string> &t_fname, int t_start_line, int t_start_col, int t_end_line, int t_end_col) :
          AST_Node(std::move(t_ast_node_text), AST_Node_Type::Float, t_fname, t_start_line, t_start_col, t_end_line, t_end_col),
          m_value(std::move(t_bv)) { }
        virtual ~Float_AST_Node() {}
        virtual Boxed_Value eval_internal(chaiscript::detail::Dispatch_Engine &) const CHAISCRIPT_OVERRIDE{
          return m_value;
        }

      private:
        Boxed_Value m_value;

    };

    struct Id_AST_Node : public AST_Node {
      public:
        Id_AST_Node(const std::string &t_ast_node_text, const std::shared_ptr<std::string> &t_fname, int t_start_line, int t_start_col, int t_end_line, int t_end_col) :
          AST_Node(t_ast_node_text, AST_Node_Type::Id, t_fname, t_start_line, t_start_col, t_end_line, t_end_col),
          m_value(get_value(t_ast_node_text))
      { }

        virtual ~Id_AST_Node() {}
        virtual Boxed_Value eval_internal(chaiscript::detail::Dispatch_Engine &t_ss) const CHAISCRIPT_OVERRIDE {
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
        Char_AST_Node(std::string t_ast_node_text, const std::shared_ptr<std::string> &t_fname, int t_start_line, int t_start_col, int t_end_line, int t_end_col) :
          AST_Node(std::move(t_ast_node_text), AST_Node_Type::Char, t_fname, t_start_line, t_start_col, t_end_line, t_end_col) { }
        virtual ~Char_AST_Node() {}
    };

    struct Str_AST_Node : public AST_Node {
      public:
        Str_AST_Node(std::string t_ast_node_text, const std::shared_ptr<std::string> &t_fname, int t_start_line, int t_start_col, int t_end_line, int t_end_col) :
          AST_Node(std::move(t_ast_node_text), AST_Node_Type::Str, t_fname, t_start_line, t_start_col, t_end_line, t_end_col) { }
        virtual ~Str_AST_Node() {}
    };

    struct Eol_AST_Node : public AST_Node {
      public:
        Eol_AST_Node(std::string t_ast_node_text, const std::shared_ptr<std::string> &t_fname, int t_start_line, int t_start_col, int t_end_line, int t_end_col) :
          AST_Node(std::move(t_ast_node_text), AST_Node_Type::Eol, t_fname, t_start_line, t_start_col, t_end_line, t_end_col) { }
        virtual ~Eol_AST_Node() {}

        virtual std::string pretty_print() const CHAISCRIPT_OVERRIDE 
        {
          return "\n";
        }
    };

    struct Fun_Call_AST_Node : public AST_Node {
      public:
        Fun_Call_AST_Node(const std::string &t_ast_node_text = "", const std::shared_ptr<std::string> &t_fname=std::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          AST_Node(t_ast_node_text, AST_Node_Type::Fun_Call, t_fname, t_start_line, t_start_col, t_end_line, t_end_col) { }
        virtual ~Fun_Call_AST_Node() {}
        virtual Boxed_Value eval_internal(chaiscript::detail::Dispatch_Engine &t_ss) const CHAISCRIPT_OVERRIDE{
          chaiscript::eval::detail::Function_Push_Pop fpp(t_ss);

          std::vector<Boxed_Value> params;

          if ((this->children.size() > 1)) {
            const AST_Node &first_child(*(this->children[1]));
            if (first_child.identifier == AST_Node_Type::Arg_List) {
              for (const auto &child : first_child.children) {
                params.push_back(child->eval(t_ss));
              }
            }
          }

          fpp.save_params(params);

          Boxed_Value fn(this->children[0]->eval(t_ss));

          try {
            chaiscript::eval::detail::Stack_Push_Pop spp(t_ss);
            return (*t_ss.boxed_cast<const Const_Proxy_Function &>(fn))(params, t_ss.conversions());
          }
          catch(const exception::dispatch_error &e){
            throw exception::eval_error(std::string(e.what()) + " with function '" + this->children[0]->text + "'", e.parameters, e.functions, false, t_ss);
          }
          catch(const exception::bad_boxed_cast &){
            try {
              Const_Proxy_Function f = t_ss.boxed_cast<const Const_Proxy_Function &>(fn);
              // handle the case where there is only 1 function to try to call and dispatch fails on it
              throw exception::eval_error("Error calling function '" + this->children[0]->text + "'", params, {f}, false, t_ss);
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

        virtual std::string pretty_print() const CHAISCRIPT_OVERRIDE 
        {
          std::ostringstream oss;

          int count = 0;
          for (const auto &child : this->children) {
            oss << child->pretty_print();

            if (count == 0)
            {
              oss << "(";
            }
            ++count;
          }

          oss << ")";

          return oss.str();
        }

    };

    /// Used in the context of in-string ${} evals, so that no new scope is created
    struct Inplace_Fun_Call_AST_Node : public AST_Node {
      public:
        Inplace_Fun_Call_AST_Node(const std::string &t_ast_node_text = "", const std::shared_ptr<std::string> &t_fname=std::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          AST_Node(t_ast_node_text, AST_Node_Type::Inplace_Fun_Call, t_fname, t_start_line, t_start_col, t_end_line, t_end_col) { }
        virtual ~Inplace_Fun_Call_AST_Node() {}
        virtual Boxed_Value eval_internal(chaiscript::detail::Dispatch_Engine &t_ss) const CHAISCRIPT_OVERRIDE{
          std::vector<Boxed_Value> params;
          chaiscript::eval::detail::Function_Push_Pop fpp(t_ss);

          if ((this->children.size() > 1) && (this->children[1]->identifier == AST_Node_Type::Arg_List)) {
            for (const auto &child : this->children[1]->children) {
              params.push_back(child->eval(t_ss));
            }
          }

          Const_Proxy_Function fn;

          try {
            Boxed_Value bv = this->children[0]->eval(t_ss);
            try {
              fn = t_ss.boxed_cast<const Const_Proxy_Function &>(bv);
            } catch (const exception::bad_boxed_cast &) {
              throw exception::eval_error("'" + this->children[0]->pretty_print() + "' does not evaluate to a function.");
            }
            return (*fn)(params, t_ss.conversions());
          }
          catch(const exception::dispatch_error &e){
            throw exception::eval_error(std::string(e.what()) + " with function '" + this->children[0]->text + "'", e.parameters, e.functions, false, t_ss);
          }
          catch(const exception::bad_boxed_cast &){
            // handle the case where there is only 1 function to try to call and dispatch fails on it
            throw exception::eval_error("Error calling function '" + this->children[0]->text + "'", params, {fn}, false, t_ss);
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

        virtual std::string pretty_print() const CHAISCRIPT_OVERRIDE 
        {
          std::ostringstream oss;
          int count = 0;
          for (const auto &child : this->children) {
            oss << child->pretty_print();

            if (count == 0)
            {
              oss << "(";
            }
            ++count;
          }

          oss << ")";

          return oss.str();
        }

    };

    struct Arg_AST_Node : public AST_Node {
      public:
        Arg_AST_Node(std::string t_ast_node_text = "", const std::shared_ptr<std::string> &t_fname=std::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          AST_Node(std::move(t_ast_node_text), AST_Node_Type::Arg_List, t_fname, t_start_line, t_start_col, t_end_line, t_end_col) { }
        virtual ~Arg_AST_Node() {}

        virtual std::string pretty_print() const CHAISCRIPT_OVERRIDE 
        {
          std::ostringstream oss;
          for (size_t j = 0; j < this->children.size(); ++j) {
            if (j != 0)
            {
              oss << " ";
            }

            oss << this->children[j]->pretty_print();
          }

          return oss.str();
        }
    };

    struct Arg_List_AST_Node : public AST_Node {
      public:
        Arg_List_AST_Node(std::string t_ast_node_text = "", const std::shared_ptr<std::string> &t_fname=std::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          AST_Node(std::move(t_ast_node_text), AST_Node_Type::Arg_List, t_fname, t_start_line, t_start_col, t_end_line, t_end_col) { }
        virtual ~Arg_List_AST_Node() {}

        virtual std::string pretty_print() const CHAISCRIPT_OVERRIDE 
        {
          std::ostringstream oss;
          for (size_t j = 0; j < this->children.size(); ++j) {
            if (j != 0)
            {
              oss << ", ";
            }

            oss << this->children[j]->pretty_print();
          }

          return oss.str();
        }

        static std::string get_arg_name(const AST_NodePtr &t_node) {
          if (t_node->children.empty())
          {
            return t_node->text;
          } else if (t_node->children.size() == 1) {
            return t_node->children[0]->text;
          } else {
            return t_node->children[1]->text;
          }
        }

        static std::vector<std::string> get_arg_names(const AST_NodePtr &t_node) {
          std::vector<std::string> retval;

          for (const auto &node : t_node->children)
          {
            retval.push_back(get_arg_name(node));
          }

          return retval;
        }

        static std::pair<std::string, Type_Info> get_arg_type(const AST_NodePtr &t_node, chaiscript::detail::Dispatch_Engine &t_ss) 
        {
          if (t_node->children.size() < 2)
          {
            return std::pair<std::string, Type_Info>();
          } else {
            try {
              return std::pair<std::string, Type_Info>(t_node->children[0]->text, t_ss.get_type(t_node->children[0]->text));
            } catch (const std::range_error &) {
              return std::pair<std::string, Type_Info>(t_node->children[0]->text, Type_Info());
            }
          }
        }

        static dispatch::Param_Types get_arg_types(const AST_NodePtr &t_node, chaiscript::detail::Dispatch_Engine &t_ss) {
          std::vector<std::pair<std::string, Type_Info>> retval;

          for (const auto &child : t_node->children)
          {
            retval.push_back(get_arg_type(child, t_ss));
          }

          return dispatch::Param_Types(std::move(retval));
        }
    };

    struct Equation_AST_Node : public AST_Node {
      public:
        Equation_AST_Node(std::string t_ast_node_text = "", const std::shared_ptr<std::string> &t_fname=std::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          AST_Node(std::move(t_ast_node_text), AST_Node_Type::Equation, t_fname, t_start_line, t_start_col, t_end_line, t_end_col)
        {}

        virtual ~Equation_AST_Node() {}
        virtual Boxed_Value eval_internal(chaiscript::detail::Dispatch_Engine &t_ss) const CHAISCRIPT_OVERRIDE {
          chaiscript::eval::detail::Function_Push_Pop fpp(t_ss);
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
                  if (!this->children.empty() && 
                      !this->children[0]->children.empty() 
                      && this->children[0]->children[0]->identifier == AST_Node_Type::Reference)
                  {
                    /// \todo This does not handle the case of an unassigned reference variable
                    ///       being assigned outside of its declaration
                    lhs.assign(retval);
                    return retval;
                  } else {
                    retval = t_ss.call_function("clone", retval);
                  }
                }

                try {
                  retval = t_ss.call_function(this->children[1]->text, std::move(lhs), retval);
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
              if (lhs.is_undef() || Boxed_Value::type_match(lhs, retval)) {
                lhs.assign(retval);
              } else {
                throw exception::eval_error("Mismatched types in equation");
              }
            }
            else {
              try {
                retval = t_ss.call_function(this->children[1]->text, std::move(lhs), retval);
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
        Var_Decl_AST_Node(std::string t_ast_node_text = "", const std::shared_ptr<std::string> &t_fname=std::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          AST_Node(std::move(t_ast_node_text), AST_Node_Type::Var_Decl, t_fname, t_start_line, t_start_col, t_end_line, t_end_col) { }
        virtual ~Var_Decl_AST_Node() {}
        virtual Boxed_Value eval_internal(chaiscript::detail::Dispatch_Engine &t_ss) const CHAISCRIPT_OVERRIDE {
          if (this->children[0]->identifier == AST_Node_Type::Reference)
          {
            return this->children[0]->eval(t_ss);
          } else {
            const std::string &idname = this->children[0]->text;

            Boxed_Value bv;
            try {
              t_ss.add_object(idname, bv);
            }
            catch (const exception::reserved_word_error &) {
              throw exception::eval_error("Reserved word used as variable '" + idname + "'");
            } catch (const exception::name_conflict_error &e) {
              throw exception::eval_error("Variable redefined '" + e.name() + "'");
            }
            return bv;
          }

        }

        virtual std::string pretty_print() const CHAISCRIPT_OVERRIDE 
        {
          return "var " + this->children[0]->text;
        }

    };


    struct Array_Call_AST_Node : public AST_Node {
      public:
        Array_Call_AST_Node(const std::string &t_ast_node_text = "", const std::shared_ptr<std::string> &t_fname=std::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          AST_Node(t_ast_node_text, AST_Node_Type::Array_Call, t_fname, t_start_line, t_start_col, t_end_line, t_end_col) { }
        virtual ~Array_Call_AST_Node() {}
        virtual Boxed_Value eval_internal(chaiscript::detail::Dispatch_Engine &t_ss) const CHAISCRIPT_OVERRIDE{
          chaiscript::eval::detail::Function_Push_Pop fpp(t_ss);

          Boxed_Value retval(this->children[0]->eval(t_ss));
          std::vector<Boxed_Value> params{retval};

          for (size_t i = 1; i < this->children.size(); ++i) {
            try {
              Boxed_Value p1(this->children[i]->eval(t_ss));

              chaiscript::eval::detail::Stack_Push_Pop spp(t_ss);
              params.push_back(p1);
              fpp.save_params(params);
              params.clear();
              retval = t_ss.call_function("[]", retval, std::move(p1));
            }
            catch(const exception::dispatch_error &e){
              throw exception::eval_error("Can not find appropriate array lookup operator '[]'.", e.parameters, e.functions, false, t_ss );
            }
          }

          return retval;
        }

        virtual std::string pretty_print() const CHAISCRIPT_OVERRIDE 
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
        Dot_Access_AST_Node(const std::string &t_ast_node_text = "", const std::shared_ptr<std::string> &t_fname=std::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          AST_Node(t_ast_node_text, AST_Node_Type::Dot_Access, t_fname, t_start_line, t_start_col, t_end_line, t_end_col) { }
        virtual ~Dot_Access_AST_Node() {}
        virtual Boxed_Value eval_internal(chaiscript::detail::Dispatch_Engine &t_ss) const CHAISCRIPT_OVERRIDE{
          chaiscript::eval::detail::Function_Push_Pop fpp(t_ss);
          Boxed_Value retval = this->children[0]->eval(t_ss);

          if (this->children.size() > 1) {
            for (size_t i = 2; i < this->children.size(); i+=2) {
              std::vector<Boxed_Value> params{retval};

              if (this->children[i]->children.size() > 1) {
                for (const auto &child : this->children[i]->children[1]->children) {
                  params.push_back(child->eval(t_ss));
                }
              }

              fpp.save_params(params);

              std::string fun_name;
              if ((this->children[i]->identifier == AST_Node_Type::Fun_Call) || (this->children[i]->identifier == AST_Node_Type::Array_Call)) {
                fun_name = this->children[i]->children[0]->text;
              }
              else {
                fun_name = this->children[i]->text;
              }

              try {
                chaiscript::eval::detail::Stack_Push_Pop spp(t_ss);
                retval = t_ss.call_function(fun_name, std::move(params));
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
        Quoted_String_AST_Node(const std::string &t_ast_node_text = "", const std::shared_ptr<std::string> &t_fname=std::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          AST_Node(t_ast_node_text, AST_Node_Type::Quoted_String, t_fname, t_start_line, t_start_col, t_end_line, t_end_col),
          m_value(const_var(t_ast_node_text)) { }
        virtual ~Quoted_String_AST_Node() {}

        virtual Boxed_Value eval_internal(chaiscript::detail::Dispatch_Engine &) const CHAISCRIPT_OVERRIDE {
          return m_value;
        }

        virtual std::string pretty_print() const CHAISCRIPT_OVERRIDE 
        {
          return "\"" + text + "\"";
        }

      private:
        Boxed_Value m_value;

    };

    struct Single_Quoted_String_AST_Node : public AST_Node {
      public:
        Single_Quoted_String_AST_Node(const std::string &t_ast_node_text = "", const std::shared_ptr<std::string> &t_fname=std::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          AST_Node(t_ast_node_text, AST_Node_Type::Single_Quoted_String, t_fname, t_start_line, t_start_col, t_end_line, t_end_col),
          m_value(const_var(char(t_ast_node_text.at(0)))) { }

        virtual ~Single_Quoted_String_AST_Node() {}
        virtual Boxed_Value eval_internal(chaiscript::detail::Dispatch_Engine &) const CHAISCRIPT_OVERRIDE{
          return m_value;
        }

        virtual std::string pretty_print() const CHAISCRIPT_OVERRIDE 
        {
          return "'" + text + "'";
        }

      private:
        Boxed_Value m_value;
    };

    struct Lambda_AST_Node : public AST_Node {
      public:
        Lambda_AST_Node(const std::string &t_ast_node_text = "", int t_id = AST_Node_Type::Lambda, const std::shared_ptr<std::string> &t_fname=std::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          AST_Node(t_ast_node_text, t_id, t_fname, t_start_line, t_start_col, t_end_line, t_end_col) { }
        virtual ~Lambda_AST_Node() {}

        virtual Boxed_Value eval_internal(chaiscript::detail::Dispatch_Engine &t_ss) const CHAISCRIPT_OVERRIDE{
          std::vector<std::string> t_param_names;

          size_t numparams = 0;

          dispatch::Param_Types param_types;

          if (!this->children.empty() && (this->children[0]->identifier == AST_Node_Type::Arg_List)) {
            numparams = this->children[0]->children.size();
            t_param_names = Arg_List_AST_Node::get_arg_names(this->children[0]);
            param_types = Arg_List_AST_Node::get_arg_types(this->children[0], t_ss);
          }

          const auto &lambda_node = this->children.back();

          return Boxed_Value(Proxy_Function(new dispatch::Dynamic_Proxy_Function(
                [&t_ss, lambda_node, t_param_names](const std::vector<Boxed_Value> &t_params)
                {
                  return detail::eval_function(t_ss, lambda_node, t_param_names, t_params);
                },
                static_cast<int>(numparams), lambda_node, param_types)));
        }

    };

    struct Block_AST_Node : public AST_Node {
      public:
        Block_AST_Node(std::string t_ast_node_text = "", const std::shared_ptr<std::string> &t_fname=std::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          AST_Node(std::move(t_ast_node_text), AST_Node_Type::Block, t_fname, t_start_line, t_start_col, t_end_line, t_end_col) { }
        virtual ~Block_AST_Node() {}

        virtual Boxed_Value eval_internal(chaiscript::detail::Dispatch_Engine &t_ss) const CHAISCRIPT_OVERRIDE{
          const auto num_children = this->children.size();

          chaiscript::eval::detail::Scope_Push_Pop spp(t_ss);

          for (size_t i = 0; i < num_children; ++i) {
            try {
              if (i + 1 < num_children)
              {
                this->children[i]->eval(t_ss);
              } else {
                return this->children[i]->eval(t_ss);
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
        Def_AST_Node(const std::string &t_ast_node_text = "", const std::shared_ptr<std::string> &t_fname=std::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          AST_Node(t_ast_node_text, AST_Node_Type::Def, t_fname, t_start_line, t_start_col, t_end_line, t_end_col) { }
        virtual ~Def_AST_Node() {}
        virtual Boxed_Value eval_internal(chaiscript::detail::Dispatch_Engine &t_ss) const CHAISCRIPT_OVERRIDE{
          std::vector<std::string> t_param_names;
          size_t numparams = 0;
          AST_NodePtr guardnode;

          dispatch::Param_Types param_types;

          if ((this->children.size() > 2) && (this->children[1]->identifier == AST_Node_Type::Arg_List)) {
            numparams = this->children[1]->children.size();
            t_param_names = Arg_List_AST_Node::get_arg_names(this->children[1]);
            param_types = Arg_List_AST_Node::get_arg_types(this->children[1], t_ss);

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

          std::shared_ptr<dispatch::Dynamic_Proxy_Function> guard;
          if (guardnode) {
            guard = std::shared_ptr<dispatch::Dynamic_Proxy_Function>
              (new dispatch::Dynamic_Proxy_Function([&t_ss, guardnode, t_param_names](const std::vector<Boxed_Value> &t_params)
                                                    {
                                                      return detail::eval_function(t_ss, guardnode, t_param_names, t_params);
                                                    }, static_cast<int>(numparams), guardnode));
          }

          try {
            const std::string & l_function_name = this->children[0]->text;
            const std::string & l_annotation = this->annotation?this->annotation->text:"";
            const auto & func_node = this->children.back();
            t_ss.add(Proxy_Function
                (new dispatch::Dynamic_Proxy_Function([&t_ss, guardnode, func_node, t_param_names](const std::vector<Boxed_Value> &t_params)
                                                      {
                                                        return detail::eval_function(t_ss, func_node, t_param_names, t_params);
                                                      }, static_cast<int>(numparams), this->children.back(),
                                                         param_types, l_annotation, guard)), l_function_name);
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
        While_AST_Node(std::string t_ast_node_text = "", const std::shared_ptr<std::string> &t_fname=std::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          AST_Node(std::move(t_ast_node_text), AST_Node_Type::While, t_fname, t_start_line, t_start_col, t_end_line, t_end_col) { }
        virtual ~While_AST_Node() {}
        virtual Boxed_Value eval_internal(chaiscript::detail::Dispatch_Engine &t_ss) const CHAISCRIPT_OVERRIDE {
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

    struct Class_AST_Node : public AST_Node {
      public:
        Class_AST_Node(const std::string &t_ast_node_text = "", const std::shared_ptr<std::string> &t_fname=std::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          AST_Node(t_ast_node_text, AST_Node_Type::Class, t_fname, t_start_line, t_start_col, t_end_line, t_end_col) { }
        virtual ~Class_AST_Node() {}
        virtual Boxed_Value eval_internal(chaiscript::detail::Dispatch_Engine &t_ss) const CHAISCRIPT_OVERRIDE {
          chaiscript::eval::detail::Scope_Push_Pop spp(t_ss);

          // put class name in current scope so it can be looked up by the attrs and methods
          t_ss.add_object("_current_class_name", const_var(this->children[0]->text));

          this->children[1]->eval(t_ss);

          return Boxed_Value();
        }
    };

    struct Ternary_Cond_AST_Node : public AST_Node {
      public:
        Ternary_Cond_AST_Node(const std::string &t_ast_node_text = "", const std::shared_ptr<std::string> &t_fname=std::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          AST_Node(t_ast_node_text, AST_Node_Type::If, t_fname, t_start_line, t_start_col, t_end_line, t_end_col) { }
        virtual ~Ternary_Cond_AST_Node() {}
        virtual Boxed_Value eval_internal(chaiscript::detail::Dispatch_Engine &t_ss) const CHAISCRIPT_OVERRIDE{
          bool cond = false;
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
        If_AST_Node(const std::string &t_ast_node_text = "", const std::shared_ptr<std::string> &t_fname=std::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          AST_Node(t_ast_node_text, AST_Node_Type::If, t_fname, t_start_line, t_start_col, t_end_line, t_end_col) { }
        virtual ~If_AST_Node() {}
        virtual Boxed_Value eval_internal(chaiscript::detail::Dispatch_Engine &t_ss) const CHAISCRIPT_OVERRIDE{
          bool cond = false;
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
                i += 3;
              }
            }
          }

          return const_var(false);
        }

    };

    struct For_AST_Node : public AST_Node {
      public:
        For_AST_Node(std::string t_ast_node_text = "", const std::shared_ptr<std::string> &t_fname=std::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          AST_Node(std::move(t_ast_node_text), AST_Node_Type::For, t_fname, t_start_line, t_start_col, t_end_line, t_end_col) { }
        virtual ~For_AST_Node() {}

        virtual Boxed_Value eval_internal(chaiscript::detail::Dispatch_Engine &t_ss) const CHAISCRIPT_OVERRIDE{
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
        Switch_AST_Node(std::string t_ast_node_text = "", const std::shared_ptr<std::string> &t_fname=std::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          AST_Node(std::move(t_ast_node_text), AST_Node_Type::Switch, t_fname, t_start_line, t_start_col, t_end_line, t_end_col) { }
        virtual ~Switch_AST_Node() {}
        virtual Boxed_Value eval_internal(chaiscript::detail::Dispatch_Engine &t_ss) const CHAISCRIPT_OVERRIDE {
          bool breaking = false;
          size_t currentCase = 1;
          bool hasMatched = false;

          chaiscript::eval::detail::Scope_Push_Pop spp(t_ss);

          Boxed_Value match_value(this->children[0]->eval(t_ss));

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
        Case_AST_Node(std::string t_ast_node_text = "", const std::shared_ptr<std::string> &t_fname=std::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          AST_Node(std::move(t_ast_node_text), AST_Node_Type::Case, t_fname, t_start_line, t_start_col, t_end_line, t_end_col) { }
        virtual ~Case_AST_Node() {}
        virtual Boxed_Value eval_internal(chaiscript::detail::Dispatch_Engine &t_ss) const CHAISCRIPT_OVERRIDE {
          chaiscript::eval::detail::Scope_Push_Pop spp(t_ss);

          this->children[1]->eval(t_ss);

          return Boxed_Value();
        }
    };
   
    struct Default_AST_Node : public AST_Node {
      public:
        Default_AST_Node(std::string t_ast_node_text = "", const std::shared_ptr<std::string> &t_fname=std::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          AST_Node(std::move(t_ast_node_text), AST_Node_Type::Default, t_fname, t_start_line, t_start_col, t_end_line, t_end_col) { }
        virtual ~Default_AST_Node() {}
        virtual Boxed_Value eval_internal(chaiscript::detail::Dispatch_Engine &t_ss) const CHAISCRIPT_OVERRIDE {
          chaiscript::eval::detail::Scope_Push_Pop spp(t_ss);

          this->children[0]->eval(t_ss);

          return Boxed_Value();
        }
    };


    struct Inline_Array_AST_Node : public AST_Node {
      public:
        Inline_Array_AST_Node(std::string t_ast_node_text = "", const std::shared_ptr<std::string> &t_fname=std::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          AST_Node(std::move(t_ast_node_text), AST_Node_Type::Inline_Array, t_fname, t_start_line, t_start_col, t_end_line, t_end_col) { }
        virtual ~Inline_Array_AST_Node() {}
        virtual Boxed_Value eval_internal(chaiscript::detail::Dispatch_Engine &t_ss) const CHAISCRIPT_OVERRIDE {
          try {
            std::vector<Boxed_Value> vec;
            if (!this->children.empty()) {
              for (const auto &child : this->children[0]->children) {
                vec.push_back(t_ss.call_function("clone", child->eval(t_ss)));
              }
            }
            return const_var(std::move(vec));
          }
          catch (const exception::dispatch_error &) {
            throw exception::eval_error("Can not find appropriate 'clone' or copy constructor for vector elements");
          }

        }

        virtual std::string pretty_print() const CHAISCRIPT_OVERRIDE
        {
          return "[" + AST_Node::pretty_print() + "]";
        }
    };

    struct Inline_Map_AST_Node : public AST_Node {
      public:
        Inline_Map_AST_Node(std::string t_ast_node_text = "", const std::shared_ptr<std::string> &t_fname=std::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          AST_Node(std::move(t_ast_node_text), AST_Node_Type::Inline_Map, t_fname, t_start_line, t_start_col, t_end_line, t_end_col) { }
        virtual ~Inline_Map_AST_Node() {}
        virtual Boxed_Value eval_internal(chaiscript::detail::Dispatch_Engine &t_ss) const CHAISCRIPT_OVERRIDE{
          try {
            std::map<std::string, Boxed_Value> retval;

            for (const auto &child : this->children[0]->children) {
              Boxed_Value bv = t_ss.call_function("clone", child->children[1]->eval(t_ss));
              retval[t_ss.boxed_cast<std::string>(child->children[0]->eval(t_ss))] = std::move(bv);
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
        Return_AST_Node(std::string t_ast_node_text = "", const std::shared_ptr<std::string> &t_fname=std::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          AST_Node(std::move(t_ast_node_text), AST_Node_Type::Return, t_fname, t_start_line, t_start_col, t_end_line, t_end_col) { }
        virtual ~Return_AST_Node() {}
        virtual Boxed_Value eval_internal(chaiscript::detail::Dispatch_Engine &t_ss) const CHAISCRIPT_OVERRIDE{
          if (!this->children.empty()) {
            throw detail::Return_Value(this->children[0]->eval(t_ss));
          }
          else {
            throw detail::Return_Value(Boxed_Value());
          }
        }

    };

    struct File_AST_Node : public AST_Node {
      public:
        File_AST_Node(std::string t_ast_node_text = "", const std::shared_ptr<std::string> &t_fname=std::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          AST_Node(std::move(t_ast_node_text), AST_Node_Type::File, t_fname, t_start_line, t_start_col, t_end_line, t_end_col) { }
        virtual ~File_AST_Node() {}
        virtual Boxed_Value eval_internal(chaiscript::detail::Dispatch_Engine &t_ss) const CHAISCRIPT_OVERRIDE {
          const size_t size = this->children.size(); 
          for (size_t i = 0; i < size; ++i) {
            Boxed_Value retval(this->children[i]->eval(t_ss));
            if (i + 1 == size) {
              return retval;
            }
          }
          return Boxed_Value();
        }
    };

    struct Reference_AST_Node : public AST_Node {
      public:
        Reference_AST_Node(std::string t_ast_node_text = "", int t_id = AST_Node_Type::Reference, const std::shared_ptr<std::string> &t_fname=std::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          AST_Node(std::move(t_ast_node_text), t_id, t_fname, t_start_line, t_start_col, t_end_line, t_end_col)
        { }

        virtual Boxed_Value eval_internal(chaiscript::detail::Dispatch_Engine &t_ss) const CHAISCRIPT_OVERRIDE{
          try {
            Boxed_Value bv;
            t_ss.add_object(this->children[0]->text, bv);
            return bv;
          }
          catch (const exception::reserved_word_error &) {
            throw exception::eval_error("Reserved word used as variable '" + this->children[0]->text + "'");
          }
        }

        virtual ~Reference_AST_Node() {}
    };

    struct Prefix_AST_Node : public AST_Node {
      public:
        Prefix_AST_Node(Operators::Opers t_oper) :
          AST_Node("", AST_Node_Type::Prefix, std::make_shared<std::string>(""), 0, 0, 0, 0),
          m_oper(t_oper)
        { }

        virtual ~Prefix_AST_Node() {}
        virtual Boxed_Value eval_internal(chaiscript::detail::Dispatch_Engine &t_ss) const CHAISCRIPT_OVERRIDE{
          chaiscript::eval::detail::Function_Push_Pop fpp(t_ss);
          Boxed_Value bv(this->children[1]->eval(t_ss));

          try {
            // short circuit arithmetic operations
            if (m_oper != Operators::invalid && bv.get_type_info().is_arithmetic())
            {
              return Boxed_Number::do_oper(m_oper, std::move(bv));
            } else {
              chaiscript::eval::detail::Stack_Push_Pop spp(t_ss);
              fpp.save_params({bv});
              return t_ss.call_function(this->children[0]->text, std::move(bv));
            }
          } catch (const exception::dispatch_error &e) {
            throw exception::eval_error("Error with prefix operator evaluation: '" + children[0]->text + "'", e.parameters, e.functions, false, t_ss);
          }
        }

      private:
        Operators::Opers m_oper;
    };

    struct Break_AST_Node : public AST_Node {
      public:
        Break_AST_Node(const std::string &t_ast_node_text = "", const std::shared_ptr<std::string> &t_fname=std::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          AST_Node(t_ast_node_text, AST_Node_Type::Break, t_fname, t_start_line, t_start_col, t_end_line, t_end_col) { }
        virtual ~Break_AST_Node() {}
        virtual Boxed_Value eval_internal(chaiscript::detail::Dispatch_Engine &) const CHAISCRIPT_OVERRIDE{
          throw detail::Break_Loop();
        }
    };

    struct Continue_AST_Node : public AST_Node {
      public:
        Continue_AST_Node(const std::string &t_ast_node_text = "", const std::shared_ptr<std::string> &t_fname=std::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          AST_Node(t_ast_node_text, AST_Node_Type::Continue, t_fname, t_start_line, t_start_col, t_end_line, t_end_col) { }
        virtual ~Continue_AST_Node() {}
        virtual Boxed_Value eval_internal(chaiscript::detail::Dispatch_Engine &) const CHAISCRIPT_OVERRIDE{
          throw detail::Continue_Loop();
        }
    };

    struct Noop_AST_Node : public AST_Node {
      public:
        Noop_AST_Node(const std::string &t_ast_node_text = "", const std::shared_ptr<std::string> &t_fname=std::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          AST_Node(t_ast_node_text, AST_Node_Type::Noop, t_fname, t_start_line, t_start_col, t_end_line, t_end_col),
          m_value(const_var(true))
        { }

        virtual ~Noop_AST_Node() {}
        virtual Boxed_Value eval_internal(chaiscript::detail::Dispatch_Engine &) const CHAISCRIPT_OVERRIDE{
          // It's a no-op, that evaluates to "true"
          return m_value;
        }

      private:
        Boxed_Value m_value;
    };

    struct Map_Pair_AST_Node : public AST_Node {
      public:
        Map_Pair_AST_Node(const std::string &t_ast_node_text = "", const std::shared_ptr<std::string> &t_fname=std::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          AST_Node(t_ast_node_text, AST_Node_Type::Map_Pair, t_fname, t_start_line, t_start_col, t_end_line, t_end_col) { }
        virtual ~Map_Pair_AST_Node() {}
    };

    struct Value_Range_AST_Node : public AST_Node {
      public:
        Value_Range_AST_Node(const std::string &t_ast_node_text = "", const std::shared_ptr<std::string> &t_fname=std::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          AST_Node(t_ast_node_text, AST_Node_Type::Value_Range, t_fname, t_start_line, t_start_col, t_end_line, t_end_col) { }
        virtual ~Value_Range_AST_Node() {}
    };

    struct Inline_Range_AST_Node : public AST_Node {
      public:
        Inline_Range_AST_Node(const std::string &t_ast_node_text = "", const std::shared_ptr<std::string> &t_fname=std::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          AST_Node(t_ast_node_text, AST_Node_Type::Inline_Range, t_fname, t_start_line, t_start_col, t_end_line, t_end_col) { }
        virtual ~Inline_Range_AST_Node() {}
        virtual Boxed_Value eval_internal(chaiscript::detail::Dispatch_Engine &t_ss) const CHAISCRIPT_OVERRIDE{
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
        Annotation_AST_Node(const std::string &t_ast_node_text = "", const std::shared_ptr<std::string> &t_fname=std::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          AST_Node(t_ast_node_text, AST_Node_Type::Annotation, t_fname, t_start_line, t_start_col, t_end_line, t_end_col) { }
        virtual ~Annotation_AST_Node() {}
    };

    struct Try_AST_Node : public AST_Node {
      public:
        Try_AST_Node(const std::string &t_ast_node_text = "", const std::shared_ptr<std::string> &t_fname=std::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          AST_Node(t_ast_node_text, AST_Node_Type::Try, t_fname, t_start_line, t_start_col, t_end_line, t_end_col) { }
        virtual ~Try_AST_Node() {}

        Boxed_Value handle_exception(chaiscript::detail::Dispatch_Engine &t_ss, const Boxed_Value &t_except) const
        {
          Boxed_Value retval;

          size_t end_point = this->children.size();
          if (this->children.back()->identifier == AST_Node_Type::Finally) {
            assert(end_point > 0);
            end_point = this->children.size() - 1;
          }
          for (size_t i = 1; i < end_point; ++i) {
            chaiscript::eval::detail::Scope_Push_Pop catchscope(t_ss);
            AST_NodePtr catch_block = this->children[i];

            if (catch_block->children.size() == 1) {
              //No variable capture, no guards
              retval = catch_block->children[0]->eval(t_ss);
              break;
            } else if (catch_block->children.size() == 2 || catch_block->children.size() == 3) {
              const auto name = Arg_List_AST_Node::get_arg_name(catch_block->children[0]);

              if (dispatch::Param_Types(
                    std::vector<std::pair<std::string, Type_Info>>{Arg_List_AST_Node::get_arg_type(catch_block->children[0], t_ss)}
                    ).match(std::vector<Boxed_Value>{t_except}, t_ss.conversions()))
              {
                t_ss.add_object(name, t_except);

                if (catch_block->children.size() == 2) {
                  //Variable capture, no guards
                  retval = catch_block->children[1]->eval(t_ss);
                  break;
                }
                else if (catch_block->children.size() == 3) {
                  //Variable capture, guards

                  bool guard = false;
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
              }
            }
            else {
              if (this->children.back()->identifier == AST_Node_Type::Finally) {
                this->children.back()->children[0]->eval(t_ss);
              }
              throw exception::eval_error("Internal error: catch block size unrecognized");
            }
          }

          return retval;
        }

        virtual Boxed_Value eval_internal(chaiscript::detail::Dispatch_Engine &t_ss) const CHAISCRIPT_OVERRIDE{
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
            retval = handle_exception(t_ss, Boxed_Value(std::ref(e)));

          }
          catch (Boxed_Value &e) {
            retval = handle_exception(t_ss, e);
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
        Catch_AST_Node(const std::string &t_ast_node_text = "", const std::shared_ptr<std::string> &t_fname=std::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          AST_Node(t_ast_node_text, AST_Node_Type::Catch, t_fname, t_start_line, t_start_col, t_end_line, t_end_col) { }
        virtual ~Catch_AST_Node() {}
    };

    struct Finally_AST_Node : public AST_Node {
      public:
        Finally_AST_Node(const std::string &t_ast_node_text = "", const std::shared_ptr<std::string> &t_fname=std::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          AST_Node(t_ast_node_text, AST_Node_Type::Finally, t_fname, t_start_line, t_start_col, t_end_line, t_end_col) { }
        virtual ~Finally_AST_Node() {}
    };

    struct Method_AST_Node : public AST_Node {
      public:
        Method_AST_Node(const std::string &t_ast_node_text = "", const std::shared_ptr<std::string> &t_fname=std::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          AST_Node(t_ast_node_text, AST_Node_Type::Method, t_fname, t_start_line, t_start_col, t_end_line, t_end_col) { }
        virtual ~Method_AST_Node() {}
        virtual Boxed_Value eval_internal(chaiscript::detail::Dispatch_Engine &t_ss) const CHAISCRIPT_OVERRIDE{

          AST_NodePtr guardnode;

          const auto d = t_ss.get_parent_locals();
          const auto itr = d.find("_current_class_name");
          const auto class_offset = (itr != d.end())?-1:0;
          const std::string & class_name = (itr != d.end())?std::string(boxed_cast<std::string>(itr->second)):this->children[0]->text;

          //The first param of a method is always the implied this ptr.
          std::vector<std::string> t_param_names{"this"};
          dispatch::Param_Types param_types;

          if ((this->children.size() > static_cast<size_t>(3 + class_offset)) && (this->children[static_cast<size_t>(2 + class_offset)]->identifier == AST_Node_Type::Arg_List)) {
            auto args = Arg_List_AST_Node::get_arg_names(this->children[static_cast<size_t>(2 + class_offset)]);
            t_param_names.insert(t_param_names.end(), args.begin(), args.end());
            param_types = Arg_List_AST_Node::get_arg_types(this->children[static_cast<size_t>(2 + class_offset)], t_ss);

            if (this->children.size() > static_cast<size_t>(4 + class_offset)) {
              guardnode = this->children[static_cast<size_t>(3 + class_offset)];
            }
          }
          else {
            //no parameters

            if (this->children.size() > static_cast<size_t>(3 + class_offset)) {
              guardnode = this->children[static_cast<size_t>(2 + class_offset)];
            }
          }

          const size_t numparams = t_param_names.size();

          std::shared_ptr<dispatch::Dynamic_Proxy_Function> guard;
          if (guardnode) {
            guard = std::make_shared<dispatch::Dynamic_Proxy_Function>
              (std::bind(chaiscript::eval::detail::eval_function,
                         std::ref(t_ss), guardnode,
                         t_param_names, std::placeholders::_1), static_cast<int>(numparams), guardnode);
          }

          try {
            const std::string & l_annotation = this->annotation?this->annotation->text:"";

            const std::string & function_name = this->children[static_cast<size_t>(1 + class_offset)]->text;

            if (function_name == class_name) {
              param_types.push_front(class_name, Type_Info());
              t_ss.add(std::make_shared<dispatch::detail::Dynamic_Object_Constructor>(class_name, std::make_shared<dispatch::Dynamic_Proxy_Function>(std::bind(chaiscript::eval::detail::eval_function,
                        std::ref(t_ss), this->children.back(), t_param_names, std::placeholders::_1), 
                      static_cast<int>(numparams), this->children.back(), param_types, l_annotation, guard)), 
                  function_name);

            }
            else {
              try {
                // Do know type name (if this line fails, the catch block is called and the 
                // other version is called, with no Type_Info object known)
                auto type = t_ss.get_type(class_name);
                param_types.push_front(class_name, type);

                t_ss.add(
                    std::make_shared<dispatch::detail::Dynamic_Object_Function>(class_name, 
                      std::make_shared<dispatch::Dynamic_Proxy_Function>(std::bind(chaiscript::eval::detail::eval_function,
                                                                         std::ref(t_ss), this->children.back(),
                                                                         t_param_names, std::placeholders::_1), static_cast<int>(numparams), this->children.back(),
                                                               param_types, l_annotation, guard), type), function_name);
              } catch (const std::range_error &) {
                param_types.push_front(class_name, Type_Info());
                // Do not know type name
                t_ss.add(
                    std::make_shared<dispatch::detail::Dynamic_Object_Function>(class_name, 
                         std::make_shared<dispatch::Dynamic_Proxy_Function>(std::bind(chaiscript::eval::detail::eval_function,
                                                                         std::ref(t_ss), this->children.back(),
                                                                         t_param_names, std::placeholders::_1), static_cast<int>(numparams), this->children.back(),
                                                               param_types, l_annotation, guard)), function_name);
              }
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
        Attr_Decl_AST_Node(const std::string &t_ast_node_text = "", const std::shared_ptr<std::string> &t_fname=std::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          AST_Node(t_ast_node_text, AST_Node_Type::Attr_Decl, t_fname, t_start_line, t_start_col, t_end_line, t_end_col) { }
        virtual ~Attr_Decl_AST_Node() {}
        virtual Boxed_Value eval_internal(chaiscript::detail::Dispatch_Engine &t_ss) const CHAISCRIPT_OVERRIDE 
        {
          const auto &d = t_ss.get_parent_locals();
          const auto itr = d.find("_current_class_name");
          const auto class_offset = (itr != d.end())?-1:0;
          std::string class_name = (itr != d.end())?std::string(boxed_cast<std::string>(itr->second)):this->children[0]->text;

          try {
            t_ss.add(
                std::make_shared<dispatch::detail::Dynamic_Object_Function>(
                     std::move(class_name),
                     fun(std::function<Boxed_Value (dispatch::Dynamic_Object &)>(std::bind(&dispatch::Dynamic_Object::get_attr, 
                                                                                   std::placeholders::_1,
                                                                                   this->children[static_cast<size_t>(1 + class_offset)]->text
                                                                                   ))
                     )
                ), this->children[static_cast<size_t>(1 + class_offset)]->text);

          }
          catch (const exception::reserved_word_error &) {
            throw exception::eval_error("Reserved word used as attribute '" + this->children[static_cast<size_t>(1 + class_offset)]->text + "'");
          } catch (const exception::name_conflict_error &e) {
            throw exception::eval_error("Attribute redefined '" + e.name() + "'");
          }
          return Boxed_Value();
        }

    };


    struct Logical_And_AST_Node : public AST_Node {
      public:
        Logical_And_AST_Node(const std::string &t_ast_node_text = "", const std::shared_ptr<std::string> &t_fname=std::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          AST_Node(t_ast_node_text, AST_Node_Type::Logical_And, t_fname, t_start_line, t_start_col, t_end_line, t_end_col) { }
        virtual ~Logical_And_AST_Node() {}
        virtual Boxed_Value eval_internal(chaiscript::detail::Dispatch_Engine &t_ss) const CHAISCRIPT_OVERRIDE{
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

        virtual std::string pretty_print() const CHAISCRIPT_OVERRIDE
        {
          return "(" + AST_Node::pretty_print() + ")";
        }
    };

    struct Logical_Or_AST_Node : public AST_Node {
      public:
        Logical_Or_AST_Node(const std::string &t_ast_node_text = "", const std::shared_ptr<std::string> &t_fname=std::shared_ptr<std::string>(), int t_start_line = 0, int t_start_col = 0, int t_end_line = 0, int t_end_col = 0) :
          AST_Node(t_ast_node_text, AST_Node_Type::Logical_Or, t_fname, t_start_line, t_start_col, t_end_line, t_end_col) { }
        virtual ~Logical_Or_AST_Node() {}
        virtual Boxed_Value eval_internal(chaiscript::detail::Dispatch_Engine &t_ss) const CHAISCRIPT_OVERRIDE{
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

        virtual std::string pretty_print() const CHAISCRIPT_OVERRIDE
        {
          return "(" + AST_Node::pretty_print() + ")";
        }

    };
  }


}
#endif /* CHAISCRIPT_EVAL_HPP_ */
