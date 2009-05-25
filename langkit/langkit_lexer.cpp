// This file is distributed under the BSD License.
// See LICENSE.TXT for details.

#include <iostream>
#include "langkit_lexer.hpp"

Lexer Lexer::operator<<(const Pattern &p) {
    lex_patterns.push_back(p);
    return *this;
}

std::vector<Token> Lexer::lex(const std::string &input) {
    std::vector<Pattern>::iterator iter, end;
    //std::string::const_iterator str_end = input.end();
    std::vector<Token> retval;
    bool found;
    std::string::const_iterator input_iter = input.begin();

    while (input_iter != input.end()) {
        found = false;
        for (iter = lex_patterns.begin(), end = lex_patterns.end(); iter != end; ++iter) {
            boost::match_results<std::string::const_iterator> what;
            if (regex_search(input_iter, input.end(), what, iter->regex, boost::match_continuous)) {
                Token t(what[0], iter->identifier);
                t.start.column = input_iter - input.begin();
                t.end.column = t.start.column + t.text.size();
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
                    found = true;
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
    return retval;
}

void Lexer::set_skip(const Pattern &p) {
    skip_patterns.push_back(p);
}
