// This file is distributed under the BSD License.
// See LICENSE.TXT for details.

#include <iostream>
#include <map>

#include "langkit_lexer.hpp"
//#include "lexer.hpp"
//#include "parser.hpp"
//#include "eval.hpp"

class TokenType { public: enum Type { Whitespace, Identifier, Number, Operator, Parens_Open, Parens_Close,
    Square_Open, Square_Close, Curly_Open, Curly_Close }; };

void debug_print(std::vector<Token> &tokens) {
    for (unsigned int i = 0; i < tokens.size(); ++i) {
        std::cout << "Token: " << tokens[i].text << "(" << tokens[i].identifier << ") @ " << tokens[i].start.column
            << " to " << tokens[i].end.column << std::endl;
    }
}

int main(int argc, char *argv[]) {
    std::string input;

    Lexer lexer;
    lexer.set_skip(Pattern("\\s+", TokenType::Whitespace));
    lexer << Pattern("[A-Za-z]+", TokenType::Identifier);
    lexer << Pattern("[0-9]+(\\.[0-9]+)?", TokenType::Number);
    lexer << Pattern("[!@#$%^&*\\-+=/:]+", TokenType::Operator);
    lexer << Pattern("\\(", TokenType::Parens_Open);
    lexer << Pattern("\\)", TokenType::Parens_Close);
    lexer << Pattern("\\[", TokenType::Square_Open);
    lexer << Pattern("\\]", TokenType::Square_Close);
    lexer << Pattern("\\{", TokenType::Curly_Open);
    lexer << Pattern("\\}", TokenType::Curly_Close);
    lexer << Pattern("[!@#$%^&*\\-+=/<>]+", TokenType::Operator);

    std::cout << "Expression> ";
    std::getline(std::cin, input);
    while (input != "quit") {
        std::vector<Token> tokens = lexer.lex(input);
        debug_print(tokens);
        std::cout << "Expression> ";
        std::getline(std::cin, input);
    }
}

/*
int main(int argc, char *argv[]) {
    std::string input;
    std::map<std::string, NodePtr> symbols;

    std::cout << "Expression> ";
    std::getline(std::cin, input);
    while (input != "quit") {
        if (input == "vars") {
            //debug_print(symbols);
        }
        else {
            try {
                NodePtr lex_nodes = lex(input);
                //clean_whitespace(lex_nodes);
                //NodePtr parse_nodes = parse(lex_nodes);
                //Result result = eval(parse_nodes, symbols);

                //debug_print(lex_nodes);
                //std::cout << std::endl;
                //debug_print(parse_nodes, "");
                //if (result.type == ResultType::NUMBER) {
                //    std::cout << result.value << std::endl;
                //}
            }
            catch (LexerError &le) {
                std::cerr << "Lexer error: " << le.reason << std::endl;
            }
            //catch (ParserError &pe) {
            //    std::cerr << "Parser error: " << pe.reason << std::endl;
            //}
            //catch (EvalError &ee) {
            //    std::cerr << "Eval error: " << ee.reason << std::endl;
            //}
        }
        std::cout << "Expression> ";
        std::getline(std::cin, input);
    }
}
*/
