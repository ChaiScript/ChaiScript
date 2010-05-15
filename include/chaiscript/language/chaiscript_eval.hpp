// This file is distributed under the BSD License.
// See "license.txt" for details.
// Copyright 2009-2010, Jonathan Turner (jturner@minnow-lang.org)
// and Jason Turner (lefticus@gmail.com)
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
    const Boxed_Value eval_function (Eval_System &ss, const TokenPtr &node, const std::vector<std::string> &param_names, const std::vector<Boxed_Value> &vals) {
        ss.new_scope();

        for (unsigned int i = 0; i < param_names.size(); ++i) {
            ss.add_object(param_names[i], vals[i]);
        }

        try {
            Boxed_Value retval(eval_token(ss, node));
            ss.pop_scope();
            return retval;
        } catch (const Return_Value &rv) {
            ss.pop_scope();
            return rv.retval;
        } catch (...) {
            ss.pop_scope();
            throw;
        }
    }

    /**
     * Evaluates the top-level file node
     */
    template <typename Eval_System>
    Boxed_Value eval_file(Eval_System &ss, const TokenPtr &node) {
        const unsigned int size = node->children.size(); 
        for (unsigned int i = 0; i < size; ++i) {
            const Boxed_Value &retval = eval_token(ss, node->children[i]);
            if (i + 1 == size)
            {
                return retval;
            }
        }
        return Boxed_Value();
    }

    template <typename Eval_System>
    void cache_const(Eval_System &/*ss*/, const TokenPtr &node, const Boxed_Value &value) {
        node->cached_value = value;
        node->is_cached = true;
    }
    /**
     * Evaluates a variable or function name identifier
     */
    template <typename Eval_System>
    Boxed_Value eval_id(Eval_System &ss, const TokenPtr &node) {

        if (node->text == "true") {
            //return const_var(true);
            if (!node->is_cached) {
                cache_const(ss, node, const_var(true));
            }
            return node->cached_value;
        }
        else if (node->text == "false") {
            //return const_var(false);
            if (!node->is_cached) {
                cache_const(ss, node, const_var(false));
            }
            return node->cached_value;
        }
        else if (node->text == "Infinity") {
            //return const_var(false);
            if (!node->is_cached) {
                cache_const(ss, node, const_var(std::numeric_limits<double>::infinity()));
            }
            return node->cached_value;
        }
        else if (node->text == "NaN") {
            //return const_var(false);
            if (!node->is_cached) {
                cache_const(ss, node, const_var(std::numeric_limits<double>::quiet_NaN()));
            }
            return node->cached_value;
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
    Boxed_Value eval_float(Eval_System &ss, const TokenPtr &node) {
        //return const_var(double(atof(node->text.c_str())));

        if (!node->is_cached) {
            cache_const(ss, node, const_var(double(atof(node->text.c_str()))));
        }
        return node->cached_value;
    }

    /**
     * Evaluates an integer
     */
    template <typename Eval_System>
    Boxed_Value eval_int(Eval_System &ss, const TokenPtr &node) {
        //return const_var(atoi(node->text.c_str()));

        if (!node->is_cached) {
            cache_const(ss, node, const_var(int(atoi(node->text.c_str()))));
        }
        return node->cached_value;
    }

    /**
     * Evaluates a quoted string
     */
    template <typename Eval_System>
    Boxed_Value eval_quoted_string(Eval_System &ss, const TokenPtr &node) {
        //return const_var(node->text);

        /*
        if ((node->text.size() > 0) && (node->text[0] == '$')) {
            node->text.erase(0, 1);
            Param_List_Builder plb;
            plb << node->text;

            return ss.call_function("eval", plb);
        }
        */
        if (!node->is_cached) {
            cache_const(ss, node, const_var(node->text));
        }
        return node->cached_value;
    }

    /**
     * Evaluates a char group
     */
    template <typename Eval_System>
    Boxed_Value eval_single_quoted_string(Eval_System &ss, const TokenPtr &node) {
        if (!node->is_cached) {
            cache_const(ss, node, const_var(char(node->text[0])));
        }
        return node->cached_value;
    }

    /**
     * Evaluates a string of equations in reverse order so that the right-most side has precedence
     */
    template <typename Eval_System>
    Boxed_Value eval_equation(Eval_System &ss, const TokenPtr &node) {
        int i;
        Boxed_Value retval = eval_token(ss, node->children.back());
        if (node->children.size() > 1) {
            for (i = node->children.size()-3; i >= 0; i -= 2) {
                if (node->children[i+1]->text == "=") {
                    Boxed_Value lhs = eval_token(ss, node->children[i]);

                    try {
                        if (lhs.is_undef())
                        {
                          retval = ss.call_function("clone", retval);
                        }

                        try {
                            retval = ss.call_function(node->children[i+1]->text, lhs, retval);
                        }
                        catch(const dispatch_error &){
                            throw Eval_Error(std::string("Mismatched types in equation") + (lhs.is_const()?", lhs is const.":"."), node->children[i+1]);
                        }
                    }
                    catch(const dispatch_error &){
                        throw Eval_Error("Can not clone right hand side of equation", node->children[i+1]);
                    }
                }
                else if (node->children[i+1]->text == ":=") {
                    Boxed_Value lhs = eval_token(ss, node->children[i]);
                    if (lhs.is_undef() || type_match(lhs, retval)) {
                        lhs.assign(retval);
                    }
                    else {
                        throw Eval_Error("Mismatched types in equation", node->children[i+1]);
                    }
                }
                else {
                    try {
                        retval = ss.call_function(node->children[i+1]->text, eval_token(ss, node->children[i]), retval);
                    }
                    catch(const dispatch_error &){
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
    Boxed_Value eval_var_decl(Eval_System &ss, const TokenPtr &node) {
        try {
            ss.add_object(node->children[0]->text, Boxed_Value());
        }
        catch (reserved_word_error &) {
            throw Eval_Error("Reserved word used as variable '" + node->children[0]->text + "'", node);
        }
        return ss.get_object(node->children[0]->text);
    }

    /**
     * Evaluates an attribute declaration
     */
    template <typename Eval_System>
    Boxed_Value eval_attr_decl(Eval_System &ss, const TokenPtr &node) {
        try {
            ss.add(fun(boost::function<Boxed_Value (Dynamic_Object &)>(boost::bind(&Dynamic_Object_Attribute::func, node->children[0]->text,
                    node->children[1]->text, _1))), node->children[1]->text);

        }
        catch (reserved_word_error &) {
            throw Eval_Error("Reserved word used as attribute '" + node->children[1]->text + "'", node);
        }
        return Boxed_Value();
    }

    /**
     * Evaluates binary boolean operators.  Respects short-circuiting rules.
     */
    template <typename Eval_System>
    Boxed_Value eval_logical(Eval_System &ss, const TokenPtr &node) {
        unsigned int i;

        Boxed_Value retval(eval_token(ss, node->children[0]));
        if (node->children.size() > 1) {
            for (i = 1; i < node->children.size(); i += 2) {
                bool lhs;
                try {
                    lhs = boxed_cast<bool>(retval);
                }
                catch (const bad_boxed_cast &) {
                    throw Eval_Error("Condition not boolean", node);
                }
                if (node->children[i]->text == "&&") {
                    if (lhs) {
                        retval = eval_token(ss, node->children[i+1]);
                    }
                    else {
                        retval = Boxed_Value(false);
                    }
                }
                else if (node->children[i]->text == "||") {
                    if (lhs) {
                        retval = Boxed_Value(true);
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
    Boxed_Value eval_comp_add_mul(Eval_System &ss, const TokenPtr &node) {
        unsigned int i;

        Boxed_Value retval = eval_token(ss, node->children[0]);
        for (i = 1; i < node->children.size(); i += 2) {
            try {
                retval = ss.call_function(node->children[i]->text, retval, eval_token(ss, node->children[i + 1]));
            }
            catch(const dispatch_error &){
                throw Eval_Error("Can not find appropriate '" + node->children[i]->text + "'", node->children[i]);
            }
        }

        return retval;
    }

    /**
     * Evaluates an array lookup
     */
    template <typename Eval_System>
    Boxed_Value eval_array_call(Eval_System &ss, const TokenPtr &node) {
        unsigned int i;

        Boxed_Value retval = eval_token(ss, node->children[0]);
        for (i = 1; i < node->children.size(); ++i) {
            try {
                retval = ss.call_function("[]", retval, eval_token(ss, node->children[i]));
            }
            catch(std::out_of_range &) {
                throw Eval_Error("Out of bounds exception", node);
            }
            catch(const dispatch_error &){
                throw Eval_Error("Can not find appropriate array lookup '[]' " + node->children[i]->text, node->children[i]);
            }
        }

        return retval;
    }

    /**
     * Evaluates any unary prefix
     */
    template <typename Eval_System>
    Boxed_Value eval_prefix(Eval_System &ss, const TokenPtr &node) {
        try {
            return ss.call_function(node->children[0]->text, eval_token(ss, node->children[1]));
        }
        catch(std::exception &){
            throw Eval_Error("Can not find appropriate unary '" + node->children[0]->text + "'", node->children[0]);
        }
    }

    /**
     * Evaluates (and generates) an inline array initialization
     */
    template <typename Eval_System>
    Boxed_Value eval_inline_array(Eval_System &ss, const TokenPtr &node) {
        unsigned int i;

        try {
            Boxed_Value retval = ss.call_function("Vector");
	    if (node->children.size() > 0) {
                for (i = 0; i < node->children[0]->children.size(); ++i) {
                    try {
                        ss.call_function("push_back", retval, eval_token(ss, node->children[0]->children[i]));
                    }
                    catch (const dispatch_error &) {
                        throw Eval_Error("Can not find appropriate 'push_back'", node->children[0]->children[i]);
                    }
                }
            }

            return retval;
        }
        catch (const dispatch_error &) {
            throw Eval_Error("Can not find appropriate 'Vector()'", node);
        }

    }

    /**
     * Evaluates (and generates) an inline range initialization
     */
    template <typename Eval_System>
    Boxed_Value eval_inline_range(Eval_System &ss, const TokenPtr &node) {
        try {
            return ss.call_function("generate_range", 
                eval_token(ss, node->children[0]->children[0]->children[0]),
                eval_token(ss, node->children[0]->children[0]->children[1]));
        }
        catch (const dispatch_error &) {
            throw Eval_Error("Unable to generate range vector", node);
        }
    }

    /**
     * Evaluates (and generates) an inline map initialization
     */
    template <typename Eval_System>
    Boxed_Value eval_inline_map(Eval_System &ss, const TokenPtr &node) {
        unsigned int i;

        try {
            Boxed_Value retval = ss.call_function("Map");
            for (i = 0; i < node->children[0]->children.size(); ++i) {
                try {
                    Boxed_Value slot 
                      = ss.call_function("[]", retval, eval_token(ss, node->children[0]->children[i]->children[0]));
                    ss.call_function("=", slot, eval_token(ss, node->children[0]->children[i]->children[1]));
                }
                catch (const dispatch_error &) {
                    throw Eval_Error("Can not find appropriate '=' for map init", node->children[0]->children[i]);
                }
            }
            return retval;
        }
        catch (const dispatch_error &) {
            throw Eval_Error("Can not find appropriate 'Map()'", node);
        }
    }

    /**
     * Evaluates a function call, starting with its arguments.  Handles resetting the scope to the previous one after the call.
     */
    template <typename Eval_System>
    Boxed_Value eval_fun_call(Eval_System &ss, const TokenPtr &node) {
        Param_List_Builder plb;
        Dispatch_Engine::Stack prev_stack = ss.get_stack();
        Dispatch_Engine::Stack new_stack = ss.new_stack();
        unsigned int i;

        if ((node->children.size() > 1) && (node->children[1]->identifier == Token_Type::Arg_List)) {
            for (i = 0; i < node->children[1]->children.size(); ++i) {
                plb << eval_token(ss, node->children[1]->children[i]);
            }
        }

        try {
            Boxed_Value fn = eval_token(ss, node->children[0]);

            try {
                ss.set_stack(new_stack);
                const Boxed_Value &retval = (*boxed_cast<Const_Proxy_Function>(fn))(plb);
                ss.set_stack(prev_stack);
                return retval;
            }
            catch(const dispatch_error &e){
                ss.set_stack(prev_stack);
                throw Eval_Error(std::string(e.what()) + " with function '" + node->children[0]->text + "'", node->children[0]);
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
            ss.set_stack(prev_stack);
            throw Eval_Error(ee.reason, node->children[0]);
        }

    }

    /**
     * Evaluates a function call, starting with its arguments.  Does NOT change scope.
     */
    template <typename Eval_System>
    Boxed_Value eval_inplace_fun_call(Eval_System &ss, const TokenPtr &node) {
        Param_List_Builder plb;
        unsigned int i;

        if ((node->children.size() > 1) && (node->children[1]->identifier == Token_Type::Arg_List)) {
            for (i = 0; i < node->children[1]->children.size(); ++i) {
                plb << eval_token(ss, node->children[1]->children[i]);
            }
        }

        try {
            Boxed_Value fn = eval_token(ss, node->children[0]);

            try {
                return (*boxed_cast<Const_Proxy_Function >(fn))(plb);
            }
            catch(const dispatch_error &e){
                throw Eval_Error(std::string(e.what()) + " with function '" + node->children[0]->text + "'", node->children[0]);
            }
            catch(Return_Value &rv) {
                return rv.retval;
            }
            catch(...) {
                throw;
            }
        }
        catch(Eval_Error &ee) {
            throw Eval_Error(ee.reason, node->children[0]);
        }

    }

    /**
     * Evaluates a method/attributes invocation
     */
    template <typename Eval_System>
    Boxed_Value eval_dot_access(Eval_System &ss, const TokenPtr &node) {
        Dispatch_Engine::Stack prev_stack = ss.get_stack();
        Dispatch_Engine::Stack new_stack = ss.new_stack();
        unsigned int i, j;

        //todo: Please extract a single way of doing function calls between this and eval_fun_call

        Boxed_Value retval(eval_token(ss, node->children[0]));
        if (node->children.size() > 1) {
            for (i = 2; i < node->children.size(); i+=2) {
                Param_List_Builder plb;
                plb << retval;

                if (node->children[i]->children.size() > 1) {
                    for (j = 0; j < node->children[i]->children[1]->children.size(); ++j) {
                        plb << eval_token(ss, node->children[i]->children[1]->children[j]);
                    }
                }

                std::string fun_name;
                std::vector<std::pair<std::string, Proxy_Function > > funs;
                if (node->children[i]->identifier == Token_Type::Fun_Call) {
                    fun_name = node->children[i]->children[0]->text;
                }
                else {
                    fun_name = node->children[i]->text;
                }

                try {
                    ss.set_stack(new_stack);
                    retval = ss.call_function(fun_name, plb);
                    ss.set_stack(prev_stack);
                }
                catch(const dispatch_error &e){
                    ss.set_stack(prev_stack);
                    throw Eval_Error(std::string(e.what()), node->children[i]);
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
    Boxed_Value eval_try(Eval_System &ss, const TokenPtr &node) {
        Boxed_Value retval;        

        ss.new_scope();
        try {
            retval = eval_token(ss, node->children[0]);
        }
        catch (const Eval_Error &) {
            if (node->children.back()->identifier == Token_Type::Finally) {
                eval_token(ss, node->children.back()->children[0]);
            }
            ss.pop_scope();
            throw;
        }
        catch (const std::exception &e) {
            Boxed_Value except = Boxed_Value(boost::ref(e));

            unsigned int end_point = node->children.size();
            if (node->children.back()->identifier == Token_Type::Finally) {
                end_point = node->children.size() - 1;
            }
            for (unsigned int i = 1; i < end_point; ++i) {
                TokenPtr catch_block = node->children[i];

                if (catch_block->children.size() == 1) {
                    //No variable capture, no guards
                    retval = eval_token(ss, catch_block->children[0]);
                    break;
                }
                else if (catch_block->children.size() == 2) {
                    //Variable capture, no guards
                    ss.add_object(catch_block->children[0]->text, except);
                    retval = eval_token(ss, catch_block->children[1]);
                    break;
                }
                else if (catch_block->children.size() == 3) {
                    //Variable capture, no guards
                    ss.add_object(catch_block->children[0]->text, except);

                    bool guard;
                    try {
                        guard = boxed_cast<bool>(eval_token(ss, catch_block->children[1]));
                    } catch (const bad_boxed_cast &) {
                        if (node->children.back()->identifier == Token_Type::Finally) {
                            eval_token(ss, node->children.back()->children[0]);
                        }
                        ss.pop_scope();
                        throw Eval_Error("Guard condition not boolean", catch_block->children[1]);
                    }
                    if (guard) {
                        retval = eval_token(ss, catch_block->children[2]);
                        break;
                    }
                }
                else {
                    if (node->children.back()->identifier == Token_Type::Finally) {
                        eval_token(ss, node->children.back()->children[0]);
                    }
                    ss.pop_scope();
                    throw Eval_Error("Internal error: catch block size unrecognized", catch_block);
                }
            }
        }
        catch (Boxed_Value &bv) {
            Boxed_Value except = bv;
            for (unsigned int i = 1; i < node->children.size(); ++i) {
                TokenPtr catch_block = node->children[i];

                if (catch_block->children.size() == 1) {
                    //No variable capture, no guards
                    retval = eval_token(ss, catch_block->children[0]);
                    break;
                }
                else if (catch_block->children.size() == 2) {
                    //Variable capture, no guards
                    ss.add_object(catch_block->children[0]->text, except);
                    retval = eval_token(ss, catch_block->children[1]);
                    break;
                }
                else if (catch_block->children.size() == 3) {
                    //Variable capture, no guards
                    ss.add_object(catch_block->children[0]->text, except);

                    bool guard;
                    try {
                        guard = boxed_cast<bool>(eval_token(ss, catch_block->children[1]));
                    } catch (const bad_boxed_cast &) {
                        if (node->children.back()->identifier == Token_Type::Finally) {
                            eval_token(ss, node->children.back()->children[0]);
                        }
                        ss.pop_scope();
                        throw Eval_Error("Guard condition not boolean", catch_block->children[1]);
                    }
                    if (guard) {
                        retval = eval_token(ss, catch_block->children[2]);
                        break;
                    }
                }
                else {
                    if (node->children.back()->identifier == Token_Type::Finally) {
                        eval_token(ss, node->children.back()->children[0]);
                    }
                    ss.pop_scope();
                    throw Eval_Error("Internal error: catch block size unrecognized", catch_block);
                }
            }
        }
        catch (...) {
            if (node->children.back()->identifier == Token_Type::Finally) {
                eval_token(ss, node->children.back()->children[0]);
            }
            ss.pop_scope();
            throw;
        }

        if (node->children.back()->identifier == Token_Type::Finally) {
            retval = eval_token(ss, node->children.back()->children[0]);
        }

        ss.pop_scope();

        return retval;
    }

    /**
     * Evaluates an if/elseif/else block
     */
    template <typename Eval_System>
    Boxed_Value eval_if(Eval_System &ss, const TokenPtr &node) {
        unsigned int i;

        bool cond;
        try {
            cond = boxed_cast<bool>(eval_token(ss, node->children[0]));
        }
        catch (const bad_boxed_cast &) {
            throw Eval_Error("If condition not boolean", node->children[0]);
        }
        if (cond) {
            return eval_token(ss, node->children[1]);
        }
        else {
            if (node->children.size() > 2) {
                i = 2;
                while ((!cond) && (i < node->children.size())) {
                    if (node->children[i]->text == "else") {
                        return eval_token(ss, node->children[i+1]);
                    }
                    else if (node->children[i]->text == "else if") {
                        try {
                            cond = boxed_cast<bool>(eval_token(ss, node->children[i+1]));
                        }
                        catch (const bad_boxed_cast &) {
                            throw Eval_Error("'else if' condition not boolean", node->children[i+1]);
                        }
                        if (cond) {
                            return eval_token(ss, node->children[i+2]);
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
    template <typename Eval_System>
    Boxed_Value eval_while(Eval_System &ss, const TokenPtr &node) {
        bool cond;

        ss.new_scope();

        try {
            cond = boxed_cast<bool>(eval_token(ss, node->children[0]));
        }
        catch (const bad_boxed_cast &) {
            ss.pop_scope();
            throw Eval_Error("While condition not boolean", node->children[0]);
        }
        while (cond) {
            try {
                eval_token(ss, node->children[1]);
                try {
                    cond = boxed_cast<bool>(eval_token(ss, node->children[0]));
                }
                catch (const bad_boxed_cast &) {
                    ss.pop_scope();
                    throw Eval_Error("While condition not boolean", node->children[0]);
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
    template <typename Eval_System>
    Boxed_Value eval_for(Eval_System &ss, const TokenPtr &node) {
        bool cond;

        ss.new_scope();

        try {
            if (node->children.size() == 4) {
                eval_token(ss, node->children[0]);
                cond = boxed_cast<bool>(eval_token(ss, node->children[1]));
            }
            else {
                cond = boxed_cast<bool>(eval_token(ss, node->children[0]));
            }
        }
        catch (const bad_boxed_cast &) {
            ss.pop_scope();
            throw Eval_Error("For condition not boolean", node);
        }
        while (cond) {
            try {
                if (node->children.size() == 4) {
                    eval_token(ss, node->children[3]);
                    eval_token(ss, node->children[2]);
                    cond = boxed_cast<bool>(eval_token(ss, node->children[1]));
                }
                else {
                    eval_token(ss, node->children[2]);
                    eval_token(ss, node->children[1]);
                    cond = boxed_cast<bool>(eval_token(ss, node->children[0]));
                }
            }
            catch (const bad_boxed_cast &) {
                ss.pop_scope();
                throw Eval_Error("For condition not boolean", node);
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
    template <typename Eval_System>
    Boxed_Value eval_def(Eval_System &ss, const TokenPtr &node) {
        unsigned int i;

        std::vector<std::string> param_names;
        std::string annotation = node->annotation?node->annotation->text:"";
        boost::shared_ptr<Dynamic_Proxy_Function> guard;
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
            guard = boost::shared_ptr<Dynamic_Proxy_Function>
                (new Dynamic_Proxy_Function(boost::bind(&eval_function<Eval_System>,
                                                                     boost::ref(ss), guardnode,
                                                                     param_names, _1), numparams));
        }

        try {
            ss.add(Proxy_Function
                    (new Dynamic_Proxy_Function(boost::bind(&eval_function<Eval_System>,
                                                                 boost::ref(ss), node->children.back(),
                                                                 param_names, _1), numparams,
                                                     annotation, guard)), function_name);
        }
        catch (reserved_word_error &) {
            throw Eval_Error("Reserved word used as function name '" + function_name + "'", node);
        }
        return Boxed_Value();
    }

    /**
     * Evaluates a function definition
     */
    template <typename Eval_System>
    Boxed_Value eval_def_method(Eval_System &ss, const TokenPtr &node) {
        //TODO: Merge with eval_def cleanly someday?
        unsigned int i;

        std::vector<std::string> param_names;
        std::string annotation = node->annotation?node->annotation->text:"";
        boost::shared_ptr<Dynamic_Proxy_Function> guard;
        size_t numparams;
        std::string class_name = node->children[0]->text;
        std::string function_name = node->children[1]->text;
        TokenPtr guardnode;

        //The first param of a method is always the implied this ptr.
        param_names.push_back("this");

        if ((node->children.size() > 3) && (node->children[2]->identifier == Token_Type::Arg_List)) {
            for (i = 0; i < node->children[2]->children.size(); ++i) {
                param_names.push_back(node->children[2]->children[i]->text);
            }

            if (node->children.size() > 4) {
                guardnode = node->children[3];
            }
        }
        else {
            //no parameters

            if (node->children.size() > 3) {
                guardnode = node->children[2];
            }
        }

        numparams = param_names.size();

        if (guardnode) {
            guard = boost::shared_ptr<Dynamic_Proxy_Function>
                (new Dynamic_Proxy_Function(boost::bind(&eval_function<Eval_System>,
                                                                     boost::ref(ss), guardnode,
                                                                     param_names, _1), numparams));
        }

        try {
            if (function_name == class_name) {
                ss.add(Proxy_Function
                    (new Dynamic_Object_Constructor(class_name, Proxy_Function(new Dynamic_Proxy_Function(boost::bind(&eval_function<Eval_System>,
                                                                 boost::ref(ss), node->children.back(),
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
                    (new Dynamic_Object_Function(class_name, Proxy_Function(new Dynamic_Proxy_Function(boost::bind(&eval_function<Eval_System>,
                                                                 boost::ref(ss), node->children.back(),
                                                                 param_names, _1), numparams,
                                                     annotation, guard)), ti)), function_name);

            }
        }
        catch (reserved_word_error &) {
            throw Eval_Error("Reserved word used as method name '" + function_name + "'", node);
        }
        return Boxed_Value();
    }

    /**
     * Evaluates a lambda (anonymous function)
     */
    template <typename Eval_System>
    Boxed_Value eval_lambda(Eval_System &ss, const TokenPtr &node) {
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

        return Boxed_Value(Proxy_Function(
              new Dynamic_Proxy_Function(
                boost::bind(&eval_function<Eval_System>, boost::ref(ss), node->children.back(), param_names, _1), numparams)));
    }

    /**
     * Evaluates a scoped block.  Handles resetting the scope after the block has completed.
     */
    template <typename Eval_System>
    Boxed_Value eval_block(Eval_System &ss, const TokenPtr &node) {
        unsigned int i;
        unsigned int num_children = node->children.size();

        ss.new_scope();
        for (i = 0; i < num_children; ++i) {
            try {
                const Boxed_Value &retval = eval_token(ss, node->children[i]);

                if (i + 1 == num_children)
                {
                    ss.pop_scope();
                    return retval;
                }
            }
            catch (const chaiscript::Return_Value &/*rv*/) {
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
    template <typename Eval_System>
    Boxed_Value eval_return(Eval_System &ss, const TokenPtr &node) {
        if (node->children.size() > 0) {
            throw Return_Value(eval_token(ss, node->children[0]), node);
        }
        else {
            throw Return_Value(Boxed_Value(), node);
        }
    }

    /**
     * Evaluates a break statement
     */
    template <typename Eval_System>
    Boxed_Value eval_break(Eval_System &, const TokenPtr &node) {
        throw Break_Loop(node);
    }

    /**
     * Top-level evaluation dispatch for all AST node types
     */
    template <typename Eval_System>
    Boxed_Value eval_token(Eval_System &ss, const TokenPtr &node) {
        switch (node->identifier) {
            case (Token_Type::File) :
                return eval_file(ss, node);
            break;

            case (Token_Type::Id) :
                return eval_id(ss, node);
            break;

            case (Token_Type::Float) :
                return eval_float(ss, node);
            break;

            case (Token_Type::Int) :
                return eval_int(ss, node);
            break;

            case (Token_Type::Quoted_String) :
                return eval_quoted_string(ss, node);
            break;

            case (Token_Type::Single_Quoted_String) :
                return eval_single_quoted_string(ss, node);
            break;

            case (Token_Type::Equation) :
                return eval_equation(ss, node);
            break;

            case (Token_Type::Var_Decl) :
                return eval_var_decl(ss, node);
            break;

            case (Token_Type::Logical_And) :
            case (Token_Type::Logical_Or) :
                return eval_logical(ss, node);
            break;

            case (Token_Type::Bitwise_And) :
            case (Token_Type::Bitwise_Xor) :
            case (Token_Type::Bitwise_Or) :
            case (Token_Type::Comparison) :
            case (Token_Type::Equality) :
            case (Token_Type::Additive) :
            case (Token_Type::Multiplicative) :
            case (Token_Type::Shift) :
                return eval_comp_add_mul(ss, node);
            break;

            case (Token_Type::Array_Call) :
                return eval_array_call(ss, node);
            break;

            case (Token_Type::Prefix) :
                return eval_prefix(ss, node);
            break;

            case (Token_Type::Inline_Array) :
                return eval_inline_array(ss, node);
            break;

            case (Token_Type::Inline_Range) :
                return eval_inline_range(ss, node);
            break;

            case (Token_Type::Inline_Map) :
                return eval_inline_map(ss, node);
            break;

            case (Token_Type::Fun_Call) :
                return eval_fun_call(ss, node);
            break;

            case (Token_Type::Inplace_Fun_Call) :
                return eval_inplace_fun_call(ss, node);
            break;

            case (Token_Type::Dot_Access) :
                return eval_dot_access(ss, node);
            break;

            case(Token_Type::Try) :
                return eval_try(ss, node);
            break;

            case(Token_Type::If) :
                return eval_if(ss, node);
            break;

            case(Token_Type::While) :
                return eval_while(ss, node);
            break;

            case(Token_Type::For) :
                return eval_for(ss, node);
            break;

            case (Token_Type::Def) :
                return eval_def(ss, node);
            break;

            case (Token_Type::Method) :
                return eval_def_method(ss, node);
            break;

            case (Token_Type::Attr_Decl) :
                return eval_attr_decl(ss, node);
            break;

            case (Token_Type::Lambda) :
                return eval_lambda(ss, node);
            break;

            case (Token_Type::Block) :
                return eval_block(ss, node);
            break;

            case (Token_Type::Return) :
                return eval_return(ss, node);
            break;

            case (Token_Type::Break) :
                return eval_break(ss, node);
            break;

            default :
                return Boxed_Value();
        }
    }
}
#endif /* CHAISCRIPT_EVAL_HPP_ */
