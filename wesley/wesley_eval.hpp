// This file is distributed under the BSD License.
// See LICENSE.TXT for details.

#ifndef WESLEY_EVAL_HPP_
#define WESLEY_EVAL_HPP_

struct ParserError {
    std::string reason;
    TokenPtr location;

    ParserError(const std::string &why, const TokenPtr where) : reason(why), location(where){ }
};

struct EvalError {
    std::string reason;
    TokenPtr location;

    EvalError(const std::string &why, const TokenPtr where) : reason(why), location(where) { }
};

struct ReturnValue {
    Boxed_Value retval;
    TokenPtr location;

    ReturnValue(const Boxed_Value &return_value, const TokenPtr where) : retval(return_value), location(where) { }
};

struct BreakLoop {
    TokenPtr location;

    BreakLoop(const TokenPtr where) : location(where) { }
};

template <typename Eval_System>
const Boxed_Value eval_function (Eval_System &ss, TokenPtr node, const std::vector<std::string> &param_names, const std::vector<Boxed_Value> &vals) {
    for (unsigned int i = 0; i < param_names.size(); ++i) {
        ss.add_object(param_names[i], vals[i]);
    }
    return eval_token(ss, node);
}

template <typename Eval_System>
Boxed_Value eval_token(Eval_System &ss, TokenPtr node) {
    Boxed_Value retval;
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
                retval = Boxed_Value(true);
            }
            else if (node->text == "false") {
                retval = Boxed_Value(false);
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
            retval = Boxed_Value(double(atof(node->text.c_str())));
        break;
        case (TokenType::Integer) :
            retval = Boxed_Value(atoi(node->text.c_str()));
        break;
        case (TokenType::Quoted_String) :
            retval = Boxed_Value(node->text);
        break;
        case (TokenType::Single_Quoted_String) :
            retval = Boxed_Value(node->text);
        break;
        case (TokenType::Equation) :
            retval = eval_token(ss, node->children.back());
            if (node->children.size() > 1) {
                for (i = node->children.size()-3; ((int)i) >= 0; i -= 2) {
                    Param_List_Builder plb;
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
            ss.set_object(node->children[0]->text, Boxed_Value());
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
                    Param_List_Builder plb;
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
                Param_List_Builder plb;
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
            Param_List_Builder plb;
            plb << retval;
            plb << Boxed_Value(-1);

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
                cond = Cast_Helper<bool &>()(retval);
            }
            catch (std::exception) {
                throw EvalError("Boolean not('!') condition not boolean", node->children[0]);
            }
            retval = Boxed_Value(!cond);
        }
        break;
        case (TokenType::Prefix) : {
            retval = eval_token(ss, node->children[1]);
            Param_List_Builder plb;
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
                retval = dispatch(ss.get_function("Vector"), Param_List_Builder());
                for (i = 0; i < node->children.size(); ++i) {
                    try {
                        Boxed_Value tmp = eval_token(ss, node->children[i]);
                        dispatch(ss.get_function("push_back"), Param_List_Builder() << retval << tmp);
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
        case (TokenType::Fun_Call) : {

            //BoxedCPP_System::Stack prev_stack = ss.set_stack(BoxedCPP_System::Stack());

            Param_List_Builder plb;
            for (i = 1; i < node->children.size(); ++i) {
                plb << eval_token(ss, node->children[i]);
            }
            try {
                retval = dispatch(ss.get_function(node->children[0]->text), plb);
                //ss.set_stack(prev_stack);
            }
            catch(EvalError &ee) {
                //ss.set_stack(prev_stack);
                throw EvalError(ee.reason, node->children[0]);
            }
            catch(std::exception &e){
                //ss.set_stack(prev_stack);
                throw EvalError("Engine error: " + std::string(e.what()), node->children[0]);
            }
            catch(ReturnValue &rv) {
                //ss.set_stack(prev_stack);
                retval = rv.retval;
            }
        }
        break;
        case (TokenType::Method_Call) : {
            retval = eval_token(ss, node->children[0]);
            if (node->children.size() > 1) {
                for (i = 1; i < node->children.size(); ++i) {
                    Param_List_Builder plb;
                    plb << retval;

                    for (j = 1; j < node->children[i]->children.size(); ++j) {
                        plb << eval_token(ss, node->children[i]->children[j]);
                    }

                    try {
                        retval = dispatch(ss.get_function(node->children[i]->children[0]->text), plb);
                    }
                    catch(EvalError &ee) {
                        throw EvalError(ee.reason, node->children[0]);
                    }
                    catch(std::exception &e){
                        throw EvalError("Can not find appropriate '" + node->children[i]->children[0]->text + "'", node->children[0]);
                    }
                    catch(ReturnValue &rv) {
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
                cond = Cast_Helper<bool &>()(retval);
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
                                cond = Cast_Helper<bool &>()(retval);
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
                cond = Cast_Helper<bool &>()(retval);
            }
            catch (std::exception) {
                throw EvalError("While condition not boolean", node->children[0]);
            }
            while (cond) {
                try {
                    eval_token(ss, node->children[1]);
                    retval = eval_token(ss, node->children[0]);
                    try {
                        cond = Cast_Helper<bool &>()(retval);
                    }
                    catch (std::exception) {
                        throw EvalError("While condition not boolean", node->children[0]);
                    }
                }
                catch (BreakLoop &bl) {
                    cond = false;
                }
            }
            retval = Boxed_Value();
        }
        break;
        case(TokenType::For_Block) : {
            Boxed_Value condition;
            bool cond;

            try {
                if (node->children.size() == 4) {
                    eval_token(ss, node->children[0]);
                    condition = eval_token(ss, node->children[1]);
                }
                else if (node->children.size() == 3){
                    condition = eval_token(ss, node->children[0]);
                }
                cond = Cast_Helper<bool &>()(condition);
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
                    cond = Cast_Helper<bool &>()(condition);

                }
                catch (std::exception &e) {
                    throw EvalError("For condition not boolean", node);
                }
                catch (BreakLoop &bl) {
                    cond = false;
                }
            }
            retval = Boxed_Value();
        }
        break;
        case (TokenType::Function_Def) : {
            unsigned int num_args = node->children.size() - 2;
            std::vector<std::string> param_names;
            for (i = 0; i < num_args; ++i) {
                param_names.push_back(node->children[i+1]->text);
            }

            ss.register_function(boost::shared_ptr<Proxy_Function>(
                  new Dynamic_Proxy_Function(boost::bind(&eval_function<Eval_System>, boost::ref(ss), node->children.back(), param_names, _1))), node->children[0]->text);
        }
        break;
        case (TokenType::Lambda_Def) : {
            unsigned int num_args = node->children.size() - 1;
            std::vector<std::string> param_names;
            for (i = 0; i < num_args; ++i) {
                param_names.push_back(node->children[i]->text);
            }

            //retval = boost::shared_ptr<Proxy_Function>(new Proxy_Function_Impl<boost::function<void (const std::string &)> >(&test));
            retval = Boxed_Value(boost::shared_ptr<Proxy_Function>(new Dynamic_Proxy_Function(
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
                retval = Boxed_Value();
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

#endif /* WESLEY_EVAL_HPP_ */
