// This file is distributed under the BSD License.
// See LICENSE.TXT for details.

#ifndef LANGKIT_LEXER_HPP_
#define LANGKIT_LEXER_HPP_

#include <boost/regex.hpp>
#include <tr1/memory>
#include <string>

struct File_Position {
    int line;
    int column;

    File_Position(int file_line, int file_column)
        : line(file_line), column(file_column) { }

    File_Position() : line(0), column(0) { }
};

struct Pattern {
    boost::regex regex;
    std::string regex_string;
    int identifier;

    Pattern(const std::string &regexp, int id) : regex(regexp), regex_string(regexp), identifier(id) { }
};

typedef std::tr1::shared_ptr<struct Token> TokenPtr;

struct Token {
    std::string text;
    int identifier;
    const char *filename;
    File_Position start, end;

    std::vector<TokenPtr> children;

    Token(const std::string &token_text, int id, const char *fname) : text(token_text), identifier(id), filename(fname) { }
};

struct Lexer {
    std::vector<Pattern> lex_patterns;
    std::vector<Pattern> skip_patterns;
    std::vector<Pattern> command_sep_patterns;
    std::vector<Pattern> line_sep_patterns;

    Lexer operator<<(const Pattern &p);
    std::vector<TokenPtr> lex(const std::string &input, const char *fname);

    void set_skip(const Pattern &p);
    void set_line_sep(const Pattern &p);
    void set_command_sep(const Pattern &p);
};


#endif /* LANGKIT_LEXER_HPP_ */
