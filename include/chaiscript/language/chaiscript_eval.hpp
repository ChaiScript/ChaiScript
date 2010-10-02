// This file is distributed under the BSD License.
// See "license.txt" for details.
// Copyright 2009-2010, Jonathan Turner (jonathan@emptycrate.com)
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
  const Boxed_Value eval_function (Eval_System &ss, const AST_NodePtr &node, const std::vector<std::string> &param_names, const std::vector<Boxed_Value> &vals) {
    ss.new_scope();

    for (unsigned int i = 0; i < param_names.size(); ++i) {
      ss.add_object(param_names[i], vals[i]);
    }

    try {
      Boxed_Value retval(node->eval(ss));
      ss.pop_scope();
      return retval;
    } catch (const Return_Value &rv) {
      ss.pop_scope();
      return rv.retval;
    } catch (Eval_Error &ee) {
      ee.call_stack.push_back(node);
      ss.pop_scope();
      throw;
    } catch (...) {
      ss.pop_scope();
      throw;
    }
  }

  /**
   * Evaluates the top-level file node
   */
  Boxed_Value File_AST_Node::eval(Dispatch_Engine &ss) {
    const unsigned int size = this->children.size(); 
    for (unsigned int i = 0; i < size; ++i) {
      try {
        const Boxed_Value &retval = this->children[i]->eval(ss);
        if (i + 1 == size) {
          return retval;
        }
      }
      catch (Eval_Error &ee) {
        ee.call_stack.push_back(this->children[i]);
        throw;
      }
    }
    return Boxed_Value();
  }

  /**
   * Evaluates a variable or function name identifier
   */
  Boxed_Value Id_AST_Node::eval(Dispatch_Engine &ss) {
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
        return ss.get_object(this->text);
      }
      catch (std::exception &) {
        throw Eval_Error("Can not find object: " + this->text);
      }
    }
  }

  /**
   * Evaluates a floating point number
   */
  Boxed_Value Float_AST_Node::eval(Dispatch_Engine &) {
    return const_var(double(atof(this->text.c_str())));
  }

  /**
   * Evaluates an integer
   */
  Boxed_Value Int_AST_Node::eval(Dispatch_Engine &) {
    return const_var(int(atoi(this->text.c_str())));
  }

  /**
   * Evaluates a quoted string
   */
  Boxed_Value Quoted_String_AST_Node::eval(Dispatch_Engine &) {
    return const_var(this->text);
  }

  /**
   * Evaluates a char group
   */

  Boxed_Value Single_Quoted_String_AST_Node::eval(Dispatch_Engine &) {
    return const_var(char(this->text[0]));
  }

  /**
   * Evaluates a string of equations in reverse order so that the right-most side has precedence
   */
  Boxed_Value Equation_AST_Node::eval(Dispatch_Engine &ss) {
    Boxed_Value retval;
    try {
      retval = this->children.back()->eval(ss); 
    }
    catch (Eval_Error &ee) {
      ee.call_stack.push_back(this->children.back());
      throw;
    }
    
    if (this->children.size() > 1) {
      for (int i = this->children.size()-3; i >= 0; i -= 2) {
        if (this->children[i+1]->text == "=") {
          try {
            Boxed_Value lhs = this->children[i]->eval(ss);

            try {
              if (lhs.is_undef()) {
                retval = ss.call_function("clone", retval);
                retval.clear_dependencies();
              }

              try {
                retval = ss.call_function(this->children[i+1]->text, lhs, retval);
              }
              catch(const dispatch_error &){
                throw Eval_Error(std::string("Mismatched types in equation") + (lhs.is_const()?", lhs is const.":"."));
              }
            }
            catch(const dispatch_error &){
              throw Eval_Error("Can not clone right hand side of equation");
            }
          }
          catch(Eval_Error &ee) {
            ee.call_stack.push_back(this->children[i]);
            throw;
          }
        }
        else if (this->children[i+1]->text == ":=") {
          try {
            Boxed_Value lhs = this->children[i]->eval(ss);
            if (lhs.is_undef() || type_match(lhs, retval)) {
              lhs.assign(retval);
            }
            else {
              throw Eval_Error("Mismatched types in equation");
            }
          }
          catch (Eval_Error &ee) {
            ee.call_stack.push_back(this->children[i]);
            throw;
          }
        }
        else {
          try {
            retval = ss.call_function(this->children[i+1]->text, this->children[i]->eval(ss), retval);
          }
          catch(const dispatch_error &){
            throw Eval_Error("Can not find appropriate '" + this->children[i+1]->text + "'");
          }
          catch(Eval_Error &ee) {
            ee.call_stack.push_back(this->children[i]);
            throw;
          }
        }
      }
    }
    return retval;
  }

  /**
   * Evaluates a variable declaration
   */
  Boxed_Value Var_Decl_AST_Node::eval(Dispatch_Engine &ss) {
    try {
      ss.add_object(this->children[0]->text, Boxed_Value());
    }
    catch (reserved_word_error &) {
      throw Eval_Error("Reserved word used as variable '" + this->children[0]->text + "'");
    }
    return ss.get_object(this->children[0]->text);
  }

  /**
   * Evaluates an attribute declaration
   */
  Boxed_Value Attr_Decl_AST_Node::eval(Dispatch_Engine &ss) {
    try {
      ss.add(fun(boost::function<Boxed_Value (Dynamic_Object &)>(boost::bind(&Dynamic_Object_Attribute::func, this->children[0]->text,
                                                                             this->children[1]->text, _1))), this->children[1]->text);

    }
    catch (reserved_word_error &) {
      throw Eval_Error("Reserved word used as attribute '" + this->children[1]->text + "'");
    }
    return Boxed_Value();
  }

  /**
   * Evaluates binary boolean operators.  Respects short-circuiting rules.
   */
  Boxed_Value Logical_And_AST_Node::eval(Dispatch_Engine &ss) {
    Boxed_Value retval;
    try {
      retval = this->children[0]->eval(ss);
    }
    catch (Eval_Error &ee) {
      ee.call_stack.push_back(this->children[0]);
      throw;
    }

    if (this->children.size() > 1) {
      for (size_t i = 1; i < this->children.size(); i += 2) {
        bool lhs;
        try {
          lhs = boxed_cast<bool>(retval);
        }
        catch (const bad_boxed_cast &) {
          throw Eval_Error("Condition not boolean");
        }
        if (lhs) {
          try {
            retval = this->children[i+1]->eval(ss);
          }
          catch (Eval_Error &ee) {
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

  Boxed_Value Logical_Or_AST_Node::eval(Dispatch_Engine &ss) {
    Boxed_Value retval;

    try {
      retval = this->children[0]->eval(ss);
    }
    catch (Eval_Error &ee) {
      ee.call_stack.push_back(this->children[0]);
      throw;
    }

    if (this->children.size() > 1) {
      for (size_t i = 1; i < this->children.size(); i += 2) {
        bool lhs;
        try {
          lhs = boxed_cast<bool>(retval);
        }
        catch (const bad_boxed_cast &) {
          throw Eval_Error("Condition not boolean");
        }
        if (lhs) {
          retval = Boxed_Value(true);
        }
        else {
          try {
            retval = this->children[i+1]->eval(ss);
            }
          catch (Eval_Error &ee) {
            ee.call_stack.push_back(this->children[i+1]);
            throw;
          }
        }
      }
    }
    return retval;
  }

  /**
   * Evaluates comparison, additions, and multiplications and their relatives
   */
  Boxed_Value Binary_Operator_AST_Node::eval(Dispatch_Engine &ss) {
    Boxed_Value retval;

    try {
      retval = this->children[0]->eval(ss);
    }
    catch (Eval_Error &ee) {
      ee.call_stack.push_back(this->children[0]);
      throw;
    }

    for (size_t i = 1; i < this->children.size(); i += 2) {
      try {
        retval = ss.call_function(this->children[i]->text, retval, this->children[i+1]->eval(ss));
      }
      catch(const dispatch_error &){
        throw Eval_Error("Can not find appropriate '" + this->children[i]->text + "'");
      }
      catch(Eval_Error &ee) {
        ee.call_stack.push_back(this->children[i+1]);
        throw;
      }
    }

    return retval;
  }

  /**
   * Evaluates an array lookup
   */
  Boxed_Value Array_Call_AST_Node::eval(Dispatch_Engine &ss) {
    Boxed_Value retval;

    try {
      retval = this->children[0]->eval(ss);
    }
    catch (Eval_Error &ee) {
      ee.call_stack.push_back(this->children[0]);
      throw;
    }

    for (size_t i = 1; i < this->children.size(); ++i) {
      try {
        retval = ss.call_function("[]", retval, this->children[i]->eval(ss));
      }
      catch(std::out_of_range &) {
        throw Eval_Error("Out of bounds exception");
      }
      catch(const dispatch_error &){
        throw Eval_Error("Can not find appropriate array lookup '[]' ");
      }
      catch(Eval_Error &ee) {
        ee.call_stack.push_back(this->children[i]);
        throw;
      }
    }

    return retval;
  }

  /**
   * Evaluates any unary prefix
   */
  Boxed_Value Prefix_AST_Node::eval(Dispatch_Engine &ss) {
    try {
      return ss.call_function(this->children[0]->text, this->children[1]->eval(ss));
    }
    catch(std::exception &){
      throw Eval_Error("Can not find appropriate unary '" + this->children[0]->text + "'");
    }
  }

  /**
   * Evaluates (and generates) an inline array initialization
   */
  Boxed_Value Inline_Array_AST_Node::eval(Dispatch_Engine &ss) {
    std::vector<Boxed_Value> vec;
    if (this->children.size() > 0) {
      for (size_t i = 0; i < this->children[0]->children.size(); ++i) {
        try {
          vec.push_back(this->children[0]->children[i]->eval(ss));
        }
        catch(Eval_Error &ee) {
          ee.call_stack.push_back(this->children[0]->children[i]);
          throw;
        }
      }
    }

    return const_var(vec);
  }

  /**
   * Evaluates (and generates) an inline range initialization
   */
  Boxed_Value Inline_Range_AST_Node::eval(Dispatch_Engine &ss) {
    try {
      return ss.call_function("generate_range",
                              this->children[0]->children[0]->children[0]->eval(ss),
                              this->children[0]->children[0]->children[1]->eval(ss));
    }
    catch (const dispatch_error &) {
      throw Eval_Error("Unable to generate range vector");
    }
    catch(Eval_Error &ee) {
      ee.call_stack.push_back(this->children[0]->children[0]);
      throw;
    }
  }

  /**
   * Evaluates (and generates) an inline map initialization
   */
  Boxed_Value Inline_Map_AST_Node::eval(Dispatch_Engine &ss) {
    try {
      Boxed_Value retval = ss.call_function("Map");
      for (size_t i = 0; i < this->children[0]->children.size(); ++i) {
        try {
          Boxed_Value slot 
            = ss.call_function("[]", retval, this->children[0]->children[i]->children[0]->eval(ss));
          ss.call_function("=", slot, this->children[0]->children[i]->children[1]->eval(ss));
        }
        catch (const dispatch_error &) {
          throw Eval_Error("Can not find appropriate '=' for map init");
        }
        catch(Eval_Error &ee) {
          ee.call_stack.push_back(this->children[0]->children[i]);
          throw;
        }
      }
      return retval;
    }
    catch (const dispatch_error &) {
      throw Eval_Error("Can not find appropriate 'Map()'");
    }
  }

  /**
   * Evaluates a function call, starting with its arguments.  Handles resetting the scope to the previous one after the call.
   */
  Boxed_Value Fun_Call_AST_Node::eval(Dispatch_Engine &ss) {
    Param_List_Builder plb;

    if ((this->children.size() > 1) && (this->children[1]->identifier == AST_Node_Type::Arg_List)) {
      for (size_t i = 0; i < this->children[1]->children.size(); ++i) {
        try {
          plb << this->children[1]->children[i]->eval(ss);
        }
        catch(Eval_Error &ee) {
          ee.call_stack.push_back(this->children[1]->children[i]);
          throw;
        }
      }
    }

    Dispatch_Engine::Stack prev_stack = ss.get_stack();
    Dispatch_Engine::Stack new_stack = ss.new_stack();

    try {
      Boxed_Value fn = this->children[0]->eval(ss);

      try {
        ss.set_stack(new_stack);
        const Boxed_Value &retval = (*boxed_cast<Const_Proxy_Function>(fn))(plb);
        ss.set_stack(prev_stack);
        return retval;
      }
      catch(const dispatch_error &e){
        ss.set_stack(prev_stack);
        throw Eval_Error(std::string(e.what()) + " with function '" + this->children[0]->text + "'");
      }
      catch(Return_Value &rv) {
        ss.set_stack(prev_stack);
        return rv.retval;
      }
      catch(...) {
        ss.set_stack(prev_stack);
        throw;
      }
    }
    catch(Eval_Error &ee) {
      ee.call_stack.push_back(this->children[0]);
      ss.set_stack(prev_stack);
      throw Eval_Error(ee.reason);
    }

  }

  /**
   * Evaluates a function call, starting with its arguments.  Does NOT change scope.
   */
  Boxed_Value Inplace_Fun_Call_AST_Node::eval(Dispatch_Engine &ss) {
    Param_List_Builder plb;

    if ((this->children.size() > 1) && (this->children[1]->identifier == AST_Node_Type::Arg_List)) {
      for (size_t i = 0; i < this->children[1]->children.size(); ++i) {
        try {
          plb << this->children[1]->children[i]->eval(ss);
        }
        catch (Eval_Error &ee) {
          ee.call_stack.push_back(this->children[1]->children[i]);
          throw;
        }
      }
    }

    try {
      Boxed_Value fn = this->children[0]->eval(ss);

      try {
        return (*boxed_cast<Const_Proxy_Function >(fn))(plb);
      }
      catch(const dispatch_error &e){
        throw Eval_Error(std::string(e.what()) + " with function '" + this->children[0]->text + "'");
      }
      catch(Return_Value &rv) {
        return rv.retval;
      }
      catch(...) {
        throw;
      }
    }
    catch(Eval_Error &ee) {
      ee.call_stack.push_back(this->children[0]);
      throw Eval_Error(ee.reason);
    }

  }

  /**
   * Evaluates a method/attributes invocation
   */
  Boxed_Value Dot_Access_AST_Node::eval(Dispatch_Engine &ss) {
    Boxed_Value retval;
    try {
      retval = this->children[0]->eval(ss);
    }
    catch (Eval_Error &ee) {
      ee.call_stack.push_back(this->children[0]);
      throw;
    }
    
    if (this->children.size() > 1) {
      for (size_t i = 2; i < this->children.size(); i+=2) {
        Param_List_Builder plb;
        plb << retval;

        if (this->children[i]->children.size() > 1) {
          for (size_t j = 0; j < this->children[i]->children[1]->children.size(); ++j) {
            try {
              plb << this->children[i]->children[1]->children[j]->eval(ss);
            }
            catch (Eval_Error &ee) {
              ee.call_stack.push_back(this->children[i]->children[1]->children[j]);
              throw;
            }
          }
        }

        std::string fun_name;
        std::vector<std::pair<std::string, Proxy_Function > > funs;
        if ((this->children[i]->identifier == AST_Node_Type::Fun_Call) || (this->children[i]->identifier == AST_Node_Type::Array_Call)) {
          fun_name = this->children[i]->children[0]->text;
        }
        else {
          fun_name = this->children[i]->text;
        }
        
        Dispatch_Engine::Stack prev_stack = ss.get_stack();
        Dispatch_Engine::Stack new_stack = ss.new_stack();

        try {
          ss.set_stack(new_stack);
          retval = ss.call_function(fun_name, plb);
          ss.set_stack(prev_stack);
        }
        catch(const dispatch_error &e){
          ss.set_stack(prev_stack);
          throw Eval_Error(std::string(e.what()));
        }
        catch(Return_Value &rv) {
          ss.set_stack(prev_stack);
          retval = rv.retval;
        }
        catch(...) {
          ss.set_stack(prev_stack);
          throw;
        }
        if (this->children[i]->identifier == AST_Node_Type::Array_Call) {
          for (size_t j = 1; j < this->children[i]->children.size(); ++j) {
            try {
              retval = ss.call_function("[]", retval, this->children[i]->children[j]->eval(ss));
            }
            catch(std::out_of_range &) {
              throw Eval_Error("Out of bounds exception");
            }
            catch(const dispatch_error &){
              throw Eval_Error("Can not find appropriate array lookup '[]' ");
            }
            catch(Eval_Error &ee) {
              ee.call_stack.push_back(this->children[i]->children[j]);
              throw;
            }
          }
        }
      }
    }

    return retval;
  }

  /**
   * Evaluates an if/elseif/else block
   */
  Boxed_Value Try_AST_Node::eval(Dispatch_Engine &ss) {
    Boxed_Value retval;        

    ss.new_scope();
    try {
      retval = this->children[0]->eval(ss);
    }
    catch (Eval_Error &ee) {
      ee.call_stack.push_back(this->children[0]);
      if (this->children.back()->identifier == AST_Node_Type::Finally) {
        try {
          this->children.back()->children[0]->eval(ss);
        }
        catch (Eval_Error &ee) {
          ee.call_stack.push_back(this->children.back()->children[0]);
          ss.pop_scope();
          throw;
        }
      }
      ss.pop_scope();
      throw;
    }
    catch (const std::exception &e) {
      Boxed_Value except = Boxed_Value(boost::ref(e));

      unsigned int end_point = this->children.size();
      if (this->children.back()->identifier == AST_Node_Type::Finally) {
        end_point = this->children.size() - 1;
      }
      for (unsigned int i = 1; i < end_point; ++i) {
        AST_NodePtr catch_block = this->children[i];

        if (catch_block->children.size() == 1) {
          //No variable capture, no guards
          try {
            retval = catch_block->children[0]->eval(ss);
          }
          catch (Eval_Error &ee) {
            ee.call_stack.push_back(catch_block->children[0]);
            throw;
          }
          break;
        }
        else if (catch_block->children.size() == 2) {
          //Variable capture, no guards
          ss.add_object(catch_block->children[0]->text, except);
          try {
            retval = catch_block->children[1]->eval(ss);
          }
          catch (Eval_Error &ee) {
            ee.call_stack.push_back(catch_block->children[1]);
            throw;
          }

          break;
        }
        else if (catch_block->children.size() == 3) {
          //Variable capture, no guards
          ss.add_object(catch_block->children[0]->text, except);

          bool guard;
          try {
            guard = boxed_cast<bool>(catch_block->children[1]->eval(ss));
          } catch (const bad_boxed_cast &) {
            if (this->children.back()->identifier == AST_Node_Type::Finally) {
              try {
                this->children.back()->children[0]->eval(ss);
              }
              catch (Eval_Error &ee) {
                ee.call_stack.push_back(this->children.back()->children[0]);
                ss.pop_scope();
                throw;
              }
            }
            ss.pop_scope();
            throw Eval_Error("Guard condition not boolean");
          }
          if (guard) {
            try {
              retval = catch_block->children[2]->eval(ss);
            }
            catch (Eval_Error &ee) {
              ee.call_stack.push_back(catch_block->children[2]);
              throw;
            }

            break;
          }
        }
        else {
          if (this->children.back()->identifier == AST_Node_Type::Finally) {
            try {
              this->children.back()->children[0]->eval(ss);
            }
            catch (Eval_Error &ee) {
              ee.call_stack.push_back(this->children.back()->children[0]);
              ss.pop_scope();
              throw;
            }
          }
          ss.pop_scope();
          throw Eval_Error("Internal error: catch block size unrecognized");
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
            retval = catch_block->children[0]->eval(ss);
          }
          catch (Eval_Error &ee) {
            ee.call_stack.push_back(catch_block->children[0]);
            throw;
          }

          break;
        }
        else if (catch_block->children.size() == 2) {
          //Variable capture, no guards
          ss.add_object(catch_block->children[0]->text, except);
          try {
            retval = catch_block->children[1]->eval(ss);
          }
          catch (Eval_Error &ee) {
            ee.call_stack.push_back(catch_block->children[1]);
            throw;
          }

          break;
        }
        else if (catch_block->children.size() == 3) {
          //Variable capture, no guards
          ss.add_object(catch_block->children[0]->text, except);

          bool guard;
          try {
            guard = boxed_cast<bool>(catch_block->children[1]->eval(ss));
          }
          catch (const bad_boxed_cast &) {
            if (this->children.back()->identifier == AST_Node_Type::Finally) {
              try {
                this->children.back()->children[0]->eval(ss);
              }
              catch (Eval_Error &ee) {
                ee.call_stack.push_back(this->children.back()->children[0]);
                ss.pop_scope();
                throw;
              }
            }
            
            ss.pop_scope();
            throw Eval_Error("Guard condition not boolean");
          }
          catch (Eval_Error &ee) {
            ee.call_stack.push_back(catch_block->children[1]);
            throw;
          }
          if (guard) {
            try {
              retval = catch_block->children[2]->eval(ss);
            }
            catch (Eval_Error &ee) {
              ee.call_stack.push_back(catch_block->children[2]);
              throw;
            }
            break;
          }
        }
        else {
          if (this->children.back()->identifier == AST_Node_Type::Finally) {
            try {
              this->children.back()->children[0]->eval(ss);
            }
            catch (Eval_Error &ee) {
              ee.call_stack.push_back(this->children.back()->children[0]);
              ss.pop_scope();
              throw;
            }
          }
          ss.pop_scope();
          throw Eval_Error("Internal error: catch block size unrecognized");
        }
      }
    }
    catch (...) {
      if (this->children.back()->identifier == AST_Node_Type::Finally) {
        try {
          this->children.back()->children[0]->eval(ss);
        }
        catch (Eval_Error &ee) {
          ee.call_stack.push_back(this->children.back()->children[0]);
          ss.pop_scope();
          throw;
        }
      }
      ss.pop_scope();
      throw;
    }

    if (this->children.back()->identifier == AST_Node_Type::Finally) {
      try {
        retval = this->children.back()->children[0]->eval(ss);
      }
      catch (Eval_Error &ee) {
        ee.call_stack.push_back(this->children.back()->children[0]);
        ss.pop_scope();
        throw;
      }
    }

    ss.pop_scope();

    return retval;
  }

  /**
   * Evaluates an if/elseif/else block
   */
  Boxed_Value If_AST_Node::eval(Dispatch_Engine &ss) {
    bool cond;
    try {
      cond = boxed_cast<bool>(this->children[0]->eval(ss));
    }
    catch (const bad_boxed_cast &) {
      throw Eval_Error("If condition not boolean");
    }
    catch (Eval_Error &ee) {
      ee.call_stack.push_back(this->children[0]);
      throw;
    }
    
    if (cond) {
      try {
        return this->children[1]->eval(ss);
      }
      catch (Eval_Error &ee) {
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
              return this->children[i+1]->eval(ss);
            }
            catch (Eval_Error &ee) {
              ee.call_stack.push_back(this->children[i+1]);
              throw;
            }
          }
          else if (this->children[i]->text == "else if") {
            try {
              cond = boxed_cast<bool>(this->children[i+1]->eval(ss));
            }
            catch (const bad_boxed_cast &) {
              throw Eval_Error("'else if' condition not boolean");
            }
            catch (Eval_Error &ee) {
              ee.call_stack.push_back(this->children[i+1]);
              throw;
            }
            if (cond) {
              try {
                return this->children[i+2]->eval(ss);
              }
              catch (Eval_Error &ee) {
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

  /**
   * Evaluates a while block
   */
  Boxed_Value While_AST_Node::eval(Dispatch_Engine &ss) {
    bool cond;

    ss.new_scope();

    try {
      cond = boxed_cast<bool>(this->children[0]->eval(ss));
    }
    catch (const bad_boxed_cast &) {
      ss.pop_scope();
      throw Eval_Error("While condition not boolean");
    }
    catch (Eval_Error &ee) {
      ee.call_stack.push_back(this->children[0]);
      ss.pop_scope();
      throw;
    }
    while (cond) {
      try {
        try {
          this->children[1]->eval(ss);
        }
        catch (Eval_Error &ee) {
          ee.call_stack.push_back(this->children[1]);
          throw;
        }

        try {
          cond = boxed_cast<bool>(this->children[0]->eval(ss));
        }
        catch (const bad_boxed_cast &) {
          ss.pop_scope();
          throw Eval_Error("While condition not boolean");
        }
        catch (Eval_Error &ee) {
          ee.call_stack.push_back(this->children[0]);
          ss.pop_scope();
          throw;
        }
      }
      catch (Break_Loop &) {
        cond = false;
      }
    }
    ss.pop_scope();
    return Boxed_Value();
  }

  /**
   * Evaluates a for block, including the for's conditions, from left to right
   */
  Boxed_Value For_AST_Node::eval(Dispatch_Engine &ss) {
    bool cond;

    ss.new_scope();

    try {
      if (this->children.size() == 4) {
        try {
          this->children[0]->eval(ss);
        }
        catch (Eval_Error &ee) {
          ee.call_stack.push_back(this->children[0]);
          throw;
        }
       
        try {
          cond = boxed_cast<bool>(this->children[1]->eval(ss));
        }
        catch (Eval_Error &ee) {
          ee.call_stack.push_back(this->children[1]);
          throw;
        }
      }
      else {
        try {
          cond = boxed_cast<bool>(this->children[0]->eval(ss));
        }
        catch (Eval_Error &ee) {
          ee.call_stack.push_back(this->children[0]);
          ss.pop_scope();
          throw;
        }
      }
    }
    catch (const bad_boxed_cast &) {
      ss.pop_scope();
      throw Eval_Error("For condition not boolean");
    }
    while (cond) {
      try {
        if (this->children.size() == 4) {
          try {
            this->children[3]->eval(ss);
          }
          catch (Eval_Error &ee) {
            ee.call_stack.push_back(this->children[3]);
            throw;
          }

          try {
            this->children[2]->eval(ss);
          }
          catch (Eval_Error &ee) {
            ee.call_stack.push_back(this->children[2]);
            throw;
          }
          
          try {
            cond = boxed_cast<bool>(this->children[1]->eval(ss));
          }
          catch (Eval_Error &ee) {
            ee.call_stack.push_back(this->children[1]);
            throw;
          }
        }
        else {
          try {
            this->children[2]->eval(ss);
          }
          catch (Eval_Error &ee) {
            ee.call_stack.push_back(this->children[2]);
            throw;
          }

          try {
            this->children[1]->eval(ss);
          }
          catch (Eval_Error &ee) {
            ee.call_stack.push_back(this->children[1]);
            throw;
          }
          
          try {
            cond = boxed_cast<bool>(this->children[0]->eval(ss));
          }
          catch (Eval_Error &ee) {
            ee.call_stack.push_back(this->children[0]);
            ss.pop_scope();
            throw;
          }
        }
      }
      catch (const bad_boxed_cast &) {
        ss.pop_scope();
        throw Eval_Error("For condition not boolean");
      }
      catch (Break_Loop &) {
        cond = false;
      }
    }
    ss.pop_scope();
    return Boxed_Value();
  }

  /**
   * Evaluates a function definition
   */
  Boxed_Value Def_AST_Node::eval(Dispatch_Engine &ss) {
    std::vector<std::string> param_names;
    size_t numparams = 0;
    AST_NodePtr guardnode;

    if ((this->children.size() > 2) && (this->children[1]->identifier == AST_Node_Type::Arg_List)) {
      numparams = this->children[1]->children.size();
      for (size_t i = 0; i < numparams; ++i) {
        param_names.push_back(this->children[1]->children[i]->text);
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

    boost::shared_ptr<Dynamic_Proxy_Function> guard;
    if (guardnode) {
      guard = boost::shared_ptr<Dynamic_Proxy_Function>
        (new Dynamic_Proxy_Function(boost::bind(&eval_function<Dispatch_Engine>,
                                                boost::ref(ss), guardnode,
                                                param_names, _1), numparams));
    }

    try {
      const std::string & function_name = this->children[0]->text;
      const std::string & annotation = this->annotation?this->annotation->text:"";
      ss.add(Proxy_Function
          (new Dynamic_Proxy_Function(boost::bind(&eval_function<Dispatch_Engine>,
                                                  boost::ref(ss), this->children.back(),
                                                  param_names, _1), numparams,
                                      annotation, guard)), function_name);
    }
    catch (reserved_word_error &e) {
      throw Eval_Error("Reserved word used as function name '" + e.word + "'");
    }
    return Boxed_Value();
  }

  /**
   * Evaluates a function definition
   */
  Boxed_Value Method_AST_Node::eval(Dispatch_Engine &ss) {

    std::vector<std::string> param_names;
    AST_NodePtr guardnode;

    //The first param of a method is always the implied this ptr.
    param_names.push_back("this");

    if ((this->children.size() > 3) && (this->children[2]->identifier == AST_Node_Type::Arg_List)) {
      for (size_t i = 0; i < this->children[2]->children.size(); ++i) {
        param_names.push_back(this->children[2]->children[i]->text);
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

    size_t numparams = param_names.size();

    boost::shared_ptr<Dynamic_Proxy_Function> guard;
    if (guardnode) {
      guard = boost::shared_ptr<Dynamic_Proxy_Function>
        (new Dynamic_Proxy_Function(boost::bind(&eval_function<Dispatch_Engine>,
                                                boost::ref(ss), guardnode,
                                                param_names, _1), numparams));
    }

    try {
      const std::string & annotation = this->annotation?this->annotation->text:"";
      const std::string & class_name = this->children[0]->text;
      const std::string & function_name = this->children[1]->text;
      if (function_name == class_name) {
        ss.add(Proxy_Function
            (new Dynamic_Object_Constructor(class_name, Proxy_Function
                                            (new Dynamic_Proxy_Function(boost::bind(&eval_function<Dispatch_Engine>,
                                                                                    boost::ref(ss), this->children.back(),
                                                                                    param_names, _1), numparams,
                                                                        annotation, guard)))), function_name);

      }
      else {
        boost::optional<chaiscript::Type_Info> ti;
        try {
          ti = ss.get_type(class_name);
        } catch (const std::range_error &) {
          // No biggie, the type name is just not known
        }
        ss.add(Proxy_Function
            (new Dynamic_Object_Function(class_name, Proxy_Function
                                         (new Dynamic_Proxy_Function(boost::bind(&eval_function<Dispatch_Engine>,
                                                                                 boost::ref(ss), this->children.back(),
                                                                                 param_names, _1), numparams,
                                                                     annotation, guard)), ti)), function_name);

      }
    }
    catch (reserved_word_error &e) {
      throw Eval_Error("Reserved word used as method name '" + e.word + "'");
    }
    return Boxed_Value();
  }

  /**
   * Evaluates a lambda (anonymous function)
   */
  Boxed_Value Lambda_AST_Node::eval(Dispatch_Engine &ss) {
    std::vector<std::string> param_names;
    size_t numparams = 0;

    if ((this->children.size() > 0) && (this->children[0]->identifier == AST_Node_Type::Arg_List)) {
      numparams = this->children[0]->children.size();
      for (size_t i = 0; i < numparams; ++i) {
        param_names.push_back(this->children[0]->children[i]->text);
      }

    }
    else {
      //no parameters
      numparams = 0;
    }

    return Boxed_Value(Proxy_Function(new Dynamic_Proxy_Function
                                      (boost::bind(&eval_function<Dispatch_Engine>, boost::ref(ss), this->children.back(), param_names, _1),
                                       numparams)));
  }

  /**
   * Evaluates a scoped block.  Handles resetting the scope after the block has completed.
   */
  Boxed_Value Block_AST_Node::eval(Dispatch_Engine &ss) {
    unsigned int num_children = this->children.size();

    ss.new_scope();
    for (size_t i = 0; i < num_children; ++i) {
      try {
        const Boxed_Value &retval = this->children[i]->eval(ss);

        if (i + 1 == num_children)
          {
            ss.pop_scope();
            return retval;
          }
      }
      catch (const chaiscript::Return_Value &) {
        ss.pop_scope();
        throw;
      }
      catch (Eval_Error &ee) {
        ee.call_stack.push_back(this->children[i]);
        ss.pop_scope();
        throw;
      }
      catch (...) {
        ss.pop_scope();
        throw;
      }
    }

    ss.pop_scope();
    return Boxed_Value();
  }

  /**
   * Evaluates a return statement
   */
  Boxed_Value Return_AST_Node::eval(Dispatch_Engine &ss) {
    if (this->children.size() > 0) {
      try {
        throw Return_Value(this->children[0]->eval(ss));
      }
      catch (Eval_Error &ee) {
        ee.call_stack.push_back(this->children[0]);
        throw;
      }
    }
    else {
      throw Return_Value(Boxed_Value());
    }
  }

  /**
   * Evaluates a break statement
   */
  Boxed_Value Break_AST_Node::eval(Dispatch_Engine &) {
    throw Break_Loop();
  }
}
#endif /* CHAISCRIPT_EVAL_HPP_ */
