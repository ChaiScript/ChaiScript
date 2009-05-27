// This file is distributed under the BSD License.
// See LICENSE.TXT for details.

#include <boost/bind.hpp>

#include <iostream>
#include <map>
#include <fstream>

#include "langkit_lexer.hpp"
#include "langkit_parser.hpp"

class TokenType { public: enum Type { Whitespace, Identifier, Number, Operator, Parens_Open, Parens_Close,
    Square_Open, Square_Close, Curly_Open, Curly_Close, Comma, Quoted_String, Single_Quoted_String, Carriage_Return, Semicolon,
    Function_Def}; };

void debug_print(TokenPtr token, std::string prepend) {
    std::cout << prepend << "Token: " << token->text << "(" << token->identifier << ") @ " << token->filename << ": ("  << token->start.column
        << ", " << token->start.line << ") to (" << token->end.column << ", " << token->end.line << ") " << std::endl;

    for (unsigned int i = 0; i < token->children.size(); ++i) {
        debug_print(token->children[i], prepend + "  ");
    }
}

void debug_print(std::vector<TokenPtr> &tokens) {
    for (unsigned int i = 0; i < tokens.size(); ++i) {
        debug_print(tokens[i], "");
    }
}

std::string load_file(const char *filename) {
    std::ifstream infile (filename, std::ios::in | std::ios::ate);

    if (!infile.is_open()) {
        std::cerr << "Can not open " << filename << std::endl;
        exit(0);
    }

    std::streampos size = infile.tellg();
    infile.seekg(0, std::ios::beg);

    std::vector<char> v(size);
    infile.read(&v[0], size);

    std::string ret_val (v.empty() ? std::string() : std::string (v.begin(), v.end()).c_str());

    return ret_val;
}

void parse(std::vector<TokenPtr> &tokens) {

    /*
    Rule lhs;
    Rule rhs;
    Rule rule = lhs << rhs;
    lhs = Str("def", true);
    rhs = Id(TokenType::Identifier, true);
    */

    //Rule rule(TokenType::Function_Def);
    //rule = Str("def") | Str("int");

    //Rule rule = Str("def", false) << Id(TokenType::Identifier);

    Rule params;
    Rule rule(TokenType::Function_Def);
    rule = Ign(Str("def")) << Id(TokenType::Identifier) << Ign(Str("(")) << params << Ign(Str(")"));
    params = Id(TokenType::Identifier) << Ign(Str(",")) << Id(TokenType::Identifier);

    Token_Iterator iter = tokens.begin(), end = tokens.end();
    TokenPtr parent(new Token("Root", 0, "test"));

    std::pair<Token_Iterator, bool> results = rule(iter, end, parent);

    if (results.second) {
        std::cout << "Parse successful: " << std::endl;
        debug_print(parent, "");
    }
    else {
        std::cout << "Parse failed: " << std::endl;
        debug_print(parent, "");
    }

}


int main(int argc, char *argv[]) {
    std::string input;

    Lexer lexer;
    lexer.set_skip(Pattern("[ \\t]+", TokenType::Whitespace));
    lexer.set_line_sep(Pattern("\\n|\\r\\n", TokenType::Carriage_Return));
    lexer.set_command_sep(Pattern(";|\\r\\n|\\n", TokenType::Semicolon));

    lexer << Pattern("[A-Za-z]+", TokenType::Identifier);
    lexer << Pattern("[0-9]+(\\.[0-9]+)?", TokenType::Number);
    lexer << Pattern("[!@#$%^&*\\-+=/<>]+", TokenType::Operator);
    lexer << Pattern("\\(", TokenType::Parens_Open);
    lexer << Pattern("\\)", TokenType::Parens_Close);
    lexer << Pattern("\\[", TokenType::Square_Open);
    lexer << Pattern("\\]", TokenType::Square_Close);
    lexer << Pattern("\\{", TokenType::Curly_Open);
    lexer << Pattern("\\}", TokenType::Curly_Close);
    lexer << Pattern(",", TokenType::Comma);
    lexer << Pattern("\"(?:[^\"\\\\]|\\\\.)*\"", TokenType::Quoted_String);
    lexer << Pattern("'(?:[^'\\\\]|\\\\.)*'", TokenType::Single_Quoted_String);

    if (argc < 2) {
        std::cout << "Expression> ";
        std::getline(std::cin, input);
        while (input != "quit") {
            std::vector<TokenPtr> tokens = lexer.lex(input, "INPUT");
            //debug_print(tokens);
            parse(tokens);

            std::cout << "Expression> ";
            std::getline(std::cin, input);
        }
    }
    else {
        std::vector<TokenPtr> tokens = lexer.lex(load_file(argv[1]), argv[1]);
        debug_print(tokens);
    }
}
