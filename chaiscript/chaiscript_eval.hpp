// This file is distributed under the BSD License.
// See LICENSE.TXT for details.

#ifndef CHAISCRIPT_EVAL_HPP_
#define CHAISCRIPT_EVAL_HPP_

#include <map>

namespace chaiscript
{
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
        } catch (const ReturnValue &rv) {
            retval = rv.retval;
            ss.pop_scope();
        } catch (...) {
            ss.pop_scope();
            throw;
        }


        return retval;
    }

    template <typename Eval_System>
    dispatchkit::Boxed_Value eval_token(Eval_System &ss, TokenPtr node) {
        dispatchkit::Boxed_Value retval;
        unsigned int i, j;

        switch (node->identifier) {
            case (Token_Type::File) :
                for (i = 0; i < node->children.size(); ++i) {
                    retval = eval_token(ss, node->children[i]);
                }
            break;
            case (Token_Type::Id) :
                if (node->text == "true") {
                    retval = dispatchkit::Boxed_Value(true);
                }
                else if (node->text == "false") {
                    retval = dispatchkit::Boxed_Value(false);
                }
                else {
                    try {
                        retval = ss.get_object(node->text);
                    }
                    catch (std::exception &e) {
                        throw EvalError("Can not find object: " + node->text, node);
                    }
                }
            break;
            case (Token_Type::Float) :
                retval = dispatchkit::Boxed_Value(double(atof(node->text.c_str())));
            break;
            case (Token_Type::Int) :
                retval = dispatchkit::Boxed_Value(atoi(node->text.c_str()));
            break;
            case (Token_Type::Quoted_String) :
                retval = dispatchkit::Boxed_Value(node->text);
            break;
            case (Token_Type::Single_Quoted_String) :
                retval = dispatchkit::Boxed_Value(node->text);
            break;
            case (Token_Type::Equation) :
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
                                catch(const dispatchkit::dispatch_error &e){
                                    throw EvalError("Can not find appropriate '" + node->children[i+1]->text + "'", node->children[i+1]);
                                }
                            }
                            catch(const dispatchkit::dispatch_error &e){
                                throw EvalError("Can not clone right hand side of equation", node->children[i+1]);
                            }
                        }
                        else if (node->children[i+1]->text == ":=") {
                            dispatchkit::Boxed_Value lhs = eval_token(ss, node->children[i]);
                            if (lhs.is_unknown() || dispatchkit::Bootstrap::type_match(lhs, retval)) {
                                lhs.assign(retval);
                            }
                            else {
                                throw EvalError("Mismatched types in equation", node->children[i+1]);
                            }
                        }
                        else {
                            dispatchkit::Param_List_Builder plb;
                            plb << eval_token(ss, node->children[i]);
                            plb << retval;
                            try {
                                retval = dispatch(ss.get_function(node->children[i+1]->text), plb);
                            }
                            catch(const dispatchkit::dispatch_error &e){
                                throw EvalError("Can not find appropriate '" + node->children[i+1]->text + "'", node->children[i+1]);
                            }
                        }
                    }
                }
            break;
            case (Token_Type::Var_Decl): {
                ss.add_object(node->children[0]->text, dispatchkit::Boxed_Value());
                retval = ss.get_object(node->children[0]->text);
            }
            break;
            case (Token_Type::Expression) : {
                retval = eval_token(ss, node->children[0]);
                if (node->children.size() > 1) {
                    for (i = 1; i < node->children.size(); i += 2) {
                        bool lhs;
                        try {
                            lhs = dispatchkit::boxed_cast<bool &>(retval);
                        }
                        catch (std::exception &e) {
                            throw EvalError("Condition not boolean", node);
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
            }
            break;
            case (Token_Type::Comparison) :
            case (Token_Type::Additive) :
            case (Token_Type::Multiplicative) : {
                retval = eval_token(ss, node->children[0]);
                if (node->children.size() > 1) {
                    for (i = 1; i < node->children.size(); i += 2) {
                        dispatchkit::Param_List_Builder plb;
                        plb << retval;
                        plb << eval_token(ss, node->children[i + 1]);

                        try {
                            retval = dispatch(ss.get_function(node->children[i]->text), plb);
                        }
                        catch(const dispatchkit::dispatch_error &e){
                            throw EvalError("Can not find appropriate '" + node->children[i]->text + "'", node->children[i]);
                        }
                    }
                }
            }
            break;
            case (Token_Type::Array_Call) : {
                retval = eval_token(ss, node->children[0]);
                for (i = 1; i < node->children.size(); ++i) {
                    dispatchkit::Param_List_Builder plb;
                    plb << retval;
                    plb << eval_token(ss, node->children[i]);
                    try {
                        retval = dispatch(ss.get_function("[]"), plb);
                    }
                    catch(std::out_of_range &oor) {
                        throw EvalError("Out of bounds exception", node);
                    }
                    catch(const dispatchkit::dispatch_error &e){
                        throw EvalError("Can not find appropriate array lookup '[]' " + node->children[i]->text, node->children[i]);
                    }
                }
            }
            break;
            case (Token_Type::Negate) : {
                retval = eval_token(ss, node->children[0]);
                dispatchkit::Param_List_Builder plb;
                plb << retval;
                plb << dispatchkit::Boxed_Value(-1);

                try {
                    retval = dispatch(ss.get_function("*"), plb);
                }
                catch(std::exception &e){
                    throw EvalError("Can not find appropriate negation", node->children[0]);
                }
            }
            break;
            case (Token_Type::Not) : {
                bool cond;
                try {
                    retval = eval_token(ss, node->children[0]);
                    cond = dispatchkit::boxed_cast<bool &>(retval);
                }
                catch (std::exception) {
                    throw EvalError("Boolean not('!') condition not boolean", node->children[0]);
                }
                retval = dispatchkit::Boxed_Value(!cond);
            }
            break;
            case (Token_Type::Prefix) : {
                retval = eval_token(ss, node->children[1]);
                dispatchkit::Param_List_Builder plb;
                plb << retval;

                try {
                    retval = dispatch(ss.get_function(node->children[0]->text), plb);
                }
                catch(std::exception &e){
                    throw EvalError("Can not find appropriate prefix", node->children[0]);
                }
            }
            break;
            case (Token_Type::Inline_Array) : {
                try {
                    retval = dispatch(ss.get_function("Vector"), dispatchkit::Param_List_Builder());
                    if (node->children.size() > 0) {
                        for (i = 0; i < node->children[0]->children.size(); ++i) {
                            try {
                                dispatchkit::Boxed_Value tmp = eval_token(ss, node->children[0]->children[i]);
                                dispatch(ss.get_function("push_back"), dispatchkit::Param_List_Builder() << retval << tmp);
                            }
                            catch (const dispatchkit::dispatch_error &inner_e) {
                                throw EvalError("Can not find appropriate 'push_back'", node->children[0]->children[i]);
                            }
                        }
                    }
                }
                catch (const dispatchkit::dispatch_error &e) {
                    throw EvalError("Can not find appropriate 'Vector()'", node);
                }
            }
            break;
            case (Token_Type::Inline_Map) : {
                try {
                    retval = dispatch(ss.get_function("Map"), dispatchkit::Param_List_Builder());
                    for (i = 0; i < node->children[0]->children.size(); ++i) {
                        try {
                            dispatchkit::Boxed_Value key = eval_token(ss, node->children[0]->children[i]->children[0]);
                            dispatchkit::Boxed_Value slot = dispatch(ss.get_function("[]"), dispatchkit::Param_List_Builder() << retval << key);
                            dispatch(ss.get_function("="), dispatchkit::Param_List_Builder() << slot << eval_token(ss, node->children[0]->children[i]->children[1]));
                        }
                        catch (const dispatchkit::dispatch_error &inner_e) {
                            throw EvalError("Can not find appropriate '=' for map init", node->children[0]->children[i]);
                        }
                    }
                }
                catch (const dispatchkit::dispatch_error &e) {
                    throw EvalError("Can not find appropriate 'Map()'", node);
                }
            }
            break;
            case (Token_Type::Fun_Call) : {
                dispatchkit::Param_List_Builder plb;

                //std::vector<std::pair<std::string, dispatchkit::Dispatch_Engine::Function_Map::mapped_type> > fn;
                dispatchkit::Dispatch_Engine::Stack prev_stack = ss.get_stack();

                dispatchkit::Dispatch_Engine::Stack new_stack;
                new_stack.push_back(dispatchkit::Dispatch_Engine::Scope());

                if ((node->children.size() > 1) && (node->children[1]->identifier == Token_Type::Arg_List)) {
                    for (i = 0; i < node->children[1]->children.size(); ++i) {
                        plb << eval_token(ss, node->children[1]->children[i]);
                    }
                }
                try {
                    dispatchkit::Boxed_Value fn = eval_token(ss, node->children[0]);
                    //fn = ss.get_function(node->children[0]->text);
                    ss.set_stack(new_stack);
                    //retval = dispatch(fn, plb);
                    //retval = dispatch
                    retval = (*dispatchkit::boxed_cast<boost::shared_ptr<dispatchkit::Proxy_Function> >(fn))(plb);
                    ss.set_stack(prev_stack);
                }
                catch(EvalError &ee) {
                    ss.set_stack(prev_stack);
                    throw EvalError(ee.reason, node->children[0]);
                }
                catch(const dispatchkit::dispatch_error &e){
                    ss.set_stack(prev_stack);
                    throw EvalError("Engine error: " + std::string(e.what()) + " with function '" + node->children[0]->text + "'", node->children[0]);
                }
                catch(ReturnValue &rv) {
                    ss.set_stack(prev_stack);
                    retval = rv.retval;
                }
                catch(...) {
                    ss.set_stack(prev_stack);
                    throw;
                }
            }
            break;

            case (Token_Type::Dot_Access) : {
                std::vector<std::pair<std::string, dispatchkit::Dispatch_Engine::Function_Map::mapped_type> > fn;
                dispatchkit::Dispatch_Engine::Stack prev_stack = ss.get_stack();

                dispatchkit::Dispatch_Engine::Stack new_stack;
                new_stack.push_back(dispatchkit::Dispatch_Engine::Scope());

                retval = eval_token(ss, node->children[0]);
                if (node->children.size() > 1) {
                    for (i = 1; i < node->children.size(); ++i) {
                        dispatchkit::Param_List_Builder plb;
                        plb << retval;

                        if (node->children[i]->children.size() > 1) {
                            //std::cout << "size: " << node->children[i]->children.size() << std::endl;
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
                        catch(EvalError &ee) {
                            ss.set_stack(prev_stack);
                            throw EvalError(ee.reason, node);
                        }
                        catch(const dispatchkit::dispatch_error &e){
                            ss.set_stack(prev_stack);
                            throw EvalError("Can not find appropriate '" + fun_name + "'", node);
                        }
                        catch(ReturnValue &rv) {
                            ss.set_stack(prev_stack);
                            retval = rv.retval;
                        }
                        catch(...) {
                            ss.set_stack(prev_stack);
                            throw;
                        }
                    }
                }
            }
            break;

            case(Token_Type::If) : {
                retval = eval_token(ss, node->children[0]);
                bool cond;
                try {
                    cond = dispatchkit::boxed_cast<bool &>(retval);
                }
                catch (std::exception &e) {
                    throw EvalError("If condition not boolean", node->children[0]);
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
                                catch (std::exception &e) {
                                    throw EvalError("Elseif condition not boolean", node->children[i+1]);
                                }
                                if (cond) {
                                    retval = eval_token(ss, node->children[i+2]);
                                }
                            }
                            i = i + 3;
                        }
                    }
                }
            }
            break;
            case(Token_Type::While) : {
                retval = eval_token(ss, node->children[0]);
                bool cond;
                try {
                    cond = dispatchkit::boxed_cast<bool &>(retval);
                }
                catch (std::exception) {
                    throw EvalError("While condition not boolean", node->children[0]);
                }
                while (cond) {
                    try {
                        eval_token(ss, node->children[1]);
                        retval = eval_token(ss, node->children[0]);
                        try {
                            cond = dispatchkit::boxed_cast<bool &>(retval);
                        }
                        catch (std::exception) {
                            throw EvalError("While condition not boolean", node->children[0]);
                        }
                    }
                    catch (BreakLoop &bl) {
                        cond = false;
                    }
                }
                retval = dispatchkit::Boxed_Value();
            }
            break;
            case(Token_Type::For) : {
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
                catch (std::exception &e) {
                    throw EvalError("For condition not boolean", node);
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
                    catch (std::exception &e) {
                        throw EvalError("For condition not boolean", node);
                    }
                    catch (BreakLoop &bl) {
                        cond = false;
                    }
                }
                retval = dispatchkit::Boxed_Value();
            }
            break;
            case (Token_Type::Def) : {
                std::vector<std::string> param_names;

                if ((node->children.size() > 2) && (node->children[1]->identifier == Token_Type::Arg_List)) {
                    for (i = 0; i < node->children[1]->children.size(); ++i) {
                        param_names.push_back(node->children[1]->children[i]->text);
                    }

                    if (node->children.size() > 3) {
                        //Guarded function
                        boost::shared_ptr<dispatchkit::Dynamic_Proxy_Function> guard = boost::shared_ptr<dispatchkit::Dynamic_Proxy_Function>
                            (new dispatchkit::Dynamic_Proxy_Function(boost::bind(&eval_function<Eval_System>, boost::ref(ss), node->children[2], param_names, _1), node->children[1]->children.size()));

                        ss.register_function(boost::shared_ptr<dispatchkit::Proxy_Function>(
                              new dispatchkit::Dynamic_Proxy_Function(boost::bind(&eval_function<Eval_System>, boost::ref(ss), node->children.back(), param_names, _1), node->children[1]->children.size(),
                              guard)), node->children[0]->text);

                    }
                    else {
                        ss.register_function(boost::shared_ptr<dispatchkit::Proxy_Function>(
                              new dispatchkit::Dynamic_Proxy_Function(boost::bind(&eval_function<Eval_System>, boost::ref(ss), node->children.back(), param_names, _1), node->children[1]->children.size())), node->children[0]->text);
                    }
                }
                else {
                    //no parameters

                    if (node->children.size() > 2) {
                        boost::shared_ptr<dispatchkit::Dynamic_Proxy_Function> guard = boost::shared_ptr<dispatchkit::Dynamic_Proxy_Function>
                            (new dispatchkit::Dynamic_Proxy_Function(boost::bind(&eval_function<Eval_System>, boost::ref(ss), node->children[1], param_names, _1), 0));

                        ss.register_function(boost::shared_ptr<dispatchkit::Proxy_Function>(
                              new dispatchkit::Dynamic_Proxy_Function(boost::bind(&eval_function<Eval_System>, boost::ref(ss), node->children.back(), param_names, _1), 0,
                              guard)), node->children[0]->text);
                    }
                    else {
                        ss.register_function(boost::shared_ptr<dispatchkit::Proxy_Function>(
                          new dispatchkit::Dynamic_Proxy_Function(boost::bind(&eval_function<Eval_System>, boost::ref(ss), node->children.back(), param_names, _1), 0)), node->children[0]->text);
                    }
                }
            }
            break;
            case (Token_Type::Lambda) : {
                std::vector<std::string> param_names;

                if ((node->children.size() > 0) && (node->children[0]->identifier == Token_Type::Arg_List)) {
                    for (i = 0; i < node->children[0]->children.size(); ++i) {
                        param_names.push_back(node->children[0]->children[i]->text);
                    }
                    //retval = boost::shared_ptr<dispatchkit::Proxy_Function>(new dispatchkit::Proxy_Function_Impl<boost::function<void (const std::string &)> >(&test));
                    retval = dispatchkit::Boxed_Value(boost::shared_ptr<dispatchkit::Proxy_Function>(
                          new dispatchkit::Dynamic_Proxy_Function(
                            boost::bind(&eval_function<Eval_System>, boost::ref(ss), node->children.back(), param_names, _1), node->children[0]->children.size())));
                }
                else {
                    //no parameters
                    retval = dispatchkit::Boxed_Value(boost::shared_ptr<dispatchkit::Proxy_Function>(
                          new dispatchkit::Dynamic_Proxy_Function(
                            boost::bind(&eval_function<Eval_System>, boost::ref(ss), node->children.back(), param_names, _1), 0)));
                }
            }
            break;

            case (Token_Type::Block) : {
                ss.new_scope();
                for (i = 0; i < node->children.size(); ++i) {
                    try {
                        retval = eval_token(ss, node->children[i]);
                    }
                    catch (const chaiscript::ReturnValue &rv) {
                        retval = rv.retval;
                        break;
                    }
                    catch (...) {
                        ss.pop_scope();
                        throw;
                    }
                }
                ss.pop_scope();
            }
            break;

            case (Token_Type::Return) : {
                if (node->children.size() > 0) {
                    retval = eval_token(ss, node->children[0]);
                }
                else {
                    retval = dispatchkit::Boxed_Value();
                }
                throw ReturnValue(retval, node);
            }
            break;
            case (Token_Type::Break) : {
                throw BreakLoop(node);
            }
            break;
        }

        return retval;
    }
}
#endif /* CHAISCRIPT_EVAL_HPP_ */
