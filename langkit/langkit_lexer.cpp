// This file is distributed under the BSD License.
// See LICENSE.TXT for details.

#include <iostream>
#include "langkit_lexer.hpp"

Lexer Lexer::operator<<(const Pattern &p) {
    lex_patterns.push_back(p);
    return *this;
}

std::vector<TokenPtr> Lexer::lex(const std::string &input, const char *filename) {
    std::vector<Pattern>::iterator iter, end, iter2, end2;
    std::vector<TokenPtr> retval;
    bool found;
    std::string::const_iterator input_iter = input.begin(), input_end = input.end();

    int current_col = 0;
    int current_line = 0;
    boost::match_results<std::string::const_iterator> what;

    while (input_iter != input_end) {
        found = false;

        if (regex_search(input_iter, input_end, what, singleline_comment_pattern.regex, boost::match_continuous)) {
            std::string comment_start(what[0]);
            input_iter += comment_start.size();

            bool found_eol = false;

            while ((!found_eol) && (input_iter != input_end)) {
                boost::match_results<std::string::const_iterator> eol_delim;
                if (regex_search(input_iter, input_end, eol_delim, line_sep_pattern.regex, boost::match_continuous)) {
                    std::string comment_end(eol_delim[0]);
                    input_iter += comment_end.size();
                    ++current_line;
                    current_col = 0;
                    found_eol = true;
                    break;
                }
                if ((!found_eol) && (input_iter != input_end)) {
                    ++input_iter;
                }
            }
        }
        else if (regex_search(input_iter, input_end, what, multiline_comment_start_pattern.regex, boost::match_continuous)) {
            std::string comment_start(what[0]);
            input_iter += comment_start.size();

            bool found_eoc = false;

            while ((!found_eoc) && (input_iter != input_end)) {
                boost::match_results<std::string::const_iterator> eol_delim;
                if (regex_search(input_iter, input_end, eol_delim, line_sep_pattern.regex, boost::match_continuous)) {
                    std::string comment_end(eol_delim[0]);
                    input_iter += comment_end.size();
                    ++current_line;
                    current_col = 0;
                    break;
                }
                boost::match_results<std::string::const_iterator> eoc_delim;
                if (regex_search(input_iter, input_end, eoc_delim, multiline_comment_end_pattern.regex, boost::match_continuous)) {
                    std::string comment_end(eoc_delim[0]);
                    input_iter += comment_end.size();
                    current_col += comment_end.size();
                    found_eoc = true;
                    break;
                }
                if ((!found_eoc) && (input_iter != input_end)) {
                    ++input_iter;
                }
            }

            if (!found_eoc) {
                std::cout << "Incomplete comment block!  Add exceptions!" << std::endl;
                return retval;
            }
        }
        else if (regex_search(input_iter, input_end, what, skip_pattern.regex, boost::match_continuous)) {
            std::string whitespace(what[0]);
            input_iter += whitespace.size();
            current_col += whitespace.size();
            found = true;
        }
        else if (regex_search(input_iter, input_end, what, line_sep_pattern.regex, boost::match_continuous)) {
            const std::string cr(what[0]);

            boost::match_results<std::string::const_iterator> if_delim;
            if (regex_search(cr.begin(), cr.end(), if_delim, command_sep_pattern.regex, boost::match_continuous)) {
                TokenPtr t(new Token(if_delim[0], command_sep_pattern.identifier, filename));
                t->start.column = current_col;
                t->start.line = current_line;
                current_col += t->text.size();
                t->end.column = current_col;
                t->end.line = current_line;
                retval.push_back(t);
            }

            input_iter += cr.size();
            ++current_line;
            current_col = 0;
            found = true;
        }
        else if (regex_search(input_iter, input_end, what, command_sep_pattern.regex, boost::match_continuous)) {
            TokenPtr t(new Token(what[0], command_sep_pattern.identifier, filename));
            t->start.column = current_col;
            t->start.line = current_line;
            current_col += t->text.size();
            t->end.column = current_col;
            t->end.line = current_line;
            retval.push_back(t);
            input_iter += t->text.size();
            found = true;
        }
        else {
            for (iter = lex_patterns.begin(), end = lex_patterns.end(); iter != end; ++iter) {
                if (regex_search(input_iter, input_end, what, iter->regex, boost::match_continuous)) {
                    TokenPtr t(new Token(what[0], iter->identifier, filename));
                    t->start.column = current_col;
                    t->start.line = current_line;
                    current_col += t->text.size();
                    t->end.column = current_col;
                    t->end.line = current_line;
                    retval.push_back(t);
                    input_iter += t->text.size();
                    found = true;
                    break;
                }
            }

            if (!found) {
                const std::string err(input_iter, input_end);
                std::cout << "Unknown string at: " << err << std::endl;
                return retval;
            }
        }
    }
    return retval;
}
