// This file is distributed under the BSD License.
// See LICENSE.TXT for details.

#ifndef LANGKIT_LEXER_HPP_
#define LANGKIT_LEXER_HPP_

#include <string>
#include <boost/regex.hpp>

struct File_Position {
    int row;
    int column;
    char *filename;

    File_Position(int file_row, int file_column, char *fname)
        : row(file_row), column(file_column), filename(fname) { }

    File_Position() : row(0), column(0), filename(NULL) { }
};

struct Pattern {
    boost::regex regex;
    int identifier;

    Pattern(const std::string &regexp, int id) : regex(regexp), identifier(id) { }
};

struct Token {
    std::string text;
    int identifier;
    File_Position start, end;

    Token(const std::string &token_text, int id) : text(token_text), identifier(id) { }
};

struct Lexer {
    std::vector<Pattern> lex_patterns;
    std::vector<Pattern> skip_patterns;

    Lexer operator<<(const Pattern &p);
    std::vector<Token> lex(const std::string &input);

    void set_skip(const Pattern &p);
};


#endif /* LANGKIT_LEXER_HPP_ */
