#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>

#include <iostream>
#include <map>
#include <fstream>

#include "boxedcpp.hpp"
#include "bootstrap.hpp"
#include "bootstrap_stl.hpp"

#include "langkit_lexer.hpp"
#include "langkit_parser.hpp"

class TokenType { public: enum Type { File, Whitespace, Identifier, Number, Operator, Parens_Open, Parens_Close,
    Square_Open, Square_Close, Curly_Open, Curly_Close, Comma, Quoted_String, Single_Quoted_String, Carriage_Return, Semicolon,
    Function_Def, Scoped_Block, Statement, Equation, Return, Expression, Term, Factor, Negate, Comment,
    Value, Fun_Call }; };

char *tokentype_to_string(int tokentype) {
    char *token_types[] = {"File", "Whitespace", "Identifier", "Number", "Operator", "Parens_Open", "Parens_Close",
        "Square_Open", "Square_Close", "Curly_Open", "Curly_Close", "Comma", "Quoted_String", "Single_Quoted_String", "Carriage_Return", "Semicolon",
        "Function_Def", "Scoped_Block", "Statement", "Equation", "Return", "Expression", "Term", "Factor", "Negate", "Comment",
        "Value", "Fun_Call" };

    return token_types[tokentype];
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

//A function that prints any string passed to it
void print_string(const std::string &s)
{
  std::cout << s << std::endl;
}

void print_double(const double &d)
{
  std::cout << d << std::endl;
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

Boxed_Value eval_token(TokenPtr node, BoxedCPP_System ss) {
    Boxed_Value retval;
    unsigned int i;

    switch (node->identifier) {
        case (TokenType::Value) :
        case (TokenType::File) :
            for (i = 0; i < node->children.size(); ++i) {
                eval_token(node->children[i], ss);
            }
        break;
        case (TokenType::Identifier) :
        break;
        case (TokenType::Number) :
            retval = Boxed_Value(double(atof(node->text.c_str())));
        break;
        case (TokenType::Quoted_String) :
            retval = Boxed_Value(node->text);
        break;
        case (TokenType::Single_Quoted_String) :
            retval = Boxed_Value(node->text);
        break;
        case (TokenType::Factor) :
        case (TokenType::Expression) :
        case (TokenType::Term) : {
            retval = eval_token(node->children[0], ss);
            if (node->children.size() > 1) {
                for (i = 1; i < node->children.size(); i += 2) {
                    Param_List_Builder plb;
                    plb << retval;
                    plb << eval_token(node->children[i + 1], ss);

                    retval = dispatch(ss.get_function(node->children[i]->text), plb);
                }
            }
        }
        break;
        case (TokenType::Fun_Call) : {
            Param_List_Builder plb;
            for (i = 1; i < node->children.size(); ++i) {
                plb << eval_token(node->children[i], ss);
            }
            retval = dispatch(ss.get_function(node->children[0]->text), plb);
        }
        break;
        case (TokenType::Scoped_Block) :
        case (TokenType::Statement) :
        case (TokenType::Negate) :
        case (TokenType::Return) :
        case (TokenType::Carriage_Return) :
        case (TokenType::Semicolon) :
        case (TokenType::Equation) :
        case (TokenType::Function_Def) :
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

void eval(TokenPtr parent) {
    BoxedCPP_System ss;
    bootstrap(ss);
    bootstrap_vector<std::vector<int> >(ss);
    //dump_system(ss);

    //Register a new function, this one with typing for us, so we don't have to ubox anything
    //right here
    ss.register_function(boost::function<void (const std::string &)>(&print_string), "print");
    ss.register_function(boost::function<void (const double &)>(&print_double), "print");

    /*
    Param_List_Builder plb;
    plb << Boxed_Value(double(2.5));
    Boxed_Value val = dispatch(ss.get_function("to_string"), plb);

    Param_List_Builder plb2;
    plb2 << val;
    dispatch(ss.get_function("print"), plb2);
    */

    Boxed_Value value = eval_token(parent, ss);
}

void parse(std::vector<TokenPtr> &tokens, const char *filename) {


    Rule params;
    Rule block(TokenType::Scoped_Block);
    Rule statement(TokenType::Statement);
    Rule return_statement(TokenType::Return);
    Rule expression(TokenType::Expression);
    Rule term(TokenType::Term);
    Rule factor(TokenType::Factor);
    Rule negate(TokenType::Negate);
    Rule funcall(TokenType::Fun_Call);
    Rule value;

    Rule rule = *(expression >> *Ign(Id(TokenType::Semicolon)));
    expression = term >> *((Str("+") >> term) | (Str("-") >> term));
    term = factor >> *((Str("*") >> factor) | (Str("/") >> factor));
    factor = value | negate | (Ign(Str("+")) >> value);
    funcall = Id(TokenType::Identifier) >> Ign(Id(TokenType::Parens_Open)) >> ~(expression >> *(Ign(Str("," )) >> expression)) >> Ign(Id(TokenType::Parens_Close));
    negate = Ign(Str("-")) >> factor;

    value = funcall | Id(TokenType::Identifier) | Id(TokenType::Number) | Id(TokenType::Quoted_String) | Id(TokenType::Single_Quoted_String);

    Token_Iterator iter = tokens.begin(), end = tokens.end();
    TokenPtr parent(new Token("Root", TokenType::File, filename));

    std::pair<Token_Iterator, bool> results = rule(iter, end, parent);

    if (results.second) {
        //std::cout << "Parse successful: " << std::endl;
        //debug_print(parent, "");
        eval(parent);
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

    lexer << Pattern("[A-Za-z_]+", TokenType::Identifier);
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
        std::cout << "eval> ";
        std::getline(std::cin, input);
        while (input != "quit") {
            std::vector<TokenPtr> tokens = lexer.lex(input, "INPUT");

            for (unsigned int i = 0; i < tokens.size(); ++i) {
                if ((tokens[i]->identifier == TokenType::Quoted_String) || (tokens[i]->identifier == TokenType::Single_Quoted_String)) {
                    tokens[i]->text = tokens[i]->text.substr(1, tokens[i]->text.size()-2);
                }
            }

            //debug_print(tokens);
            parse(tokens, "INPUT");

            std::cout << "eval> ";
            std::getline(std::cin, input);
        }
    }
    else {
        std::vector<TokenPtr> tokens = lexer.lex(load_file(argv[1]), argv[1]);
        debug_print(tokens);
        parse(tokens, argv[1]);
    }
}

