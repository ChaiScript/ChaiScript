// This file is distributed under the BSD License.
// See LICENSE.TXT for details.

#include <iostream>
#include "langkit_lexer.hpp"

Lexer Lexer::operator<<(const Pattern &p) {
    lex_patterns.push_back(p);
    return *this;
}

std::vector<Token> Lexer::lex(const std::string &input, const char *filename) {
    std::vector<Pattern>::iterator iter, end;
    std::vector<Token> retval;
    bool found;
    std::string::const_iterator input_iter = input.begin();

    int current_col = 0;
    int current_line = 0;

    while (input_iter != input.end()) {
        found = false;
        for (iter = lex_patterns.begin(), end = lex_patterns.end(); iter != end; ++iter) {
            boost::match_results<std::string::const_iterator> what;
            if (regex_search(input_iter, input.end(), what, iter->regex, boost::match_continuous)) {
                Token t(what[0], iter->identifier, filename);
                t.start.column = current_col;
                t.start.line = current_line;
                current_col += t.text.size();
                t.end.column = current_col;
                t.end.line = current_line;
                retval.push_back(t);
                input_iter += t.text.size();
                found = true;
                break;
            }
        }
        if (!found) {
            for (iter = skip_patterns.begin(), end = skip_patterns.end(); iter != end; ++iter) {
                boost::match_results<std::string::const_iterator> what;
                if (regex_search(input_iter, input.end(), what, iter->regex, boost::match_continuous)) {
                    std::string whitespace(what[0]);
                    input_iter += whitespace.size();
                    current_col += whitespace.size();
                    found = true;
                    break;
                }
            }

            if (!found) {
                for (iter = line_sep_patterns.begin(), end = line_sep_patterns.end(); iter != end; ++iter) {
                    boost::match_results<std::string::const_iterator> what;
                    if (regex_search(input_iter, input.end(), what, iter->regex, boost::match_continuous)) {
                        std::string cr(what[0]);
                        input_iter += cr.size();
                        found = true;
                        ++current_line;
                        current_col = 0;
                        break;
                    }
                }

                if (!found) {
                    const std::string err(input_iter, input.end());
                    std::cout << "Unknown string at: " << err << std::endl;
                    return retval;
                }
            }
        }
    }
    return retval;
}

void Lexer::set_skip(const Pattern &p) {
    skip_patterns.push_back(p);
}

void Lexer::set_line_sep(const Pattern &p) {
    line_sep_patterns.push_back(p);
}

void Lexer::set_command_sep(const Pattern &p) {
    command_sep_patterns.push_back(p);
}
