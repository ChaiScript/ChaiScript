// This file is distributed under the BSD License.
// See "license.txt" for details.
// Copyright 2009-2012, Jonathan Turner (jonathan@emptycrate.com)
// Copyright 2009-2016, Jason Turner (jason@emptycrate.com)
// http://www.chaiscript.com

#ifndef CHAISCRIPT_EVAL_HPP_
#define CHAISCRIPT_EVAL_HPP_

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
      static Boxed_Value eval_function(chaiscript::detail::Dispatch_Engine &t_ss, const AST_NodePtr &t_node, const std::vector<std::string> &t_param_names, const std::vector<Boxed_Value> &t_vals, const std::map<std::string, Boxed_Value> *t_locals=nullptr) {
        chaiscript::detail::Dispatch_State state(t_ss);

        const Boxed_Value *thisobj = [&]() -> const Boxed_Value *{
          auto &stack = t_ss.get_stack_data(state.stack_holder()).back();
          if (!stack.empty() && stack.back().first == "__this") {
            return &stack.back().second;
          } else if (!t_vals.empty()) {
            return &t_vals[0];
          } else {
            return nullptr;
          }
        }();

        chaiscript::eval::detail::Stack_Push_Pop tpp(state);
        if (thisobj) state.add_object("this", *thisobj);

        if (t_locals) {
          for (const auto &local : *t_locals) {
            state.add_object(local.first, local.second);
          }
        }

        for (size_t i = 0; i < t_param_names.size(); ++i) {
          if (t_param_names[i] != "this") {
            state.add_object(t_param_names[i], t_vals[i]);
          }
        }

        try {
          return t_node->eval(state);
        } catch (detail::Return_Value &rv) {
          return std::move(rv.retval);
        } 
      }
    }

    struct Binary_Operator_AST_Node : AST_Node {
      public:
        Binary_Operator_AST_Node(const std::string &t_oper, Parse_Location t_loc, std::vector<AST_NodePtr> t_children) :
          AST_Node(t_oper, AST_Node_Type::Binary, std::move(t_loc), std::move(t_children)),
          m_oper(Operators::to_operator(t_oper))
      { }

        virtual ~Binary_Operator_AST_Node() {}
        virtual Boxed_Value eval_internal(const chaiscript::detail::Dispatch_State &t_ss) const CHAISCRIPT_OVERRIDE {
          auto lhs = this->children[0]->eval(t_ss);
          auto rhs = this->children[1]->eval(t_ss);
          return do_oper(t_ss, m_oper, text, lhs, rhs);
        }

        virtual std::string pretty_print() const CHAISCRIPT_OVERRIDE 
        {
          return "(" + this->children[0]->pretty_print() + " " + text + " " + this->children[1]->pretty_print() + ")";
        }

      protected:
        Boxed_Value do_oper(const chaiscript::detail::Dispatch_State &t_ss, 
            Operators::Opers t_oper, const std::string &t_oper_string, const Boxed_Value &t_lhs, const Boxed_Value &t_rhs) const
        {
          try {
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
              chaiscript::eval::detail::Function_Push_Pop fpp(t_ss);
              fpp.save_params({t_lhs, t_rhs});
              return t_ss->call_function(t_oper_string, m_loc, {t_lhs, t_rhs}, t_ss.conversions());
            }
          }
          catch(const exception::dispatch_error &e){
            throw exception::eval_error("Can not find appropriate '" + t_oper_string + "' operator.", e.parameters, e.functions, false, *t_ss);
          }
        }

      private:
        Operators::Opers m_oper;
        mutable std::atomic_uint_fast32_t m_loc;
    };

    struct Int_AST_Node : public AST_Node {
      public:
        Int_AST_Node(std::string t_ast_node_text, Parse_Location t_loc, Boxed_Value t_bv) :
          AST_Node(std::move(t_ast_node_text), AST_Node_Type::Int, std::move(t_loc)), 
          m_value(std::move(t_bv)) { assert(text != ""); }
        virtual ~Int_AST_Node() {}
        virtual Boxed_Value eval_internal(const chaiscript::detail::Dispatch_State &) const CHAISCRIPT_OVERRIDE{
          return m_value;
        }

      private:
        Boxed_Value m_value;

    };

    struct Float_AST_Node : public AST_Node {
      public:
        Float_AST_Node(std::string t_ast_node_text, Parse_Location t_loc, Boxed_Value t_bv) :
          AST_Node(std::move(t_ast_node_text), AST_Node_Type::Float, std::move(t_loc)),
          m_value(std::move(t_bv)) { }
        virtual ~Float_AST_Node() {}
        virtual Boxed_Value eval_internal(const chaiscript::detail::Dispatch_State &) const CHAISCRIPT_OVERRIDE{
          return m_value;
        }

      private:
        Boxed_Value m_value;

    };

    struct Id_AST_Node : public AST_Node {
      public:
        Id_AST_Node(const std::string &t_ast_node_text, Parse_Location t_loc) :
          AST_Node(t_ast_node_text, AST_Node_Type::Id, std::move(t_loc)),
          m_value(get_value(t_ast_node_text)), m_loc(0)
      { }

        virtual ~Id_AST_Node() {}
        virtual Boxed_Value eval_internal(const chaiscript::detail::Dispatch_State &t_ss) const CHAISCRIPT_OVERRIDE {
          if (!m_value.is_undef())
          {
            return m_value;
          } else {
            try {
              return t_ss->get_object(this->text, m_loc);
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
          } else if (t_text == "false") {
            return const_var(false);
          } else if (t_text == "Infinity") {
            return const_var(std::numeric_limits<double>::infinity());
          } else if (t_text == "NaN") {
            return const_var(std::numeric_limits<double>::quiet_NaN());
          } else if (t_text == "_") {
            return Boxed_Value(std::make_shared<dispatch::Placeholder_Object>());
          } else {
            return Boxed_Value();
          }
        }

        Boxed_Value m_value;

        mutable std::atomic_uint_fast32_t m_loc;
    };

    struct Char_AST_Node : public AST_Node {
      public:
        Char_AST_Node(std::string t_ast_node_text, Parse_Location t_loc) :
          AST_Node(std::move(t_ast_node_text), AST_Node_Type::Char, std::move(t_loc)) { }
        virtual ~Char_AST_Node() {}
    };

    struct Str_AST_Node : public AST_Node {
      public:
        Str_AST_Node(std::string t_ast_node_text, Parse_Location t_loc) :
          AST_Node(std::move(t_ast_node_text), AST_Node_Type::Str, std::move(t_loc)) { }
        virtual ~Str_AST_Node() {}
    };

    struct Eol_AST_Node : public AST_Node {
      public:
        Eol_AST_Node(std::string t_ast_node_text, Parse_Location t_loc) :
          AST_Node(std::move(t_ast_node_text), AST_Node_Type::Eol, std::move(t_loc)) { }
        virtual ~Eol_AST_Node() {}

        virtual std::string pretty_print() const CHAISCRIPT_OVERRIDE 
        {
          return "\n";
        }
    };


    struct Fun_Call_AST_Node : public AST_Node {
      public:
        Fun_Call_AST_Node(std::string t_ast_node_text, Parse_Location t_loc, std::vector<AST_NodePtr> t_children) :
          AST_Node(std::move(t_ast_node_text), AST_Node_Type::Fun_Call, std::move(t_loc), std::move(t_children)) { }
        virtual ~Fun_Call_AST_Node() {}
        virtual Boxed_Value eval_internal(const chaiscript::detail::Dispatch_State &t_ss) const CHAISCRIPT_OVERRIDE{
          chaiscript::eval::detail::Function_Push_Pop fpp(t_ss);

          std::vector<Boxed_Value> params;

          params.reserve(this->children[1]->children.size());
          for (const auto &child : this->children[1]->children) {
            params.push_back(child->eval(t_ss));
          }

          fpp.save_params(params);

          Boxed_Value fn(this->children[0]->eval(t_ss));

          try {
            return (*t_ss->boxed_cast<const Const_Proxy_Function &>(fn))(params, t_ss.conversions());
          }
          catch(const exception::dispatch_error &e){
            throw exception::eval_error(std::string(e.what()) + " with function '" + this->children[0]->text + "'", e.parameters, e.functions, false, *t_ss);
          }
          catch(const exception::bad_boxed_cast &){
            try {
              Const_Proxy_Function f = t_ss->boxed_cast<const Const_Proxy_Function &>(fn);
              // handle the case where there is only 1 function to try to call and dispatch fails on it
              throw exception::eval_error("Error calling function '" + this->children[0]->text + "'", params, {f}, false, *t_ss);
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



    struct Arg_AST_Node : public AST_Node {
      public:
        Arg_AST_Node(std::string t_ast_node_text, Parse_Location t_loc, std::vector<AST_NodePtr> t_children) :
          AST_Node(std::move(t_ast_node_text), AST_Node_Type::Arg_List, std::move(t_loc), std::move(t_children)) { }
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
        Arg_List_AST_Node(std::string t_ast_node_text, Parse_Location t_loc, std::vector<AST_NodePtr> t_children) :
          AST_Node(std::move(t_ast_node_text), AST_Node_Type::Arg_List, std::move(t_loc), std::move(t_children)) { }
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

        static std::pair<std::string, Type_Info> get_arg_type(const AST_NodePtr &t_node, const chaiscript::detail::Dispatch_State &t_ss) 
        {
          if (t_node->children.size() < 2)
          {
            return std::pair<std::string, Type_Info>();
          } else {
            return std::pair<std::string, Type_Info>(t_node->children[0]->text, t_ss->get_type(t_node->children[0]->text, false));
          }
        }

        static dispatch::Param_Types get_arg_types(const AST_NodePtr &t_node, const chaiscript::detail::Dispatch_State &t_ss) {
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
        Equation_AST_Node(std::string t_ast_node_text, Parse_Location t_loc, std::vector<AST_NodePtr> t_children) :
          AST_Node(std::move(t_ast_node_text), AST_Node_Type::Equation, std::move(t_loc), std::move(t_children)), 
          m_oper(Operators::to_operator(children[1]->text))
        { assert(children.size() == 3); }

        Operators::Opers m_oper;
        mutable std::atomic_uint_fast32_t m_loc;
        mutable std::atomic_uint_fast32_t m_clone_loc;

        virtual ~Equation_AST_Node() {}
        virtual Boxed_Value eval_internal(const chaiscript::detail::Dispatch_State &t_ss) const CHAISCRIPT_OVERRIDE {
          chaiscript::eval::detail::Function_Push_Pop fpp(t_ss);
          Boxed_Value rhs = this->children[2]->eval(t_ss); 
          Boxed_Value lhs = this->children[0]->eval(t_ss);

          if (m_oper != Operators::invalid && lhs.get_type_info().is_arithmetic() &&
              rhs.get_type_info().is_arithmetic())
          {
            try {
              return Boxed_Number::do_oper(m_oper, lhs, rhs);
            } catch (const std::exception &) {
              throw exception::eval_error("Error with unsupported arithmetic assignment operation");
            }
          } else if (m_oper == Operators::assign) {
            if (lhs.is_return_value()) {
              throw exception::eval_error("Error, cannot assign to temporary value.");
            }

            try {

              if (lhs.is_undef()) {
                if (!this->children.empty() && 
                    !this->children[0]->children.empty() 
                    && this->children[0]->children[0]->identifier == AST_Node_Type::Reference)
                {
                  /// \todo This does not handle the case of an unassigned reference variable
                  ///       being assigned outside of its declaration
                  lhs.assign(rhs);
                  lhs.reset_return_value();
                  return rhs;
                } else {
                  if (!rhs.is_return_value())
                  {
                    rhs = t_ss->call_function("clone", m_clone_loc, {rhs}, t_ss.conversions());
                  }
                  rhs.reset_return_value();
                }
              }

              try {
                return t_ss->call_function(this->children[1]->text, m_loc, {std::move(lhs), rhs}, t_ss.conversions());
              }
              catch(const exception::dispatch_error &e){
                throw exception::eval_error("Unable to find appropriate'" + this->children[1]->text + "' operator.", e.parameters, e.functions, false, *t_ss);
              }
            }
            catch(const exception::dispatch_error &e){
              throw exception::eval_error("Missing clone or copy constructor for right hand side of equation", e.parameters, e.functions, false, *t_ss);
            }
          }
          else if (this->children[1]->text == ":=") {
            if (lhs.is_undef() || Boxed_Value::type_match(lhs, rhs)) {
              lhs.assign(rhs);
              lhs.reset_return_value();
            } else {
              throw exception::eval_error("Mismatched types in equation");
            }
          }
          else {
            try {
              return t_ss->call_function(this->children[1]->text, m_loc, {std::move(lhs), rhs}, t_ss.conversions());
            } catch(const exception::dispatch_error &e){
              throw exception::eval_error("Unable to find appropriate'" + this->children[1]->text + "' operator.", e.parameters, e.functions, false, *t_ss);
            }
          }

          return rhs;
        }
    };

    struct Global_Decl_AST_Node : public AST_Node {
      public:
        Global_Decl_AST_Node(std::string t_ast_node_text, Parse_Location t_loc, std::vector<AST_NodePtr> t_children) :
          AST_Node(std::move(t_ast_node_text), AST_Node_Type::Global_Decl, std::move(t_loc), std::move(t_children)) { }
        virtual ~Global_Decl_AST_Node() {}
        virtual Boxed_Value eval_internal(const chaiscript::detail::Dispatch_State &t_ss) const CHAISCRIPT_OVERRIDE {

          const std::string &idname =
            [&]()->const std::string &{
              if (children[0]->identifier == AST_Node_Type::Reference) {
                return children[0]->children[0]->text;
              } else {
                return children[0]->text;
              }
            }();

          try {
            return t_ss->add_global_no_throw(Boxed_Value(), idname);
          }
          catch (const exception::reserved_word_error &) {
            throw exception::eval_error("Reserved word used as global '" + idname + "'");
          }

        }
    };


    struct Var_Decl_AST_Node : public AST_Node {
      public:
        Var_Decl_AST_Node(std::string t_ast_node_text, Parse_Location t_loc, std::vector<AST_NodePtr> t_children) :
          AST_Node(std::move(t_ast_node_text), AST_Node_Type::Var_Decl, std::move(t_loc), std::move(t_children)) { }
        virtual ~Var_Decl_AST_Node() {}
        virtual Boxed_Value eval_internal(const chaiscript::detail::Dispatch_State &t_ss) const CHAISCRIPT_OVERRIDE {
          if (this->children[0]->identifier == AST_Node_Type::Reference)
          {
            return this->children[0]->eval(t_ss);
          } else {
            const std::string &idname = this->children[0]->text;

            try {
              Boxed_Value bv;
              t_ss.add_object(idname, bv);
              return bv;
            }
            catch (const exception::reserved_word_error &) {
              throw exception::eval_error("Reserved word used as variable '" + idname + "'");
            } catch (const exception::name_conflict_error &e) {
              throw exception::eval_error("Variable redefined '" + e.name() + "'");
            }
          }

        }

        virtual std::string pretty_print() const CHAISCRIPT_OVERRIDE 
        {
          return "var " + this->children[0]->text;
        }

    };


    struct Array_Call_AST_Node : public AST_Node {
      public:
        Array_Call_AST_Node(std::string t_ast_node_text, Parse_Location t_loc, std::vector<AST_NodePtr> t_children) :
          AST_Node(std::move(t_ast_node_text), AST_Node_Type::Array_Call, std::move(t_loc), std::move(t_children)) { }
        virtual ~Array_Call_AST_Node() {}
        virtual Boxed_Value eval_internal(const chaiscript::detail::Dispatch_State &t_ss) const CHAISCRIPT_OVERRIDE{
          chaiscript::eval::detail::Function_Push_Pop fpp(t_ss);

          std::vector<Boxed_Value> params{children[0]->eval(t_ss), children[1]->eval(t_ss)};

          try {
            fpp.save_params(params);
            return t_ss->call_function("[]", m_loc, params, t_ss.conversions());
          }
          catch(const exception::dispatch_error &e){
            throw exception::eval_error("Can not find appropriate array lookup operator '[]'.", e.parameters, e.functions, false, *t_ss );
          }

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

        mutable std::atomic_uint_fast32_t m_loc;
    };

    struct Dot_Access_AST_Node : public AST_Node {
      public:
        Dot_Access_AST_Node(std::string t_ast_node_text, Parse_Location t_loc, std::vector<AST_NodePtr> t_children) :
          AST_Node(std::move(t_ast_node_text), AST_Node_Type::Dot_Access, std::move(t_loc), std::move(t_children)),
          m_fun_name(
              ((children[2]->identifier == AST_Node_Type::Fun_Call) || (children[2]->identifier == AST_Node_Type::Array_Call))?
              children[2]->children[0]->text:children[2]->text) { }

        virtual ~Dot_Access_AST_Node() {}
        virtual Boxed_Value eval_internal(const chaiscript::detail::Dispatch_State &t_ss) const CHAISCRIPT_OVERRIDE{
          chaiscript::eval::detail::Function_Push_Pop fpp(t_ss);


          Boxed_Value retval = children[0]->eval(t_ss);
          std::vector<Boxed_Value> params{retval};

          bool has_function_params = false;
          if (children[2]->children.size() > 1) {
            has_function_params = true;
            for (const auto &child : children[2]->children[1]->children) {
              params.push_back(child->eval(t_ss));
            }
          }

          fpp.save_params(params);

          try {
            retval = t_ss->call_member(m_fun_name, m_loc, std::move(params), has_function_params, t_ss.conversions());
          }
          catch(const exception::dispatch_error &e){
            if (e.functions.empty())
            {
              throw exception::eval_error("'" + m_fun_name + "' is not a function.");
            } else {
              throw exception::eval_error(std::string(e.what()) + " for function '" + m_fun_name + "'", e.parameters, e.functions, true, *t_ss);
            }
          }
          catch(detail::Return_Value &rv) {
            retval = std::move(rv.retval);
          }

          if (this->children[2]->identifier == AST_Node_Type::Array_Call) {
            try {
              retval = t_ss->call_function("[]", m_array_loc, {retval, this->children[2]->children[1]->eval(t_ss)}, t_ss.conversions());
            }
            catch(const exception::dispatch_error &e){
              throw exception::eval_error("Can not find appropriate array lookup operator '[]'.", e.parameters, e.functions, true, *t_ss);
            }
          }

          return retval;
        }

      private:
        mutable std::atomic_uint_fast32_t m_loc;
        mutable std::atomic_uint_fast32_t m_array_loc;
        std::string m_fun_name;
    };

    struct Quoted_String_AST_Node : public AST_Node {
      public:
        Quoted_String_AST_Node(std::string t_ast_node_text, Parse_Location t_loc) :
          AST_Node(std::move(t_ast_node_text), AST_Node_Type::Quoted_String, std::move(t_loc)),
          m_value(const_var(text)) { }
        virtual ~Quoted_String_AST_Node() {}

        virtual Boxed_Value eval_internal(const chaiscript::detail::Dispatch_State &) const CHAISCRIPT_OVERRIDE {
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
        Single_Quoted_String_AST_Node(std::string t_ast_node_text, Parse_Location t_loc) :
          AST_Node(std::move(t_ast_node_text), AST_Node_Type::Single_Quoted_String, std::move(t_loc)),
          m_value(const_var(char(text.at(0)))) { }

        virtual ~Single_Quoted_String_AST_Node() {}
        virtual Boxed_Value eval_internal(const chaiscript::detail::Dispatch_State &) const CHAISCRIPT_OVERRIDE{
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
        Lambda_AST_Node(std::string t_ast_node_text, Parse_Location t_loc, std::vector<AST_NodePtr> t_children) :
          AST_Node(t_ast_node_text, AST_Node_Type::Lambda, std::move(t_loc), std::move(t_children)),
          m_param_names(Arg_List_AST_Node::get_arg_names(children[1])) { }

        virtual ~Lambda_AST_Node() {}

        virtual Boxed_Value eval_internal(const chaiscript::detail::Dispatch_State &t_ss) const CHAISCRIPT_OVERRIDE{

          const auto captures = [&]()->std::map<std::string, Boxed_Value>{
            std::map<std::string, Boxed_Value> named_captures;
            for (const auto &capture : children[0]->children) {
              named_captures.insert(std::make_pair(capture->children[0]->text, capture->children[0]->eval(t_ss)));
            }
            return named_captures;
          }();

          const auto numparams = this->children[1]->children.size();
          const auto param_names = m_param_names;
          const auto param_types = Arg_List_AST_Node::get_arg_types(this->children[1], t_ss);

          const auto &lambda_node = this->children.back();
          std::reference_wrapper<chaiscript::detail::Dispatch_Engine> engine(*t_ss);

          return Boxed_Value(
              dispatch::make_dynamic_proxy_function(
                  [engine, lambda_node, param_names, captures](const std::vector<Boxed_Value> &t_params)
                  {
                    return detail::eval_function(engine, lambda_node, param_names, t_params, &captures);
                  },
                  static_cast<int>(numparams), lambda_node, param_types
                )
              );
        }

      private:
        std::vector<std::string> m_param_names;

    };

    struct Block_AST_Node : public AST_Node {
      public:
        Block_AST_Node(std::string t_ast_node_text, Parse_Location t_loc, std::vector<AST_NodePtr> t_children) :
          AST_Node(std::move(t_ast_node_text), AST_Node_Type::Block, std::move(t_loc), std::move(t_children)) { }
        virtual ~Block_AST_Node() {}

        virtual Boxed_Value eval_internal(const chaiscript::detail::Dispatch_State &t_ss) const CHAISCRIPT_OVERRIDE{
          chaiscript::eval::detail::Scope_Push_Pop spp(t_ss);

          const auto num_children = children.size();
          for (size_t i = 0; i < num_children-1; ++i) {
            children[i]->eval(t_ss);
          }
          return children.back()->eval(t_ss);

        }
    };

    struct Def_AST_Node : public AST_Node {
      public:
        Def_AST_Node(std::string t_ast_node_text, Parse_Location t_loc, std::vector<AST_NodePtr> t_children) :
          AST_Node(std::move(t_ast_node_text), AST_Node_Type::Def, std::move(t_loc), std::move(t_children)) { }

        virtual ~Def_AST_Node() {}
        virtual Boxed_Value eval_internal(const chaiscript::detail::Dispatch_State &t_ss) const CHAISCRIPT_OVERRIDE{
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

          std::reference_wrapper<chaiscript::detail::Dispatch_Engine> engine(*t_ss);
          std::shared_ptr<dispatch::Proxy_Function_Base> guard;
          if (guardnode) {
            guard = dispatch::make_dynamic_proxy_function(
                [engine, guardnode, t_param_names](const std::vector<Boxed_Value> &t_params)
                {
                  return detail::eval_function(engine, guardnode, t_param_names, t_params);
                },
                static_cast<int>(numparams), guardnode);
          }

          try {
            const std::string & l_function_name = this->children[0]->text;
            const std::string & l_annotation = this->annotation?this->annotation->text:"";
            const auto & func_node = this->children.back();
            t_ss->add(
                dispatch::make_dynamic_proxy_function(
                  [engine, guardnode, func_node, t_param_names](const std::vector<Boxed_Value> &t_params)
                  {
                    return detail::eval_function(engine, func_node, t_param_names, t_params);
                  },
                  static_cast<int>(numparams), this->children.back(),
                  param_types, l_annotation, guard), l_function_name);
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
        While_AST_Node(std::string t_ast_node_text, Parse_Location t_loc, std::vector<AST_NodePtr> t_children) :
          AST_Node(std::move(t_ast_node_text), AST_Node_Type::While, std::move(t_loc), std::move(t_children)) { }
        virtual ~While_AST_Node() {}
        virtual Boxed_Value eval_internal(const chaiscript::detail::Dispatch_State &t_ss) const CHAISCRIPT_OVERRIDE {
          chaiscript::eval::detail::Scope_Push_Pop spp(t_ss);

          try {
            while (get_bool_condition(this->children[0]->eval(t_ss))) {
              try {
                this->children[1]->eval(t_ss);
              } catch (detail::Continue_Loop &) {
                // we got a continue exception, which means all of the remaining 
                // loop implementation is skipped and we just need to continue to
                // the next condition test
              }
            } 
          } catch (detail::Break_Loop &) {
            // loop was broken intentionally
          }

          return Boxed_Value();
        }
    };

    struct Class_AST_Node : public AST_Node {
      public:
        Class_AST_Node(std::string t_ast_node_text, Parse_Location t_loc, std::vector<AST_NodePtr> t_children) :
          AST_Node(std::move(t_ast_node_text), AST_Node_Type::Class, std::move(t_loc), std::move(t_children)) { }
        virtual ~Class_AST_Node() {}
        virtual Boxed_Value eval_internal(const chaiscript::detail::Dispatch_State &t_ss) const CHAISCRIPT_OVERRIDE {
          chaiscript::eval::detail::Scope_Push_Pop spp(t_ss);

          /// \todo do this better
          // put class name in current scope so it can be looked up by the attrs and methods
          t_ss.add_object("_current_class_name", const_var(children[0]->text));

          children[1]->eval(t_ss);

          return Boxed_Value();
        }
    };

    struct Ternary_Cond_AST_Node : public AST_Node {
      public:
        Ternary_Cond_AST_Node(std::string t_ast_node_text, Parse_Location t_loc, std::vector<AST_NodePtr> t_children) :
          AST_Node(std::move(t_ast_node_text), AST_Node_Type::If, std::move(t_loc), std::move(t_children)) 
          { assert(children.size() == 3); }
        virtual ~Ternary_Cond_AST_Node() {}
        virtual Boxed_Value eval_internal(const chaiscript::detail::Dispatch_State &t_ss) const CHAISCRIPT_OVERRIDE {
          if (get_bool_condition(children[0]->eval(t_ss))) {
            return children[1]->eval(t_ss);
          }
          else {
            return children[2]->eval(t_ss);
          }
        }

    };

    struct If_AST_Node : public AST_Node {
      public:
        If_AST_Node(std::string t_ast_node_text, Parse_Location t_loc, std::vector<AST_NodePtr> t_children) :
          AST_Node(std::move(t_ast_node_text), AST_Node_Type::If, std::move(t_loc), std::move(t_children)) { }
        virtual ~If_AST_Node() {}
        virtual Boxed_Value eval_internal(const chaiscript::detail::Dispatch_State &t_ss) const CHAISCRIPT_OVERRIDE{

          if (get_bool_condition(children[0]->eval(t_ss))) {
            return children[1]->eval(t_ss);
          } else {
            if (children.size() > 2) {
              size_t i = 2;
              while (i < children.size()) {
                if (children[i]->text == "else") {
                  return children[i+1]->eval(t_ss);
                }
                else if (children[i]->text == "else if") {
                  if (get_bool_condition(children[i+1]->eval(t_ss))) {
                    return children[i+2]->eval(t_ss);
                  }
                }
                i += 3;
              }
            }
          }

          return Boxed_Value();
        }

    };

    struct For_AST_Node : public AST_Node {
      public:
        For_AST_Node(std::string t_ast_node_text, Parse_Location t_loc, std::vector<AST_NodePtr> t_children) :
          AST_Node(std::move(t_ast_node_text), AST_Node_Type::For, std::move(t_loc), std::move(t_children)) 
          { assert(children.size() == 4); }
        virtual ~For_AST_Node() {}

        virtual Boxed_Value eval_internal(const chaiscript::detail::Dispatch_State &t_ss) const CHAISCRIPT_OVERRIDE{
          chaiscript::eval::detail::Scope_Push_Pop spp(t_ss);

          try {
            for (
                children[0]->eval(t_ss);
                get_bool_condition(children[1]->eval(t_ss));
                children[2]->eval(t_ss)
                ) {
              try {
                // Body of Loop
                children[3]->eval(t_ss);
              } catch (detail::Continue_Loop &) {
                // we got a continue exception, which means all of the remaining 
                // loop implementation is skipped and we just need to continue to
                // the next iteration step
              }
            }
          } catch (detail::Break_Loop &) {
            // loop broken
          }

          return Boxed_Value();
        }

    };

    struct Switch_AST_Node : public AST_Node {
      public:
        Switch_AST_Node(std::string t_ast_node_text, Parse_Location t_loc, std::vector<AST_NodePtr> t_children) :
          AST_Node(std::move(t_ast_node_text), AST_Node_Type::Switch, std::move(t_loc), std::move(t_children)) { }
        virtual ~Switch_AST_Node() {}
        virtual Boxed_Value eval_internal(const chaiscript::detail::Dispatch_State &t_ss) const CHAISCRIPT_OVERRIDE {
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
                  if (hasMatched || boxed_cast<bool>(t_ss->call_function("==", m_loc, {match_value, this->children[currentCase]->children[0]->eval(t_ss)}, t_ss.conversions()))) {
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
                hasMatched = true;
              }
            }
            catch (detail::Break_Loop &) {
              breaking = true;
            }
            ++currentCase;
          }
          return Boxed_Value();
        }

        mutable std::atomic_uint_fast32_t m_loc;
    };

    struct Case_AST_Node : public AST_Node {
      public:
        Case_AST_Node(std::string t_ast_node_text, Parse_Location t_loc, std::vector<AST_NodePtr> t_children) :
          AST_Node(std::move(t_ast_node_text), AST_Node_Type::Case, std::move(t_loc), std::move(t_children)) 
        { assert(children.size() == 2); /* how many children does it have? */ }

        virtual ~Case_AST_Node() {}
        virtual Boxed_Value eval_internal(const chaiscript::detail::Dispatch_State &t_ss) const CHAISCRIPT_OVERRIDE {
          chaiscript::eval::detail::Scope_Push_Pop spp(t_ss);

          children[1]->eval(t_ss);

          return Boxed_Value();
        }
    };
   
    struct Default_AST_Node : public AST_Node {
      public:
        Default_AST_Node(std::string t_ast_node_text, Parse_Location t_loc, std::vector<AST_NodePtr> t_children) :
          AST_Node(std::move(t_ast_node_text), AST_Node_Type::Default, std::move(t_loc), std::move(t_children))
        { assert(children.size() == 1); }
        virtual ~Default_AST_Node() {}
        virtual Boxed_Value eval_internal(const chaiscript::detail::Dispatch_State &t_ss) const CHAISCRIPT_OVERRIDE {
          chaiscript::eval::detail::Scope_Push_Pop spp(t_ss);

          children[0]->eval(t_ss);

          return Boxed_Value();
        }
    };


    struct Inline_Array_AST_Node : public AST_Node {
      public:
        Inline_Array_AST_Node(std::string t_ast_node_text, Parse_Location t_loc, std::vector<AST_NodePtr> t_children) :
          AST_Node(std::move(t_ast_node_text), AST_Node_Type::Inline_Array, std::move(t_loc), std::move(t_children)) { }
        virtual ~Inline_Array_AST_Node() {}
        virtual Boxed_Value eval_internal(const chaiscript::detail::Dispatch_State &t_ss) const CHAISCRIPT_OVERRIDE {
          try {
            std::vector<Boxed_Value> vec;
            if (!children.empty()) {
              vec.reserve(children[0]->children.size());
              for (const auto &child : children[0]->children) {
                auto obj = child->eval(t_ss);
                if (!obj.is_return_value()) {
                  vec.push_back(t_ss->call_function("clone", m_loc, {obj}, t_ss.conversions()));
                } else {
                  vec.push_back(std::move(obj));
                }
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

        mutable std::atomic_uint_fast32_t m_loc;
    };

    struct Inline_Map_AST_Node : public AST_Node {
      public:
        Inline_Map_AST_Node(std::string t_ast_node_text, Parse_Location t_loc, std::vector<AST_NodePtr> t_children) :
          AST_Node(std::move(t_ast_node_text), AST_Node_Type::Inline_Map, std::move(t_loc), std::move(t_children)) { }
        virtual ~Inline_Map_AST_Node() {}
        virtual Boxed_Value eval_internal(const chaiscript::detail::Dispatch_State &t_ss) const CHAISCRIPT_OVERRIDE{
          try {
            std::map<std::string, Boxed_Value> retval;

            for (const auto &child : children[0]->children) {
              auto obj = child->children[1]->eval(t_ss);
              if (!obj.is_return_value()) {
                obj = t_ss->call_function("clone", m_loc, {obj}, t_ss.conversions());
              }

              retval[t_ss->boxed_cast<std::string>(child->children[0]->eval(t_ss))] = std::move(obj);
            }

            return const_var(std::move(retval));
          }
          catch (const exception::dispatch_error &e) {
            throw exception::eval_error("Can not find appropriate copy constructor or 'clone' while inserting into Map.", e.parameters, e.functions, false, *t_ss);
          }
        }

        mutable std::atomic_uint_fast32_t m_loc;
    };

    struct Return_AST_Node : public AST_Node {
      public:
        Return_AST_Node(std::string t_ast_node_text, Parse_Location t_loc, std::vector<AST_NodePtr> t_children) :
          AST_Node(std::move(t_ast_node_text), AST_Node_Type::Return, std::move(t_loc), std::move(t_children)) { }
        virtual ~Return_AST_Node() {}
        virtual Boxed_Value eval_internal(const chaiscript::detail::Dispatch_State &t_ss) const CHAISCRIPT_OVERRIDE{
          if (!this->children.empty()) {
            throw detail::Return_Value(children[0]->eval(t_ss));
          }
          else {
            throw detail::Return_Value(Boxed_Value());
          }
        }

    };

    struct File_AST_Node : public AST_Node {
      public:
        File_AST_Node(std::string t_ast_node_text, Parse_Location t_loc, std::vector<AST_NodePtr> t_children) :
          AST_Node(std::move(t_ast_node_text), AST_Node_Type::File, std::move(t_loc), std::move(t_children)) { }
        virtual ~File_AST_Node() {}
        virtual Boxed_Value eval_internal(const chaiscript::detail::Dispatch_State &t_ss) const CHAISCRIPT_OVERRIDE {
          try {
            const auto num_children = children.size();

            if (num_children > 0) {
              for (size_t i = 0; i < num_children-1; ++i) {
                children[i]->eval(t_ss);
              }
              return children.back()->eval(t_ss);
            } else {
              return Boxed_Value();
            }
          } catch (const detail::Continue_Loop &) {
            throw exception::eval_error("Unexpected `continue` statement outside of a loop");
          } catch (const detail::Break_Loop &) {
            throw exception::eval_error("Unexpected `break` statement outside of a loop");
          }
        }
    };

    struct Reference_AST_Node : public AST_Node {
      public:
        Reference_AST_Node(std::string t_ast_node_text, Parse_Location t_loc, std::vector<AST_NodePtr> t_children) :
          AST_Node(std::move(t_ast_node_text), AST_Node_Type::Reference, std::move(t_loc), std::move(t_children))
        { assert(children.size() == 1); }

        virtual Boxed_Value eval_internal(const chaiscript::detail::Dispatch_State &t_ss) const CHAISCRIPT_OVERRIDE{
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
        Prefix_AST_Node(std::string t_ast_node_text, Parse_Location t_loc, std::vector<AST_NodePtr> t_children) :
          AST_Node(std::move(t_ast_node_text), AST_Node_Type::Prefix, std::move(t_loc), std::move(t_children)),
          m_oper(Operators::to_operator(children[0]->text, true))
        { }

        virtual ~Prefix_AST_Node() {}
        virtual Boxed_Value eval_internal(const chaiscript::detail::Dispatch_State &t_ss) const CHAISCRIPT_OVERRIDE{
          Boxed_Value bv(children[1]->eval(t_ss));

          try {
            // short circuit arithmetic operations
            if (m_oper != Operators::invalid && m_oper != Operators::bitwise_and && bv.get_type_info().is_arithmetic())
            {
              return Boxed_Number::do_oper(m_oper, bv);
            } else {
              chaiscript::eval::detail::Function_Push_Pop fpp(t_ss);
              fpp.save_params({bv});
              return t_ss->call_function(children[0]->text, m_loc, {std::move(bv)}, t_ss.conversions());
            }
          } catch (const exception::dispatch_error &e) {
            throw exception::eval_error("Error with prefix operator evaluation: '" + children[0]->text + "'", e.parameters, e.functions, false, *t_ss);
          }
        }

      private:
        Operators::Opers m_oper;
        mutable std::atomic_uint_fast32_t m_loc;
    };

    struct Break_AST_Node : public AST_Node {
      public:
        Break_AST_Node(std::string t_ast_node_text, Parse_Location t_loc, std::vector<AST_NodePtr> t_children) :
          AST_Node(std::move(t_ast_node_text), AST_Node_Type::Break, std::move(t_loc), std::move(t_children)) { }
        virtual ~Break_AST_Node() {}
        virtual Boxed_Value eval_internal(const chaiscript::detail::Dispatch_State &) const CHAISCRIPT_OVERRIDE{
          throw detail::Break_Loop();
        }
    };

    struct Continue_AST_Node : public AST_Node {
      public:
        Continue_AST_Node(std::string t_ast_node_text, Parse_Location t_loc, std::vector<AST_NodePtr> t_children) :
          AST_Node(std::move(t_ast_node_text), AST_Node_Type::Continue, std::move(t_loc), std::move(t_children)) { }
        virtual ~Continue_AST_Node() {}
        virtual Boxed_Value eval_internal(const chaiscript::detail::Dispatch_State &) const CHAISCRIPT_OVERRIDE{
          throw detail::Continue_Loop();
        }
    };

    struct Noop_AST_Node : public AST_Node {
      public:
        Noop_AST_Node() :
          AST_Node("", AST_Node_Type::Noop, Parse_Location()),
          m_value(const_var(true))
        { }

        virtual ~Noop_AST_Node() {}
        virtual Boxed_Value eval_internal(const chaiscript::detail::Dispatch_State &) const CHAISCRIPT_OVERRIDE{
          // It's a no-op, that evaluates to "true"
          return m_value;
        }

      private:
        Boxed_Value m_value;
    };

    struct Map_Pair_AST_Node : public AST_Node {
      public:
        Map_Pair_AST_Node(std::string t_ast_node_text, Parse_Location t_loc, std::vector<AST_NodePtr> t_children) :
          AST_Node(std::move(t_ast_node_text), AST_Node_Type::Map_Pair, std::move(t_loc), std::move(t_children)) { }
        virtual ~Map_Pair_AST_Node() {}
    };

    struct Value_Range_AST_Node : public AST_Node {
      public:
        Value_Range_AST_Node(std::string t_ast_node_text, Parse_Location t_loc, std::vector<AST_NodePtr> t_children) :
          AST_Node(std::move(t_ast_node_text), AST_Node_Type::Value_Range, std::move(t_loc), std::move(t_children)) { }
        virtual ~Value_Range_AST_Node() {}
    };

    struct Inline_Range_AST_Node : public AST_Node {
      public:
        Inline_Range_AST_Node(std::string t_ast_node_text, Parse_Location t_loc, std::vector<AST_NodePtr> t_children) :
          AST_Node(std::move(t_ast_node_text), AST_Node_Type::Inline_Range, std::move(t_loc), std::move(t_children)) { }
        virtual ~Inline_Range_AST_Node() {}
        virtual Boxed_Value eval_internal(const chaiscript::detail::Dispatch_State &t_ss) const CHAISCRIPT_OVERRIDE{
          try {
            auto oper1 = children[0]->children[0]->children[0]->eval(t_ss);
            auto oper2 = children[0]->children[0]->children[1]->eval(t_ss);
            return t_ss->call_function("generate_range", m_loc, {oper1, oper2}, t_ss.conversions());
          }
          catch (const exception::dispatch_error &e) {
            throw exception::eval_error("Unable to generate range vector, while calling 'generate_range'", e.parameters, e.functions, false, *t_ss);
          }
        }

        mutable std::atomic_uint_fast32_t m_loc;
    };

    struct Annotation_AST_Node : public AST_Node {
      public:
        Annotation_AST_Node(std::string t_ast_node_text, Parse_Location t_loc) :
          AST_Node(std::move(t_ast_node_text), AST_Node_Type::Annotation, std::move(t_loc)) { }
        virtual ~Annotation_AST_Node() {}
    };

    struct Try_AST_Node : public AST_Node {
      public:
        Try_AST_Node(std::string t_ast_node_text, Parse_Location t_loc, std::vector<AST_NodePtr> t_children) :
          AST_Node(std::move(t_ast_node_text), AST_Node_Type::Try, std::move(t_loc), std::move(t_children)) { }
        virtual ~Try_AST_Node() {}

        Boxed_Value handle_exception(const chaiscript::detail::Dispatch_State &t_ss, const Boxed_Value &t_except) const
        {
          Boxed_Value retval;

          size_t end_point = this->children.size();
          if (this->children.back()->identifier == AST_Node_Type::Finally) {
            assert(end_point > 0);
            end_point = this->children.size() - 1;
          }
          for (size_t i = 1; i < end_point; ++i) {
            chaiscript::eval::detail::Scope_Push_Pop catch_scope(t_ss);
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

        virtual Boxed_Value eval_internal(const chaiscript::detail::Dispatch_State &t_ss) const CHAISCRIPT_OVERRIDE{
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
          catch (const std::runtime_error &e) {
            retval = handle_exception(t_ss, Boxed_Value(std::ref(e)));
          }
          catch (const std::out_of_range &e) {
            retval = handle_exception(t_ss, Boxed_Value(std::ref(e)));
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
        Catch_AST_Node(std::string t_ast_node_text, Parse_Location t_loc, std::vector<AST_NodePtr> t_children) :
          AST_Node(std::move(t_ast_node_text), AST_Node_Type::Catch, std::move(t_loc), std::move(t_children)) { }
        virtual ~Catch_AST_Node() {}
    };

    struct Finally_AST_Node : public AST_Node {
      public:
        Finally_AST_Node(std::string t_ast_node_text, Parse_Location t_loc, std::vector<AST_NodePtr> t_children) :
          AST_Node(std::move(t_ast_node_text), AST_Node_Type::Finally, std::move(t_loc), std::move(t_children)) { }
        virtual ~Finally_AST_Node() {}
    };

    struct Method_AST_Node : public AST_Node {
      public:
        Method_AST_Node(std::string t_ast_node_text, Parse_Location t_loc, std::vector<AST_NodePtr> t_children) :
          AST_Node(std::move(t_ast_node_text), AST_Node_Type::Method, std::move(t_loc), std::move(t_children)) { }
        virtual ~Method_AST_Node() {}
        virtual Boxed_Value eval_internal(const chaiscript::detail::Dispatch_State &t_ss) const CHAISCRIPT_OVERRIDE{

          AST_NodePtr guardnode;

          const auto d = t_ss->get_parent_locals();
          const auto itr = d.find("_current_class_name");
          const auto class_offset = (itr != d.end())?-1:0;
          const std::string & class_name = (itr != d.end())?std::string(boxed_cast<std::string>(itr->second)):this->children[0]->text;

          //The first param of a method is always the implied this ptr.
          std::vector<std::string> t_param_names{"this"};
          dispatch::Param_Types param_types;

          if ((children.size() > static_cast<size_t>(3 + class_offset)) && (children[static_cast<size_t>(2 + class_offset)]->identifier == AST_Node_Type::Arg_List)) {
            auto args = Arg_List_AST_Node::get_arg_names(children[static_cast<size_t>(2 + class_offset)]);
            t_param_names.insert(t_param_names.end(), args.begin(), args.end());
            param_types = Arg_List_AST_Node::get_arg_types(children[static_cast<size_t>(2 + class_offset)], t_ss);

            if (children.size() > static_cast<size_t>(4 + class_offset)) {
              guardnode = children[static_cast<size_t>(3 + class_offset)];
            }
          }
          else {
            //no parameters

            if (children.size() > static_cast<size_t>(3 + class_offset)) {
              guardnode = children[static_cast<size_t>(2 + class_offset)];
            }
          }

          const size_t numparams = t_param_names.size();

          std::shared_ptr<dispatch::Proxy_Function_Base> guard;
          std::reference_wrapper<chaiscript::detail::Dispatch_Engine> engine(*t_ss);
          if (guardnode) {
            guard = dispatch::make_dynamic_proxy_function(
                [engine, t_param_names, guardnode](const std::vector<Boxed_Value> &t_params) {
                  return chaiscript::eval::detail::eval_function(engine, guardnode, t_param_names, t_params);
                }, 
                static_cast<int>(numparams), guardnode);
          }

          try {
            const std::string & l_annotation = annotation?annotation->text:"";
            const std::string & function_name = children[static_cast<size_t>(1 + class_offset)]->text;
            auto node = children.back();

            if (function_name == class_name) {
              param_types.push_front(class_name, Type_Info());

              t_ss->add(
                  std::make_shared<dispatch::detail::Dynamic_Object_Constructor>(class_name,
                    dispatch::make_dynamic_proxy_function(
                        [engine, t_param_names, node](const std::vector<Boxed_Value> &t_params) {
                          return chaiscript::eval::detail::eval_function(engine, node, t_param_names, t_params);
                        },
                        static_cast<int>(numparams), node, param_types, l_annotation, guard
                      )
                    ),
                  function_name);

            } else {
              // if the type is unknown, then this generates a function that looks up the type
              // at runtime. Defining the type first before this is called is better
              auto type = t_ss->get_type(class_name, false);
              param_types.push_front(class_name, type);

              t_ss->add(std::make_shared<dispatch::detail::Dynamic_Object_Function>(class_name,
                    dispatch::make_dynamic_proxy_function(
                      [engine, t_param_names, node](const std::vector<Boxed_Value> &t_params) {
                        return chaiscript::eval::detail::eval_function(engine, node, t_param_names, t_params);
                      },
                      static_cast<int>(numparams), node, param_types, l_annotation, guard), type), 
                  function_name);
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
        Attr_Decl_AST_Node(std::string t_ast_node_text, Parse_Location t_loc, std::vector<AST_NodePtr> t_children) :
          AST_Node(std::move(t_ast_node_text), AST_Node_Type::Attr_Decl, std::move(t_loc), std::move(t_children)) { }
        virtual ~Attr_Decl_AST_Node() {}
        virtual Boxed_Value eval_internal(const chaiscript::detail::Dispatch_State &t_ss) const CHAISCRIPT_OVERRIDE 
        {
          const auto &d = t_ss->get_parent_locals();
          const auto itr = d.find("_current_class_name");
          const auto class_offset = (itr != d.end())?-1:0;
          std::string class_name = (itr != d.end())?std::string(boxed_cast<std::string>(itr->second)):this->children[0]->text;

          try {
            std::string attr_name = this->children[static_cast<size_t>(1 + class_offset)]->text;

            t_ss->add(
                std::make_shared<dispatch::detail::Dynamic_Object_Function>(
                     std::move(class_name),
                     fun([attr_name](dispatch::Dynamic_Object &t_obj) {
                           return t_obj.get_attr(attr_name);
                         }),
                     true

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
        Logical_And_AST_Node(std::string t_ast_node_text, Parse_Location t_loc, std::vector<AST_NodePtr> t_children) :
          AST_Node(std::move(t_ast_node_text), AST_Node_Type::Logical_And, std::move(t_loc), std::move(t_children)) 
        { assert(children.size() == 3); }

        virtual ~Logical_And_AST_Node() {}
        virtual Boxed_Value eval_internal(const chaiscript::detail::Dispatch_State &t_ss) const CHAISCRIPT_OVERRIDE{
          return const_var(get_bool_condition(children[0]->eval(t_ss))
              && get_bool_condition(children[2]->eval(t_ss)));
        }

        virtual std::string pretty_print() const CHAISCRIPT_OVERRIDE
        {
          return "(" + AST_Node::pretty_print() + ")";
        }
    };

    struct Logical_Or_AST_Node : public AST_Node {
      public:
        Logical_Or_AST_Node(std::string t_ast_node_text, Parse_Location t_loc, std::vector<AST_NodePtr> t_children) :
          AST_Node(std::move(t_ast_node_text), AST_Node_Type::Logical_Or, std::move(t_loc), std::move(t_children)) 
        { assert(children.size() == 3); }
        virtual ~Logical_Or_AST_Node() {}
        virtual Boxed_Value eval_internal(const chaiscript::detail::Dispatch_State &t_ss) const CHAISCRIPT_OVERRIDE{
          return const_var(get_bool_condition(children[0]->eval(t_ss))
              || get_bool_condition(children[2]->eval(t_ss)));
        }

        virtual std::string pretty_print() const CHAISCRIPT_OVERRIDE
        {
          return "(" + AST_Node::pretty_print() + ")";
        }

    };
  }


}
#endif /* CHAISCRIPT_EVAL_HPP_ */
