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
    int identifier;

    Pattern() { }
    Pattern(const std::string &regexp, int id) : regex(regexp), identifier(id) { }
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
    Pattern skip_pattern;
    Pattern command_sep_pattern;
    Pattern line_sep_pattern;
    Pattern multiline_comment_start_pattern;
    Pattern multiline_comment_end_pattern;
    Pattern singleline_comment_pattern;

    Lexer operator<<(const Pattern &p);
    std::vector<TokenPtr> lex(const std::string &input, const char *fname);

    void set_skip(const Pattern &p) {
        skip_pattern = p;
    }
    void set_line_sep(const Pattern &p) {
        line_sep_pattern = p;
    }
    void set_command_sep(const Pattern &p) {
        command_sep_pattern = p;
    }
    void set_multiline_comment(const Pattern &start, const Pattern &end) {
        multiline_comment_start_pattern = start;
        multiline_comment_end_pattern = end;
    }
    void set_singleline_comment(const Pattern &p) {
        singleline_comment_pattern = p;
    }
};


#endif /* LANGKIT_LEXER_HPP_ */
