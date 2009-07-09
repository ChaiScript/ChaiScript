// This file is distributed under the BSD License.
// See LICENSE.TXT for details.

#ifndef CHAISCRIPT_EVAL_HPP_
#define CHAISCRIPT_EVAL_HPP_

#include <map>

namespace chaiscript
{
    /**
     * Helper function that will set up the scope around a function call, including handling the named function parameters
     */
    template <typename Eval_System>
    const dispatchkit::Boxed_Value eval_function (Eval_System &ss, TokenPtr node, const std::vector<std::string> &param_names, const std::vector<dispatchkit::Boxed_Value> &vals) {
        ss.new_scope();

        for (unsigned int i = 0; i < param_names.size(); ++i) {
            ss.add_object(param_names[i], vals[i]);
        }

        dispatchkit::Boxed_Value retval;

        try {
            retval = eval_token(ss, node);
            ss.pop_scope();
        } catch (const Return_Value &rv) {
            retval = rv.retval;
            ss.pop_scope();
        } catch (...) {
            ss.pop_scope();
            throw;
        }

        return retval;
    }

    /**
     * Evaluates the top-level file node
     */
    template <typename Eval_System>
    dispatchkit::Boxed_Value eval_file(Eval_System &ss, TokenPtr node) {
        dispatchkit::Boxed_Value retval;
        unsigned int i;
        for (i = 0; i < node->children.size(); ++i) {
            retval = eval_token(ss, node->children[i]);
        }
        return retval;
    }

    /**
     * Evaluates a variable or function name identifier
     */
    template <typename Eval_System>
    dispatchkit::Boxed_Value eval_id(Eval_System &ss, TokenPtr node) {

        if (node->text == "true") {
            return dispatchkit::Boxed_Value(true);
        }
        else if (node->text == "false") {
            return dispatchkit::Boxed_Value(false);
        }
        else {
            try {
                return ss.get_object(node->text);
            }
            catch (std::exception &) {
                throw Eval_Error("Can not find object: " + node->text, node);
            }
        }
    }

    /**
     * Evaluates a floating point number
     */
    template <typename Eval_System>
    dispatchkit::Boxed_Value eval_float(Eval_System &, TokenPtr node) {
        return dispatchkit::Boxed_Value(double(atof(node->text.c_str())));
    }

    /**
     * Evaluates an integer
     */
    template <typename Eval_System>
    dispatchkit::Boxed_Value eval_int(Eval_System &, TokenPtr node) {
        return dispatchkit::Boxed_Value(atoi(node->text.c_str()));
    }

    /**
     * Evaluates a quoted string
     */
    template <typename Eval_System>
    dispatchkit::Boxed_Value eval_quoted_string(Eval_System &, TokenPtr node) {
        return dispatchkit::Boxed_Value(node->text);
    }

    /**
     * Evaluates a char group
     */
    template <typename Eval_System>
    dispatchkit::Boxed_Value eval_single_quoted_string(Eval_System &, TokenPtr node) {
        return dispatchkit::Boxed_Value(node->text);
    }

    /**
     * Evaluates a string of equations in reverse order so that the right-most side has precedence
     */
    template <typename Eval_System>
    dispatchkit::Boxed_Value eval_equation(Eval_System &ss, TokenPtr node) {
        dispatchkit::Boxed_Value retval;
        unsigned int i;
        retval = eval_token(ss, node->children.back());
        if (node->children.size() > 1) {
            for (i = node->children.size()-3; ((int)i) >= 0; i -= 2) {
                if (node->children[i+1]->text == "=") {
                    dispatchkit::Boxed_Value lhs = eval_token(ss, node->children[i]);
                    try {
                        if (lhs.is_unknown())
                        {
                          retval = dispatch(ss.get_function("clone"), dispatchkit::Param_List_Builder() << retval);
                        }
                        dispatchkit::Param_List_Builder plb;
                        plb << lhs;
                        plb << retval;
                        try {
                            retval = dispatch(ss.get_function(node->children[i+1]->text), plb);
                        }
                        catch(const dispatchkit::dispatch_error &){
                            throw Eval_Error("Mismatched types in equation", node->children[i+1]);
                        }
                    }
                    catch(const dispatchkit::dispatch_error &){
                        throw Eval_Error("Can not clone right hand side of equation", node->children[i+1]);
                    }
                }
                else if (node->children[i+1]->text == ":=") {
                    dispatchkit::Boxed_Value lhs = eval_token(ss, node->children[i]);
                    if (lhs.is_unknown() || dispatchkit::Bootstrap::type_match(lhs, retval)) {
                        lhs.assign(retval);
                    }
                    else {
                        throw Eval_Error("Mismatched types in equation", node->children[i+1]);
                    }
                }
                else {
                    dispatchkit::Param_List_Builder plb;
                    plb << eval_token(ss, node->children[i]);
                    plb << retval;
                    try {
                        retval = dispatch(ss.get_function(node->children[i+1]->text), plb);
                    }
                    catch(const dispatchkit::dispatch_error &){
                        throw Eval_Error("Can not find appropriate '" + node->children[i+1]->text + "'", node->children[i+1]);
                    }
                }
            }
        }
        return retval;
    }

    /**
     * Evaluates a variable declaration
     */
    template <typename Eval_System>
    dispatchkit::Boxed_Value eval_var_decl(Eval_System &ss, TokenPtr node) {
        ss.add_object(node->children[0]->text, dispatchkit::Boxed_Value());
        return ss.get_object(node->children[0]->text);
    }

    /**
     * Evaluates binary boolean operators.  Respects short-circuiting rules.
     */
    template <typename Eval_System>
    dispatchkit::Boxed_Value eval_expression(Eval_System &ss, TokenPtr node) {
        dispatchkit::Boxed_Value retval;
        unsigned int i;

        retval = eval_token(ss, node->children[0]);
        if (node->children.size() > 1) {
            for (i = 1; i < node->children.size(); i += 2) {
                bool lhs;
                try {
                    lhs = dispatchkit::boxed_cast<bool &>(retval);
                }
                catch (const dispatchkit::bad_boxed_cast &) {
                    throw Eval_Error("Condition not boolean", node);
                }
                if (node->children[i]->text == "&&") {
                    if (lhs) {
                        retval = eval_token(ss, node->children[i+1]);
                    }
                    else {
                        retval = dispatchkit::Boxed_Value(false);
                    }
                }
                else if (node->children[i]->text == "||") {
                    if (lhs) {
                        retval = dispatchkit::Boxed_Value(true);
                    }
                    else {
                        retval = eval_token(ss, node->children[i+1]);
                    }
                }
            }
        }
        return retval;
    }

    /**
     * Evaluates comparison, additions, and multiplications and their relatives
     */
    template <typename Eval_System>
    dispatchkit::Boxed_Value eval_comp_add_mul(Eval_System &ss, TokenPtr node) {
        dispatchkit::Boxed_Value retval;
        unsigned int i;

        retval = eval_token(ss, node->children[0]);
        if (node->children.size() > 1) {
            for (i = 1; i < node->children.size(); i += 2) {
                dispatchkit::Param_List_Builder plb;
                plb << retval;
                plb << eval_token(ss, node->children[i + 1]);

                try {
                    retval = dispatch(ss.get_function(node->children[i]->text), plb);
                }
                catch(const dispatchkit::dispatch_error &){
                    throw Eval_Error("Can not find appropriate '" + node->children[i]->text + "'", node->children[i]);
                }
            }
        }

        return retval;
    }

    /**
     * Evaluates an array lookup
     */
    template <typename Eval_System>
    dispatchkit::Boxed_Value eval_array_call(Eval_System &ss, TokenPtr node) {
        dispatchkit::Boxed_Value retval;
        unsigned int i;

        retval = eval_token(ss, node->children[0]);
        for (i = 1; i < node->children.size(); ++i) {
            dispatchkit::Param_List_Builder plb;
            plb << retval;
            plb << eval_token(ss, node->children[i]);
            try {
                retval = dispatch(ss.get_function("[]"), plb);
            }
            catch(std::out_of_range &) {
                throw Eval_Error("Out of bounds exception", node);
            }
            catch(const dispatchkit::dispatch_error &){
                throw Eval_Error("Can not find appropriate array lookup '[]' " + node->children[i]->text, node->children[i]);
            }
        }

        return retval;
    }

    /**
     * Evaluates a unary negation
     */
    template <typename Eval_System>
    dispatchkit::Boxed_Value eval_negate(Eval_System &ss, TokenPtr node) {
        dispatchkit::Boxed_Value retval;

        retval = eval_token(ss, node->children[0]);
        dispatchkit::Param_List_Builder plb;
        plb << retval;
        plb << dispatchkit::Boxed_Value(-1.0);

        try {
            return dispatch(ss.get_function("*"), plb);
        }
        catch(std::exception &){
            throw Eval_Error("Can not find appropriate negation", node->children[0]);
        }
    }

    /**
     * Evaluates a unary boolean not
     */
    template <typename Eval_System>
    dispatchkit::Boxed_Value eval_not(Eval_System &ss, TokenPtr node) {
        dispatchkit::Boxed_Value retval;

        bool cond;
        try {
            retval = eval_token(ss, node->children[0]);
            cond = dispatchkit::boxed_cast<bool &>(retval);
        }
        catch (const dispatchkit::bad_boxed_cast &) {
            throw Eval_Error("Boolean not('!') condition not boolean", node->children[0]);
        }
        return dispatchkit::Boxed_Value(!cond);
    }

    /**
     * Evaluates any unary prefix
     */
    template <typename Eval_System>
    dispatchkit::Boxed_Value eval_prefix(Eval_System &ss, TokenPtr node) {
        dispatchkit::Boxed_Value retval;

        retval = eval_token(ss, node->children[1]);
        dispatchkit::Param_List_Builder plb;
        plb << retval;

        try {
            return dispatch(ss.get_function(node->children[0]->text), plb);
        }
        catch(std::exception &){
            throw Eval_Error("Can not find appropriate prefix", node->children[0]);
        }
    }

    /**
     * Evaluates (and generates) an inline array initialization
     */
    template <typename Eval_System>
    dispatchkit::Boxed_Value eval_inline_array(Eval_System &ss, TokenPtr node) {
        dispatchkit::Boxed_Value retval;
        unsigned int i;

        try {
            retval = dispatch(ss.get_function("Vector"), dispatchkit::Param_List_Builder());
            if (node->children.size() > 0) {
                for (i = 0; i < node->children[0]->children.size(); ++i) {
                    try {
                        dispatchkit::Boxed_Value tmp = eval_token(ss, node->children[0]->children[i]);
                        dispatch(ss.get_function("push_back"), dispatchkit::Param_List_Builder() << retval << tmp);
                    }
                    catch (const dispatchkit::dispatch_error &) {
                        throw Eval_Error("Can not find appropriate 'push_back'", node->children[0]->children[i]);
                    }
                }
            }
        }
        catch (const dispatchkit::dispatch_error &) {
            throw Eval_Error("Can not find appropriate 'Vector()'", node);
        }

        return retval;
    }

    /**
     * Evaluates (and generates) an inline range initialization
     */
    template <typename Eval_System>
    dispatchkit::Boxed_Value eval_inline_range(Eval_System &ss, TokenPtr node) {
        try {
            return dispatch(ss.get_function("generate_range"), dispatchkit::Param_List_Builder()
                << eval_token(ss, node->children[0]->children[0]->children[0])
                << eval_token(ss, node->children[0]->children[0]->children[1]));
        }
        catch (const dispatchkit::dispatch_error &) {
            throw Eval_Error("Unable to generate range vector", node);
        }
    }

    /**
     * Evaluates (and generates) an inline map initialization
     */
    template <typename Eval_System>
    dispatchkit::Boxed_Value eval_inline_map(Eval_System &ss, TokenPtr node) {
        dispatchkit::Boxed_Value retval;
        unsigned int i;

        try {
            retval = dispatch(ss.get_function("Map"), dispatchkit::Param_List_Builder());
            for (i = 0; i < node->children[0]->children.size(); ++i) {
                try {
                    dispatchkit::Boxed_Value key = eval_token(ss, node->children[0]->children[i]->children[0]);
                    dispatchkit::Boxed_Value slot = dispatch(ss.get_function("[]"), dispatchkit::Param_List_Builder() << retval << key);
                    dispatch(ss.get_function("="), dispatchkit::Param_List_Builder() << slot << eval_token(ss, node->children[0]->children[i]->children[1]));
                }
                catch (const dispatchkit::dispatch_error &) {
                    throw Eval_Error("Can not find appropriate '=' for map init", node->children[0]->children[i]);
                }
            }
        }
        catch (const dispatchkit::dispatch_error &) {
            throw Eval_Error("Can not find appropriate 'Map()'", node);
        }

        return retval;
    }

    /**
     * Evaluates a function call, starting with its arguments.  Handles resetting the scope to the previous one after the call.
     */
    template <typename Eval_System>
    dispatchkit::Boxed_Value eval_fun_call(Eval_System &ss, TokenPtr node) {
        dispatchkit::Boxed_Value retval;
        dispatchkit::Param_List_Builder plb;
        dispatchkit::Dispatch_Engine::Stack prev_stack = ss.get_stack();
        dispatchkit::Dispatch_Engine::Stack new_stack;
        unsigned int i;

        new_stack.push_back(dispatchkit::Dispatch_Engine::Scope());

        if ((node->children.size() > 1) && (node->children[1]->identifier == Token_Type::Arg_List)) {
            for (i = 0; i < node->children[1]->children.size(); ++i) {
                plb << eval_token(ss, node->children[1]->children[i]);
            }
        }
        dispatchkit::Boxed_Value fn;
        try {
            fn = eval_token(ss, node->children[0]);
        }
        catch(Eval_Error &ee) {
            ss.set_stack(prev_stack);
            throw Eval_Error(ee.reason, node->children[0]);
        }
        try {
            ss.set_stack(new_stack);
            retval = (*dispatchkit::boxed_cast<boost::shared_ptr<dispatchkit::Proxy_Function> >(fn))(plb);
            ss.set_stack(prev_stack);
        }
        catch(const dispatchkit::dispatch_error &e){
            ss.set_stack(prev_stack);
            throw Eval_Error(std::string(e.what()) + " with function '" + node->children[0]->text + "'", node->children[0]);
        }
        catch(Return_Value &rv) {
            ss.set_stack(prev_stack);
            retval = rv.retval;
        }
        catch(...) {
            ss.set_stack(prev_stack);
            throw;
        }

        return retval;
    }

    /**
     * Evaluates a method/attributes invocation
     */
    template <typename Eval_System>
    dispatchkit::Boxed_Value eval_dot_access(Eval_System &ss, TokenPtr node) {
        dispatchkit::Boxed_Value retval;
        std::vector<std::pair<std::string, boost::shared_ptr<dispatchkit::Proxy_Function> > > fn;
        dispatchkit::Dispatch_Engine::Stack prev_stack = ss.get_stack();
        dispatchkit::Dispatch_Engine::Stack new_stack;
        unsigned int i, j;

        new_stack.push_back(dispatchkit::Dispatch_Engine::Scope());

        //todo: Please extract a single way of doing function calls between this and eval_fun_call

        retval = eval_token(ss, node->children[0]);
        if (node->children.size() > 1) {
            for (i = 1; i < node->children.size(); ++i) {
                dispatchkit::Param_List_Builder plb;
                plb << retval;

                if (node->children[i]->children.size() > 1) {
                    for (j = 0; j < node->children[i]->children[1]->children.size(); ++j) {
                        plb << eval_token(ss, node->children[i]->children[1]->children[j]);
                    }
                }

                std::string fun_name;
                if (node->children[i]->identifier == Token_Type::Fun_Call) {
                    fun_name = node->children[i]->children[0]->text;
                }
                else {
                    fun_name = node->children[i]->text;
                }

                try {
                    fn = ss.get_function(fun_name);
                    ss.set_stack(new_stack);
                    retval = dispatch(fn, plb);
                    ss.set_stack(prev_stack);
                }
                catch(const dispatchkit::dispatch_error &e){
                    ss.set_stack(prev_stack);
                    throw Eval_Error(std::string(e.what()) + " with function '" + fun_name + "'", node->children[i]);
                }
                catch(Return_Value &rv) {
                    ss.set_stack(prev_stack);
                    retval = rv.retval;
                }
                catch(...) {
                    ss.set_stack(prev_stack);
                    throw;
                }
            }
        }

        return retval;
    }

    /**
     * Evaluates an if/elseif/else block
     */
    template <typename Eval_System>
    dispatchkit::Boxed_Value eval_if(Eval_System &ss, TokenPtr node) {
        dispatchkit::Boxed_Value retval;
        unsigned int i;

        retval = eval_token(ss, node->children[0]);
        bool cond;
        try {
            cond = dispatchkit::boxed_cast<bool &>(retval);
        }
        catch (const dispatchkit::bad_boxed_cast &) {
            throw Eval_Error("If condition not boolean", node->children[0]);
        }
        if (cond) {
            retval = eval_token(ss, node->children[1]);
        }
        else {
            if (node->children.size() > 2) {
                i = 2;
                while ((!cond) && (i < node->children.size())) {
                    if (node->children[i]->text == "else") {
                        retval = eval_token(ss, node->children[i+1]);
                        cond = true;
                    }
                    else if (node->children[i]->text == "elseif") {
                        retval = eval_token(ss, node->children[i+1]);
                        try {
                            cond = dispatchkit::boxed_cast<bool &>(retval);
                        }
                        catch (const dispatchkit::bad_boxed_cast &) {
                            throw Eval_Error("Elseif condition not boolean", node->children[i+1]);
                        }
                        if (cond) {
                            retval = eval_token(ss, node->children[i+2]);
                        }
                    }
                    i = i + 3;
                }
            }
        }

        return retval;
    }

    /**
     * Evaluates a while block
     */
    template <typename Eval_System>
    dispatchkit::Boxed_Value eval_while(Eval_System &ss, TokenPtr node) {
        dispatchkit::Boxed_Value retval;

        retval = eval_token(ss, node->children[0]);
        bool cond;
        try {
            cond = dispatchkit::boxed_cast<bool &>(retval);
        }
        catch (const dispatchkit::bad_boxed_cast &) {
            throw Eval_Error("While condition not boolean", node->children[0]);
        }
        while (cond) {
            try {
                eval_token(ss, node->children[1]);
                retval = eval_token(ss, node->children[0]);
                try {
                    cond = dispatchkit::boxed_cast<bool &>(retval);
                }
                catch (const dispatchkit::bad_boxed_cast &) {
                    throw Eval_Error("While condition not boolean", node->children[0]);
                }
            }
            catch (Break_Loop &) {
                cond = false;
            }
        }
        return dispatchkit::Boxed_Value();
    }

    /**
     * Evaluates a for block, including the for's conditions, from left to right
     */
    template <typename Eval_System>
    dispatchkit::Boxed_Value eval_for(Eval_System &ss, TokenPtr node) {
        dispatchkit::Boxed_Value retval;

        dispatchkit::Boxed_Value condition;
        bool cond;

        try {
            if (node->children.size() == 4) {
                eval_token(ss, node->children[0]);
                condition = eval_token(ss, node->children[1]);
            }
            else if (node->children.size() == 3){
                condition = eval_token(ss, node->children[0]);
            }
            cond = dispatchkit::boxed_cast<bool &>(condition);
        }
        catch (const dispatchkit::bad_boxed_cast &) {
            throw Eval_Error("For condition not boolean", node);
        }
        while (cond) {
            try {
                if (node->children.size() == 4) {
                    eval_token(ss, node->children[3]);
                    eval_token(ss, node->children[2]);
                    condition = eval_token(ss, node->children[1]);
                }
                else if (node->children.size() == 3) {
                    eval_token(ss, node->children[2]);
                    eval_token(ss, node->children[1]);
                    condition = eval_token(ss, node->children[0]);
                }
                cond = dispatchkit::boxed_cast<bool &>(condition);

            }
            catch (const dispatchkit::bad_boxed_cast &) {
                throw Eval_Error("For condition not boolean", node);
            }
            catch (Break_Loop &) {
                cond = false;
            }
        }
        return dispatchkit::Boxed_Value();
    }

    /**
     * Evaluates a function definition
     */
    template <typename Eval_System>
    dispatchkit::Boxed_Value eval_def(Eval_System &ss, TokenPtr node) {
        dispatchkit::Boxed_Value retval;
        unsigned int i;

        std::vector<std::string> param_names;
        std::string annotation = node->annotation?node->annotation->text:"";
        boost::shared_ptr<dispatchkit::Dynamic_Proxy_Function> guard;
        size_t numparams = 0;
        std::string function_name = node->children[0]->text;
        TokenPtr guardnode;

        if ((node->children.size() > 2) && (node->children[1]->identifier == Token_Type::Arg_List)) {
            numparams = node->children[1]->children.size();
            for (i = 0; i < numparams; ++i) {
                param_names.push_back(node->children[1]->children[i]->text);
            }

            if (node->children.size() > 3) {
                guardnode = node->children[2];
            }
        }
        else {
            //no parameters
            numparams = 0;

            if (node->children.size() > 2) {
                guardnode = node->children[1];
            }
        }

        if (guardnode) {
            guard = boost::shared_ptr<dispatchkit::Dynamic_Proxy_Function>
                (new dispatchkit::Dynamic_Proxy_Function(boost::bind(&eval_function<Eval_System>,
                                                                     boost::ref(ss), guardnode,
                                                                     param_names, _1), numparams));
        }

        ss.register_function(boost::shared_ptr<dispatchkit::Proxy_Function>
            (new dispatchkit::Dynamic_Proxy_Function(boost::bind(&eval_function<Eval_System>,
                                                                 boost::ref(ss), node->children.back(),
                                                                 param_names, _1), numparams,
                                                     annotation, guard)), function_name);

        return retval;
    }

    /**
     * Evaluates a lambda (anonymous function)
     */
    template <typename Eval_System>
    dispatchkit::Boxed_Value eval_lambda(Eval_System &ss, TokenPtr node) {
        dispatchkit::Boxed_Value retval;
        unsigned int i;

        std::vector<std::string> param_names;
        size_t numparams = 0;

        if ((node->children.size() > 0) && (node->children[0]->identifier == Token_Type::Arg_List)) {
            numparams = node->children[0]->children.size();
            for (i = 0; i < numparams; ++i) {
                param_names.push_back(node->children[0]->children[i]->text);
            }

        }
        else {
            //no parameters
            numparams = 0;
        }

        return dispatchkit::Boxed_Value(boost::shared_ptr<dispatchkit::Proxy_Function>(
              new dispatchkit::Dynamic_Proxy_Function(
                boost::bind(&eval_function<Eval_System>, boost::ref(ss), node->children.back(), param_names, _1), numparams)));
    }

    /**
     * Evaluates a scoped block.  Handles resetting the scope after the block has completed.
     */
    template <typename Eval_System>
    dispatchkit::Boxed_Value eval_block(Eval_System &ss, TokenPtr node) {
        dispatchkit::Boxed_Value retval;
        unsigned int i;

        ss.new_scope();
        for (i = 0; i < node->children.size(); ++i) {
            try {
                retval = eval_token(ss, node->children[i]);
            }
            catch (const chaiscript::Return_Value &rv) {
                ss.pop_scope();
                retval = rv.retval;
                throw;
            }
            catch (...) {
                ss.pop_scope();
                throw;
            }
        }
        ss.pop_scope();

        return retval;
    }

    /**
     * Evaluates a return statement
     */
    template <typename Eval_System>
    dispatchkit::Boxed_Value eval_return(Eval_System &ss, TokenPtr node) {
        dispatchkit::Boxed_Value retval;
        if (node->children.size() > 0) {
            retval = eval_token(ss, node->children[0]);
        }
        else {
            retval = dispatchkit::Boxed_Value();
        }
        throw Return_Value(retval, node);
    }

    /**
     * Evaluates a break statement
     */
    template <typename Eval_System>
    dispatchkit::Boxed_Value eval_break(Eval_System &, TokenPtr node) {
        throw Break_Loop(node);
    }

    /**
     * Top-level evaluation dispatch for all AST node types
     */
    template <typename Eval_System>
    dispatchkit::Boxed_Value eval_token(Eval_System &ss, TokenPtr node) {
        dispatchkit::Boxed_Value retval;

        switch (node->identifier) {
            case (Token_Type::File) :
                retval = eval_file(ss, node);
            break;

            case (Token_Type::Id) :
                retval = eval_id(ss, node);
            break;

            case (Token_Type::Float) :
                retval = eval_float(ss, node);
            break;

            case (Token_Type::Int) :
                retval = eval_int(ss, node);
            break;

            case (Token_Type::Quoted_String) :
                retval = eval_quoted_string(ss, node);
            break;

            case (Token_Type::Single_Quoted_String) :
                retval = eval_single_quoted_string(ss, node);
            break;

            case (Token_Type::Equation) :
                retval = eval_equation(ss, node);
            break;

            case (Token_Type::Var_Decl) :
                retval = eval_var_decl(ss, node);
            break;

            case (Token_Type::Expression) :
                retval = eval_expression(ss, node);
            break;

            case (Token_Type::Comparison) :
            case (Token_Type::Additive) :
            case (Token_Type::Multiplicative) :
                retval = eval_comp_add_mul(ss, node);
            break;

            case (Token_Type::Array_Call) :
                retval = eval_array_call(ss, node);
            break;

            case (Token_Type::Negate) :
                retval = eval_negate(ss, node);
            break;

            case (Token_Type::Not) :
                retval = eval_not(ss, node);
            break;

            case (Token_Type::Prefix) :
                retval = eval_prefix(ss, node);
            break;

            case (Token_Type::Inline_Array) :
                retval = eval_inline_array(ss, node);
            break;

            case (Token_Type::Inline_Range) :
                retval = eval_inline_range(ss, node);
            break;

            case (Token_Type::Inline_Map) :
                retval = eval_inline_map(ss, node);
            break;

            case (Token_Type::Fun_Call) :
                retval = eval_fun_call(ss, node);
            break;

            case (Token_Type::Dot_Access) :
                retval = eval_dot_access(ss, node);
            break;

            case(Token_Type::If) :
                retval = eval_if(ss, node);
            break;

            case(Token_Type::While) :
                retval = eval_while(ss, node);
            break;

            case(Token_Type::For) :
                retval = eval_for(ss, node);
            break;

            case (Token_Type::Def) :
                retval = eval_def(ss, node);
            break;

            case (Token_Type::Lambda) :
                retval = eval_lambda(ss, node);
            break;

            case (Token_Type::Block) :
                retval = eval_block(ss, node);
            break;

            case (Token_Type::Return) :
                retval = eval_return(ss, node);
            break;

            case (Token_Type::Break) :
                retval = eval_break(ss, node);
            break;
        }

        return retval;
    }
}
#endif /* CHAISCRIPT_EVAL_HPP_ */
