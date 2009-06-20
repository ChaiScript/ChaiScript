// This file is distributed under the BSD License.
// See LICENSE.TXT for details.

#ifndef CHAISCRIPT_ENGINE_HPP_
#define CHAISCRIPT_ENGINE_HPP_

#include <exception>

namespace chaiscript
{
    template <typename Eval_Engine>
    class ChaiScript_System {
        langkit::Lexer lexer;
        langkit::Rule parser;
        Eval_Engine engine;

    public:
        ChaiScript_System() : lexer(build_lexer()), parser(build_parser_rules()), engine(build_eval_system(lexer, parser)) {

        }

        Eval_Engine &get_eval_engine() {
            return engine;
        }

        const dispatchkit::Boxed_Value eval(const std::vector<dispatchkit::Boxed_Value> &vals) {
            std::string val;

            try {
                val = dispatchkit::Cast_Helper<std::string &>()(vals[0]);
            }
            catch (std::exception &e) {
                throw EvalError("Can not evaluate string: " + val, langkit::TokenPtr());
            }
            catch (EvalError &ee) {
                throw EvalError("Can not evaluate string: " + val + " reason: " + ee.reason, langkit::TokenPtr());
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

        void debug_print(langkit::TokenPtr token, std::string prepend) {
            using namespace langkit;
            std::cout << prepend << "Token: " << token->text << "(" << tokentype_to_string(token->identifier) << ") @ " << token->filename
                << ": ("  << token->start.line << ", " << token->start.column << ") to ("
                << token->end.line << ", " << token->end.column << ") " << std::endl;

            for (unsigned int i = 0; i < token->children.size(); ++i) {
                debug_print(token->children[i], prepend + "  ");
            }
        }

        void debug_print(std::vector<langkit::TokenPtr> &tokens) {
            using namespace langkit;
            for (unsigned int i = 0; i < tokens.size(); ++i) {
                debug_print(tokens[i], "");
            }
        }

        langkit::Lexer build_lexer() {
            using namespace langkit;
            Lexer lexer;
            lexer.set_skip(Pattern("[ \\t]+", TokenType::Whitespace));
            lexer.set_line_sep(Pattern("\\n|\\r\\n", TokenType::Carriage_Return));
            lexer.set_command_sep(Pattern(";|\\r\\n|\\n", TokenType::Semicolon));
            lexer.set_multiline_comment(Pattern("/\\*", TokenType::Comment), Pattern("\\*/", TokenType::Comment));
            lexer.set_singleline_comment(Pattern("//", TokenType::Comment));

            lexer << Pattern("[A-Za-z_][A-Za-z_0-9]*", TokenType::Identifier);
            lexer << Pattern("[0-9]+\\.[0-9]+", TokenType::Real_Number);
            lexer << Pattern("[0-9]+", TokenType::Integer);
            lexer << Pattern("[!@#$%^&*|\\-+=<>.:]+|/[!@#$%^&|\\-+=<>]*", TokenType::Operator);
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

        langkit::Rule build_parser_rules() {
            using namespace langkit;
            Rule params;
            Rule block(TokenType::Scoped_Block);
            Rule fundef(TokenType::Function_Def);
            Rule lambda_def(TokenType::Lambda_Def);
            Rule statement;
            Rule equation(TokenType::Equation);
            Rule boolean(TokenType::Boolean);
            Rule comparison(TokenType::Comparison);
            Rule expression(TokenType::Expression);
            Rule term(TokenType::Term);
            Rule factor(TokenType::Factor);
            Rule negate(TokenType::Negate);
            Rule boolean_not(TokenType::Not);
            Rule prefix(TokenType::Prefix);

            Rule funcall(TokenType::Fun_Call);
            Rule methodcall(TokenType::Method_Call);
            Rule if_block(TokenType::If_Block);
            Rule while_block(TokenType::While_Block);
            Rule for_block(TokenType::For_Block);
            Rule arraycall(TokenType::Array_Call);
            Rule vardecl(TokenType::Variable_Decl);
            Rule arrayinit(TokenType::Array_Init);
            Rule mapinit(TokenType::Map_Init);
            Rule mappair(TokenType::Map_Pair);

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
            factor = methodcall | arraycall | value | negate | boolean_not | prefix | (Ign(Str("+")) >> value);
            value =  vardecl | mapinit | arrayinit | block | paren_block | lambda_def | return_statement | break_statement |
                funcall | Id(TokenType::Identifier) | Id(TokenType::Real_Number) | Id(TokenType::Integer) | Id(TokenType::Quoted_String) |
                Id(TokenType::Single_Quoted_String) ;

            funcall = Id(TokenType::Identifier) >> Ign(Id(TokenType::Parens_Open)) >> ~(boolean >> *(Ign(Str("," )) >> boolean)) >> Ign(Id(TokenType::Parens_Close));
            methodcall = value >> +(Ign(Str(".")) >> funcall);
            negate = (Str("-") >> (boolean | arraycall));
            boolean_not = (Str("!") >> (boolean | arraycall));
            prefix = (Str("++") >> (boolean | arraycall)) | (Str("--") >> (boolean | arraycall));
            arraycall = value >> +((Ign(Id(TokenType::Square_Open)) >> boolean >> Ign(Id(TokenType::Square_Close))));

            arrayinit = Ign(Id(TokenType::Square_Open)) >> ~(boolean >> *(Ign(Str(",")) >> boolean))  >> Ign(Id(TokenType::Square_Close));
            mapinit = Ign(Id(TokenType::Square_Open)) >> ~(mappair >> *(Ign(Str(",")) >> mappair))  >> Ign(Id(TokenType::Square_Close));
            mappair = Id(TokenType::Quoted_String) >> Ign(Str(":")) >> boolean;

            vardecl = Ign(Str("var")) >> Id(TokenType::Identifier);
            return_statement = Ign(Str("return")) >> ~boolean;
            break_statement = Wrap(Ign(Str("break")));
            paren_block = (Ign(Id(TokenType::Parens_Open)) >> equation >> Ign(Id(TokenType::Parens_Close)));
            lambda_def = Ign(Str("function")) >> ~(Ign(Id(TokenType::Parens_Open)) >> ~params >> Ign(Id(TokenType::Parens_Close))) >> block;

            return rule;
        }


        Eval_Engine build_eval_system(langkit::Lexer &lexer, langkit::Rule &parser) {
            using namespace langkit;
            Eval_Engine ss;
            dispatchkit::Bootstrap::bootstrap(ss);
            dispatchkit::bootstrap_vector<std::vector<dispatchkit::Boxed_Value> >(ss, "Vector");
            dispatchkit::bootstrap_map<std::map<std::string, dispatchkit::Boxed_Value> >(ss, "Map");


            ss.register_function(boost::shared_ptr<dispatchkit::Proxy_Function>(
                  new dispatchkit::Dynamic_Proxy_Function(boost::bind(&ChaiScript_System<Eval_Engine>::eval, boost::ref(*this), _1), 1)), "eval");

            evaluate_string("def print(x) { print_string(x.to_string()) } ");

            return ss;
        }

        langkit::TokenPtr parse(langkit::Rule &rule, std::vector<langkit::TokenPtr> &tokens, const char *filename) {
            using namespace langkit;

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

        dispatchkit::Boxed_Value evaluate_string(const std::string &input, const char *filename = "__EVAL__") {
            using namespace langkit;
            std::vector<TokenPtr> tokens = lexer.lex(input, filename);
            dispatchkit::Boxed_Value value;

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

        dispatchkit::Boxed_Value evaluate_file(const char *filename) {
            return evaluate_string(load_file(filename), filename);
        }
    };

    typedef ChaiScript_System<dispatchkit::Dispatch_Engine> ChaiScript_Engine;
}
#endif /* CHAISCRIPT_ENGINE_HPP_ */

