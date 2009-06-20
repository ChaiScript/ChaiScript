// This file is distributed under the BSD License.
// See LICENSE.TXT for details.

#ifndef CHAISCRIPT_EVAL_HPP_
#define CHAISCRIPT_EVAL_HPP_

#include <map>

namespace chaiscript
{
    struct ParserError {
        std::string reason;
        langkit::TokenPtr location;

        ParserError(const std::string &why, const langkit::TokenPtr where) : reason(why), location(where){ }
    };

    struct EvalError {
        std::string reason;
        langkit::TokenPtr location;

        EvalError(const std::string &why, const langkit::TokenPtr where) : reason(why), location(where) { }
    };

    struct ReturnValue {
        dispatchkit::Boxed_Value retval;
        langkit::TokenPtr location;

        ReturnValue(const dispatchkit::Boxed_Value &return_value, const langkit::TokenPtr where) : retval(return_value), location(where) { }
    };

    struct BreakLoop {
        langkit::TokenPtr location;

        BreakLoop(const langkit::TokenPtr where) : location(where) { }
    };

    template <typename Eval_System>
    const dispatchkit::Boxed_Value eval_function (Eval_System &ss, langkit::TokenPtr node, const std::vector<std::string> &param_names, const std::vector<dispatchkit::Boxed_Value> &vals) {
        ss.new_scope();

        for (unsigned int i = 0; i < param_names.size(); ++i) {
            ss.add_object(param_names[i], vals[i]);
        }

        dispatchkit::Boxed_Value retval = eval_token(ss, node);
        ss.pop_scope();
        return retval;
    }

    template <typename Eval_System>
    dispatchkit::Boxed_Value eval_token(Eval_System &ss, langkit::TokenPtr node) {
        dispatchkit::Boxed_Value retval;
        unsigned int i, j;

        switch (node->identifier) {
            case (TokenType::Value) :
            case (TokenType::File) :
                for (i = 0; i < node->children.size(); ++i) {
                    retval = eval_token(ss, node->children[i]);
                }
            break;
            case (TokenType::Identifier) :
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
            case (TokenType::Real_Number) :
                retval = dispatchkit::Boxed_Value(double(atof(node->text.c_str())));
            break;
            case (TokenType::Integer) :
                retval = dispatchkit::Boxed_Value(atoi(node->text.c_str()));
            break;
            case (TokenType::Quoted_String) :
                retval = dispatchkit::Boxed_Value(node->text);
            break;
            case (TokenType::Single_Quoted_String) :
                retval = dispatchkit::Boxed_Value(node->text);
            break;
            case (TokenType::Equation) :
                retval = eval_token(ss, node->children.back());
                if (node->children.size() > 1) {
                    for (i = node->children.size()-3; ((int)i) >= 0; i -= 2) {
                        dispatchkit::Param_List_Builder plb;
                        plb << eval_token(ss, node->children[i]);
                        plb << retval;
                        try {
                            retval = dispatch(ss.get_function(node->children[i+1]->text), plb);
                        }
                        catch(std::exception &e){
                            throw EvalError("Can not find appropriate '" + node->children[i+1]->text + "'", node->children[i+1]);
                        }
                    }
                }
            break;
            case (TokenType::Variable_Decl): {
                ss.add_object(node->children[0]->text, dispatchkit::Boxed_Value());
                retval = ss.get_object(node->children[0]->text);
            }
            break;
            case (TokenType::Factor) :
            case (TokenType::Expression) :
            case (TokenType::Term) :
            case (TokenType::Boolean) :
            case (TokenType::Comparison) : {
                retval = eval_token(ss, node->children[0]);
                if (node->children.size() > 1) {
                    for (i = 1; i < node->children.size(); i += 2) {
                        dispatchkit::Param_List_Builder plb;
                        plb << retval;
                        plb << eval_token(ss, node->children[i + 1]);

                        try {
                            retval = dispatch(ss.get_function(node->children[i]->text), plb);
                        }
                        catch(std::exception &e){
                            throw EvalError("Can not find appropriate '" + node->children[i]->text + "'", node->children[i]);
                        }
                    }
                }
            }
            break;
            case (TokenType::Array_Call) : {
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
                    catch(std::exception &e){
                        throw EvalError("Can not find appropriate array lookup '[]'", node->children[i]);
                    }
                }
            }
            break;
            case (TokenType::Negate) : {
                retval = eval_token(ss, node->children[1]);
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
            case (TokenType::Not) : {
                bool cond;
                try {
                    retval = eval_token(ss, node->children[1]);
                    cond = dispatchkit::Cast_Helper<bool &>()(retval);
                }
                catch (std::exception) {
                    throw EvalError("Boolean not('!') condition not boolean", node->children[0]);
                }
                retval = dispatchkit::Boxed_Value(!cond);
            }
            break;
            case (TokenType::Prefix) : {
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
            case (TokenType::Array_Init) : {
                try {
                    retval = dispatch(ss.get_function("Vector"), dispatchkit::Param_List_Builder());
                    for (i = 0; i < node->children.size(); ++i) {
                        try {
                            dispatchkit::Boxed_Value tmp = eval_token(ss, node->children[i]);
                            dispatch(ss.get_function("push_back"), dispatchkit::Param_List_Builder() << retval << tmp);
                        }
                        catch (std::exception inner_e) {
                            throw EvalError("Can not find appropriate 'push_back'", node->children[i]);
                        }
                    }
                }
                catch (std::exception e) {
                    throw EvalError("Can not find appropriate 'Vector()'", node);
                }
            }
            break;
            case (TokenType::Map_Init) : {
                try {
                    retval = dispatch(ss.get_function("Map"), dispatchkit::Param_List_Builder());
                    for (i = 0; i < node->children.size(); ++i) {
                        try {
                            dispatchkit::Boxed_Value key = eval_token(ss, node->children[i]->children[0]);
                            dispatchkit::Boxed_Value slot = dispatch(ss.get_function("[]"), dispatchkit::Param_List_Builder() << retval << key);
                            dispatch(ss.get_function("="), dispatchkit::Param_List_Builder() << slot << eval_token(ss, node->children[i]->children[1]));
                        }
                        catch (std::exception inner_e) {
                            throw EvalError("Can not find appropriate '=' for map init", node->children[i]);
                        }
                    }
                }
                catch (std::exception e) {
                    throw EvalError("Can not find appropriate 'Vector()'", node);
                }
            }
            break;
            case (TokenType::Fun_Call) : {

                std::vector<std::pair<std::string, dispatchkit::Dispatch_Engine::Function_Map::mapped_type> > fn;
                dispatchkit::Dispatch_Engine::Stack prev_stack = ss.get_stack();

                dispatchkit::Dispatch_Engine::Stack new_stack;
                new_stack.push_back(dispatchkit::Dispatch_Engine::Scope());

                dispatchkit::Param_List_Builder plb;
                for (i = 1; i < node->children.size(); ++i) {
                    plb << eval_token(ss, node->children[i]);
                }
                try {
                    fn = ss.get_function(node->children[0]->text);
                    ss.set_stack(new_stack);
                    retval = dispatch(fn, plb);
                    ss.set_stack(prev_stack);
                }
                catch(EvalError &ee) {
                    ss.set_stack(prev_stack);
                    throw EvalError(ee.reason, node->children[0]);
                }
                catch(std::exception &e){
                    ss.set_stack(prev_stack);
                    throw EvalError("Engine error: " + std::string(e.what()) + " on '" + node->children[0]->text + "'", node->children[0]);
                }
                catch(ReturnValue &rv) {
                    ss.set_stack(prev_stack);
                    retval = rv.retval;
                }
            }
            break;
            case (TokenType::Method_Call) : {
                std::vector<std::pair<std::string, dispatchkit::Dispatch_Engine::Function_Map::mapped_type> > fn;
                dispatchkit::Dispatch_Engine::Stack prev_stack = ss.get_stack();

                dispatchkit::Dispatch_Engine::Stack new_stack;
                new_stack.push_back(dispatchkit::Dispatch_Engine::Scope());

                retval = eval_token(ss, node->children[0]);
                if (node->children.size() > 1) {
                    for (i = 1; i < node->children.size(); ++i) {
                        dispatchkit::Param_List_Builder plb;
                        plb << retval;

                        for (j = 1; j < node->children[i]->children.size(); ++j) {
                            plb << eval_token(ss, node->children[i]->children[j]);
                        }

                        std::string fun_name;
                        if (node->children[i]->identifier == TokenType::Fun_Call) {
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
                        catch(std::exception &e){
                            ss.set_stack(prev_stack);
                            throw EvalError("Can not find appropriate '" + fun_name + "'", node);
                        }
                        catch(ReturnValue &rv) {
                            ss.set_stack(prev_stack);
                            retval = rv.retval;
                        }
                    }
                }
            }
            break;
            case(TokenType::If_Block) : {
                retval = eval_token(ss, node->children[0]);
                bool cond;
                try {
                    cond = dispatchkit::Cast_Helper<bool &>()(retval);
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
                                    cond = dispatchkit::Cast_Helper<bool &>()(retval);
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
            case(TokenType::While_Block) : {
                retval = eval_token(ss, node->children[0]);
                bool cond;
                try {
                    cond = dispatchkit::Cast_Helper<bool &>()(retval);
                }
                catch (std::exception) {
                    throw EvalError("While condition not boolean", node->children[0]);
                }
                while (cond) {
                    try {
                        eval_token(ss, node->children[1]);
                        retval = eval_token(ss, node->children[0]);
                        try {
                            cond = dispatchkit::Cast_Helper<bool &>()(retval);
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
            case(TokenType::For_Block) : {
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
                    cond = dispatchkit::Cast_Helper<bool &>()(condition);
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
                        cond = dispatchkit::Cast_Helper<bool &>()(condition);

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
            case (TokenType::Function_Def) : {
                unsigned int num_args = node->children.size() - 2;
                std::vector<std::string> param_names;
                for (i = 0; i < num_args; ++i) {
                    param_names.push_back(node->children[i+1]->text);
                }

                ss.register_function(boost::shared_ptr<dispatchkit::Proxy_Function>(
                      new dispatchkit::Dynamic_Proxy_Function(boost::bind(&eval_function<Eval_System>, boost::ref(ss), node->children.back(), param_names, _1))), node->children[0]->text);
            }
            break;
            case (TokenType::Lambda_Def) : {
                unsigned int num_args = node->children.size() - 1;
                std::vector<std::string> param_names;
                for (i = 0; i < num_args; ++i) {
                    param_names.push_back(node->children[i]->text);
                }

                //retval = boost::shared_ptr<dispatchkit::Proxy_Function>(new dispatchkit::Proxy_Function_Impl<boost::function<void (const std::string &)> >(&test));
                retval = dispatchkit::Boxed_Value(boost::shared_ptr<dispatchkit::Proxy_Function>(
                      new dispatchkit::Dynamic_Proxy_Function(
                        boost::bind(&eval_function<Eval_System>, boost::ref(ss), node->children.back(), param_names, _1))));
            }
            break;
            case (TokenType::Scoped_Block) : {
                ss.new_scope();
                for (i = 0; i < node->children.size(); ++i) {
                    retval = eval_token(ss, node->children[i]);
                }
                ss.pop_scope();
            }
            break;
            case (TokenType::Return) : {
                if (node->children.size() > 0) {
                    retval = eval_token(ss, node->children[0]);
                }
                else {
                    retval = dispatchkit::Boxed_Value();
                }
                throw ReturnValue(retval, node);
            }
            break;
            case (TokenType::Break) : {
                throw BreakLoop(node);
            }
            break;
            case (TokenType::Statement) :
            case (TokenType::Carriage_Return) :
            case (TokenType::Semicolon) :
            case (TokenType::Comment) :
            case (TokenType::Operator) :
            case (TokenType::Whitespace) :
            case (TokenType::Parens_Open) :
            case (TokenType::Parens_Close) :
            case (TokenType::Square_Open) :
            case (TokenType::Square_Close) :
            case (TokenType::Curly_Open) :
            case (TokenType::Curly_Close) :
            case (TokenType::Comma) :
            break;
        }

        return retval;
    }
}
#endif /* CHAISCRIPT_EVAL_HPP_ */
