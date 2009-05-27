// This file is distributed under the BSD License.
// See LICENSE.TXT for details.

#include <utility>

#include "langkit_lexer.hpp"
#include "langkit_parser.hpp"

std::pair<Token_Iterator, TokenPtr> String_Rule(Token_Iterator iter, Token_Iterator end, const std::string &val) {
    if (iter != end) {
        if ((*iter)->text == val) {
            return std::pair<Token_Iterator, TokenPtr>(++iter, *iter);
        }
    }

    return std::pair<Token_Iterator, TokenPtr>(iter, *iter);
}

std::pair<Token_Iterator, TokenPtr> Type_Rule(Token_Iterator iter, Token_Iterator end, const int val) {
    if (iter != end) {
        if ((*iter)->identifier == val) {
            return std::pair<Token_Iterator, TokenPtr>(++iter, *iter);
        }
    }

    return std::pair<Token_Iterator, TokenPtr>(iter, *iter);
}

std::pair<Token_Iterator, TokenPtr> Or_Rule(Token_Iterator iter, Token_Iterator end, const Rule &lhs, const Rule &rhs) {
    Token_Iterator new_iter;

    if (iter != end) {
        new_iter = lhs.rule(iter, end).first;

        if (new_iter != iter) {
            return std::pair<Token_Iterator, TokenPtr>(new_iter, *iter);
        }
        else {
            new_iter = rhs.rule(iter, end).first;
            if (new_iter != iter) {
                return std::pair<Token_Iterator, TokenPtr>(new_iter, *iter);
            }
        }
    }
    return std::pair<Token_Iterator, TokenPtr>(iter, *iter);
}

/*
std::pair<Token_Iterator, Token> And_Rule(Token_Iterator iter, Token_Iterator end, const Rule &lhs, const Rule &rhs) {
    Token_Iterator lhs_iter, rhs_iter;

    if (iter != end) {
        lhs_iter = lhs.rule(iter, end);

        if (lhs_iter != iter) {
            rhs_iter = rhs.rule(iter, end);
            if (rhs_iter != iter) {
                return std::pair<Token_Iterator, Token>(rhs_iter, *iter);
            }
        }
    }

    return std::pair<Token_Iterator, Token>(iter, *iter);
}
*/

std::pair<Token_Iterator, TokenPtr> Rule::operator()(Token_Iterator iter, Token_Iterator end) {
    return this->rule(iter, end);
}
