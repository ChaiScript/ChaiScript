// This file is distributed under the BSD License.
// See LICENSE.TXT for details.

#include <boost/bind.hpp>

#include <iostream>
#include <map>
#include <fstream>

#include "langkit_lexer.hpp"
#include "langkit_parser.hpp"

class TokenType { public: enum Type { File, Whitespace, Identifier, Number, Operator, Parens_Open, Parens_Close,
    Square_Open, Square_Close, Curly_Open, Curly_Close, Comma, Quoted_String, Single_Quoted_String, Carriage_Return, Semicolon,
    Function_Def, Scoped_Block, Statement, Equation, Return, Add, Comment}; };

void debug_print(TokenPtr token, std::string prepend) {
    std::cout << prepend << "Token: " << token->text << "(" << token->identifier << ") @ " << token->filename << ": ("  << token->start.line
        << ", " << token->start.column << ") to (" << token->end.line << ", " << token->end.column << ") " << std::endl;

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

void parse(std::vector<TokenPtr> &tokens, const char *filename) {

    /*
    Rule lhs;
    Rule rhs;
    Rule rule = lhs << rhs;
    lhs = Str("def", true);
    rhs = Id(TokenType::Identifier, true);

    //Rule rule(TokenType::Function_Def);
    //rule = Str("def") | Str("int");

    //Rule rule = Str("def", false) << Id(TokenType::Identifier);
    */


    //Example: "def add(x,y) { return x+y }"

    Rule params;
    Rule block(TokenType::Scoped_Block);
    Rule rule(TokenType::Function_Def);
    Rule statement(TokenType::Statement);
    Rule return_statement(TokenType::Return);
    Rule add(TokenType::Add);

    rule = Ign(Str("def")) << Id(TokenType::Identifier) << ~(Ign(Str("(")) << ~params << Ign(Str(")"))) << block;
    params = Id(TokenType::Identifier) << *(Ign(Str(",")) << Id(TokenType::Identifier));
    block = Ign(Str("{")) << ~return_statement << Ign(Str("}"));
    return_statement = Ign(Str("return")) << add;
    add = Id(TokenType::Identifier) << Ign(Str("+")) << Id(TokenType::Identifier);


    /*
    Rule rule = Str("x") << Id(TokenType::Semicolon);
    */

    /*
    Rule rule;
    Rule rule2;
    rule = Nop(rule2);
    rule2 = Str("Bob");
    */

    /*
    Rule rule(3);
    rule = Ign(Str("Bob")) << Str("Fred");
    */

    /*
    statement = equation;
    equation = Id(TokenType::Identifier) << Str("+") << Id(TokenType::Identifier);
    */

    /*
    Rule rule(TokenType::Function_Def);
    rule = +Str("Bob");
    */

    Token_Iterator iter = tokens.begin(), end = tokens.end();
    TokenPtr parent(new Token("Root", TokenType::File, filename));

    std::pair<Token_Iterator, bool> results = rule(iter, end, parent);

    /*
    while (results.second) {
        results = rule(results.first + 1, end, parent);
        //debug_print(parent, "");
    }
    */

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
    lexer.set_multiline_comment(Pattern("/\\*", TokenType::Comment), Pattern("\\*/", TokenType::Comment));
    lexer.set_singleline_comment(Pattern("//", TokenType::Comment));

    lexer << Pattern("[A-Za-z]+", TokenType::Identifier);
    lexer << Pattern("[0-9]+(\\.[0-9]+)?", TokenType::Number);
    lexer << Pattern("[!@#$%^&*\\-+=<>]+|/[!@#$%^&\\-+=<>]*", TokenType::Operator);
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
            debug_print(tokens);
            parse(tokens, "INPUT");

            std::cout << "Expression> ";
            std::getline(std::cin, input);
        }
    }
    else {
        std::vector<TokenPtr> tokens = lexer.lex(load_file(argv[1]), argv[1]);
        debug_print(tokens);
        parse(tokens, argv[1]);
    }
}
