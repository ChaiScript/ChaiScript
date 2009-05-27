// This file is distributed under the BSD License.
// See LICENSE.TXT for details.

#include <utility>

#include "langkit_lexer.hpp"
#include "langkit_parser.hpp"

std::pair<Token_Iterator, bool> String_Rule(Token_Iterator iter, Token_Iterator end, TokenPtr parent, const std::string &val, bool keep) {
    if (*iter != *end) {
        if ((*iter)->text == val) {
            if (keep) {
                parent->children.push_back(*iter);
            }
            return std::pair<Token_Iterator, bool>(++iter, true);
        }
    }

    return std::pair<Token_Iterator, bool>(iter, false);
}

std::pair<Token_Iterator, bool> Type_Rule(Token_Iterator iter, Token_Iterator end, TokenPtr parent, const int val, bool keep) {
    if (*iter != *end) {
        if ((*iter)->identifier == val) {
            if (keep) {
                parent->children.push_back(*iter);
            }
            return std::pair<Token_Iterator, bool>(++iter, true);
        }
    }

    return std::pair<Token_Iterator, bool>(iter, false);
}

std::pair<Token_Iterator, bool> Or_Rule(Token_Iterator iter, Token_Iterator end, TokenPtr parent, const Rule &lhs, const Rule &rhs, bool keep, int new_id) {
    Token_Iterator new_iter;

    if (*iter != *end) {
        new_iter = lhs.rule(iter, end, parent).first;

        if (new_iter != iter) {
            return std::pair<Token_Iterator, bool>(new_iter, true);
        }
        else {
            new_iter = rhs.rule(iter, end, parent).first;
            if (new_iter != iter) {
                return std::pair<Token_Iterator, bool>(new_iter, true);
            }
        }
    }
    return std::pair<Token_Iterator, bool>(iter, false);
}

std::pair<Token_Iterator, bool> And_Rule(Token_Iterator iter, Token_Iterator end, TokenPtr parent, const Rule &lhs, const Rule &rhs, bool keep, int new_id) {
    Token_Iterator lhs_iter, rhs_iter;

    if (*iter != *end) {
        lhs_iter = lhs.rule(iter, end, parent).first;

        if (lhs_iter != iter) {
            rhs_iter = rhs.rule(iter, end, parent).first;
            if (rhs_iter != iter) {
                return std::pair<Token_Iterator, bool>(rhs_iter, true);
            }
        }
    }

    return std::pair<Token_Iterator, bool>(iter, false);
}

std::pair<Token_Iterator, bool> Rule::operator()(Token_Iterator iter, Token_Iterator end, TokenPtr parent) {
    return this->rule(iter, end, parent);
}
