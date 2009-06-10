// This file is distributed under the BSD License.
// See LICENSE.TXT for details.

#ifndef WESLEY_ENGINE_HPP_
#define WESLEY_ENGINE_HPP_

#include <exception>

//A function that prints any string passed to it

template <typename Eval_Engine>
class Wesley_System {
    Lexer lexer;
    Rule parser;
    Eval_Engine engine;

public:
    Wesley_System() : lexer(build_lexer()), parser(build_parser_rules()), engine(build_eval_system(lexer, parser)) {

    }

    Eval_Engine &get_eval_engine() {
        return engine;
    }

    const Boxed_Value eval(const std::vector<Boxed_Value> &vals) {
        std::string val;

        try {
            val = Cast_Helper<std::string &>()(vals[0]);
        }
        catch (std::exception &e) {
            throw EvalError("Can not evaluate string: " + val, TokenPtr());
        }
        catch (EvalError &ee) {
            throw EvalError("Can not evaluate string: " + val + " reason: " + ee.reason, TokenPtr());
        }
        return evaluate_string(val);
    }

    std::string load_file(const char *filename) {
        std::ifstream infile (filename, std::ios::in | std::ios::ate);

        if (!infile.is_open()) {
            std::string fname = filename;
            throw std::runtime_error("Can not open: " + fname);
        }

        std::streampos size = infile.tellg();
        infile.seekg(0, std::ios::beg);

        std::vector<char> v(size);
        infile.read(&v[0], size);

        std::string ret_val (v.empty() ? std::string() : std::string (v.begin(), v.end()).c_str());

        return ret_val;
    }

    void debug_print(TokenPtr token, std::string prepend) {
        std::cout << prepend << "Token: " << token->text << "(" << tokentype_to_string(token->identifier) << ") @ " << token->filename
            << ": ("  << token->start.line << ", " << token->start.column << ") to ("
            << token->end.line << ", " << token->end.column << ") " << std::endl;

        for (unsigned int i = 0; i < token->children.size(); ++i) {
            debug_print(token->children[i], prepend + "  ");
        }
    }

    void debug_print(std::vector<TokenPtr> &tokens) {
        for (unsigned int i = 0; i < tokens.size(); ++i) {
            debug_print(tokens[i], "");
        }
    }

    Lexer build_lexer() {
        Lexer lexer;
        lexer.set_skip(Pattern("[ \\t]+", TokenType::Whitespace));
        lexer.set_line_sep(Pattern("\\n|\\r\\n", TokenType::Carriage_Return));
        lexer.set_command_sep(Pattern(";|\\r\\n|\\n", TokenType::Semicolon));
        lexer.set_multiline_comment(Pattern("/\\*", TokenType::Comment), Pattern("\\*/", TokenType::Comment));
        lexer.set_singleline_comment(Pattern("//", TokenType::Comment));

        lexer << Pattern("[A-Za-z_]+", TokenType::Identifier);
        lexer << Pattern("[0-9]+\\.[0-9]+", TokenType::Real_Number);
        lexer << Pattern("[0-9]+", TokenType::Integer);
        lexer << Pattern("[!@#$%^&*|\\-+=<>.]+|/[!@#$%^&|\\-+=<>]*", TokenType::Operator);
        lexer << Pattern("\\(", TokenType::Parens_Open);
        lexer << Pattern("\\)", TokenType::Parens_Close);
        lexer << Pattern("\\[", TokenType::Square_Open);
        lexer << Pattern("\\]", TokenType::Square_Close);
        lexer << Pattern("\\{", TokenType::Curly_Open);
        lexer << Pattern("\\}", TokenType::Curly_Close);
        lexer << Pattern(",", TokenType::Comma);
        lexer << Pattern("\"(?:[^\"\\\\]|\\\\.)*\"", TokenType::Quoted_String);
        lexer << Pattern("'(?:[^'\\\\]|\\\\.)*'", TokenType::Single_Quoted_String);

        return lexer;
    }

    Rule build_parser_rules() {
        Rule params;
        Rule block(TokenType::Scoped_Block);
        Rule fundef(TokenType::Function_Def);
        Rule statement;
        Rule equation(TokenType::Equation);
        Rule boolean(TokenType::Boolean);
        Rule comparison(TokenType::Comparison);
        Rule expression(TokenType::Expression);
        Rule term(TokenType::Term);
        Rule factor(TokenType::Factor);
        Rule negate(TokenType::Negate);
        Rule prefix(TokenType::Prefix);

        Rule funcall(TokenType::Fun_Call);
        Rule methodcall(TokenType::Method_Call);
        Rule if_block(TokenType::If_Block);
        Rule while_block(TokenType::While_Block);
        Rule for_block(TokenType::For_Block);
        Rule arraycall(TokenType::Array_Call);
        Rule vardecl(TokenType::Variable_Decl);
        Rule arrayinit(TokenType::Array_Init);

        Rule return_statement(TokenType::Return);
        Rule break_statement(TokenType::Break);

        Rule value;
        Rule statements;
        Rule for_conditions;
        Rule source_elem;
        Rule source_elems;
        Rule statement_list;
        Rule paren_block;

        Rule rule = *(Ign(Id(TokenType::Semicolon))) >> source_elems >> *(Ign(Id(TokenType::Semicolon)));

        source_elems = source_elem >> *(+Ign(Id(TokenType::Semicolon)) >> source_elem);
        source_elem = fundef | statement;
        statement_list = statement >> *(+Ign(Id(TokenType::Semicolon)) >> statement);
        statement = if_block | while_block | for_block | equation;

        if_block = Ign(Str("if")) >> boolean >> block >> *(*Ign(Id(TokenType::Semicolon)) >> Str("elseif") >> boolean >> block) >> ~(*Ign(Id(TokenType::Semicolon)) >> Str("else") >> block);
        while_block = Ign(Str("while")) >> boolean >> block;
        for_block = Ign(Str("for")) >> for_conditions >> block;
        for_conditions = Ign(Id(TokenType::Parens_Open)) >> ~equation >> Ign(Str(";")) >> boolean >> Ign(Str(";")) >> equation >> Ign(Id(TokenType::Parens_Close));

        fundef = Ign(Str("def")) >> Id(TokenType::Identifier) >> ~(Ign(Id(TokenType::Parens_Open)) >> ~params >> Ign(Id(TokenType::Parens_Close))) >>
            block;
        params = Id(TokenType::Identifier) >> *(Ign(Str(",")) >> Id(TokenType::Identifier));
        block = *(Ign(Id(TokenType::Semicolon))) >> Ign(Id(TokenType::Curly_Open)) >> *(Ign(Id(TokenType::Semicolon))) >> ~statement_list >> *(Ign(Id(TokenType::Semicolon))) >> Ign(Id(TokenType::Curly_Close));

        equation = *(((vardecl | arraycall | Id(TokenType::Identifier)) >> Str("=")) |
                ((vardecl | arraycall | Id(TokenType::Identifier)) >> Str("+=")) |
                ((vardecl | arraycall | Id(TokenType::Identifier)) >> Str("-=")) |
                ((vardecl | arraycall | Id(TokenType::Identifier)) >> Str("*=")) |
                ((vardecl | arraycall | Id(TokenType::Identifier)) >> Str("/="))) >> boolean;
        boolean = comparison >> *((Str("&&") >> comparison) | (Str("||") >> comparison));
        comparison = expression >> *((Str("==") >> expression) | (Str("!=") >> expression) | (Str("<") >> expression) |
                (Str("<=") >> expression) |(Str(">") >> expression) | (Str(">=") >> expression));
        expression = term >> *((Str("+") >> term) | (Str("-") >> term));
        term = factor >> *((Str("*") >> factor) | (Str("/") >> factor));
        factor = methodcall | arraycall | value | negate | prefix | (Ign(Str("+")) >> value);
        value =  vardecl | arrayinit | block | paren_block | return_statement | break_statement |
            funcall | Id(TokenType::Identifier) | Id(TokenType::Real_Number) | Id(TokenType::Integer) | Id(TokenType::Quoted_String) |
            Id(TokenType::Single_Quoted_String) ;

        funcall = Id(TokenType::Identifier) >> Ign(Id(TokenType::Parens_Open)) >> ~(boolean >> *(Ign(Str("," )) >> boolean)) >> Ign(Id(TokenType::Parens_Close));
        methodcall = value >> +(Ign(Str(".")) >> funcall);
        negate = Ign(Str("-")) >> boolean;
        prefix = (Str("++") >> (boolean | arraycall)) | (Str("--") >> (boolean | arraycall));
        arraycall = value >> +((Ign(Id(TokenType::Square_Open)) >> boolean >> Ign(Id(TokenType::Square_Close))));

        arrayinit = Ign(Id(TokenType::Square_Open)) >> ~(boolean >> *(Ign(Str(",")) >> boolean))  >> Ign(Id(TokenType::Square_Close));
        vardecl = Ign(Str("var")) >> Id(TokenType::Identifier);
        return_statement = Ign(Str("return")) >> ~boolean;
        break_statement = Wrap(Ign(Str("break")));
        paren_block = (Ign(Id(TokenType::Parens_Open)) >> equation >> Ign(Id(TokenType::Parens_Close)));

        return rule;
    }


    Eval_Engine build_eval_system(Lexer &lexer, Rule &parser) {
        Eval_Engine ss;
        bootstrap(ss);
        bootstrap_vector<std::vector<int> >(ss, "VectorInt");
        bootstrap_vector<std::vector<Boxed_Value> >(ss, "Vector");
        //dump_system(ss);

        ss.register_function(boost::shared_ptr<Proxy_Function>(
              new Dynamic_Proxy_Function(boost::bind(&Wesley_System<Eval_Engine>::eval, boost::ref(*this), _1), 1)), "eval");

        return ss;
    }

    TokenPtr parse(Rule &rule, std::vector<TokenPtr> &tokens, const char *filename) {

        Token_Iterator iter = tokens.begin(), end = tokens.end();
        TokenPtr parent(new Token("Root", TokenType::File, filename));

        std::pair<Token_Iterator, bool> results = rule(iter, end, parent);

        if ((results.second) && (results.first == end)) {
            //debug_print(parent, "");
            return parent;
        }
        else {
            throw ParserError("Parse failed to complete", *(results.first));
            //throw ParserError("Parse failed to complete at: " + (*(results.first))->text , *(results.first));
        }
    }

    Boxed_Value evaluate_string(const std::string &input, const char *filename = "__EVAL__") {
        std::vector<TokenPtr> tokens = lexer.lex(input, filename);
        Boxed_Value value;

        for (unsigned int i = 0; i < tokens.size(); ++i) {
            if ((tokens[i]->identifier == TokenType::Quoted_String) || (tokens[i]->identifier == TokenType::Single_Quoted_String)) {
                tokens[i]->text = tokens[i]->text.substr(1, tokens[i]->text.size()-2);
            }
        }

        //debug_print(tokens);
        try {
            TokenPtr parent = parse(parser, tokens, filename);
            value = eval_token<Eval_Engine>(engine, parent);
        }
        catch (const ReturnValue &rv) {
            value = rv.retval;
        }
        catch (ParserError &pe) {
            if (filename != std::string("__EVAL__")) {
                std::cout << "Parsing error: \"" << pe.reason << "\" in '" << pe.location->filename << "' line: " << pe.location->start.line+1 << std::endl;
            }
            else {
                std::cout << "Parsing error: \"" << pe.reason << "\"" << std::endl;
            }
        }
        catch (EvalError &ee) {
            if (filename != std::string("__EVAL__")) {
                std::cout << "Eval error: \"" << ee.reason << "\" in '" << ee.location->filename << "' line: " << ee.location->start.line+1 << std::endl;
            }
            else {
                std::cout << "Eval error: \"" << ee.reason << "\"" << std::endl;
            }
        }
        catch (std::exception &e) {
            std::cout << "Exception: " << e.what() << std::endl;
        }

        return value;
    }

    Boxed_Value evaluate_file(const char *filename) {
        return evaluate_string(load_file(filename), filename);
    }
};

typedef Wesley_System<BoxedCPP_System> Wesley_Engine;

#endif /* WESLEY_ENGINE_HPP_ */

