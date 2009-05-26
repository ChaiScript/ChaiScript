// This file is distributed under the BSD License.
// See LICENSE.TXT for details.

#include "langkit_lexer.hpp"
#include "langkit_parser.hpp"

Token_Iterator String_Rule(Token_Iterator iter, Token_Iterator end, const std::string &val) {
    if (iter != end) {
        if (iter->text == val) {
            return ++iter;
        }
    }

    return iter;
}

Token_Iterator Type_Rule(Token_Iterator iter, Token_Iterator end, const int val) {
    if (iter != end) {
        if (iter->identifier == val) {
            return ++iter;
        }
    }

    return iter;
}

Token_Iterator Or_Rule(Token_Iterator iter, Token_Iterator end, const Rule &lhs, const Rule &rhs) {
    Token_Iterator new_iter;

    if (iter != end) {
        new_iter = lhs.rule(iter, end);

        if (new_iter != iter) {
            return new_iter;
        }
        else {
            new_iter = rhs.rule(iter, end);
            if (new_iter != iter) {
                return new_iter;
            }
        }
    }
    return iter;
}

Token_Iterator And_Rule(Token_Iterator iter, Token_Iterator end, const Rule &lhs, const Rule &rhs) {
    Token_Iterator lhs_iter, rhs_iter;

    if (iter != end) {
        lhs_iter = lhs.rule(iter, end);

        if (lhs_iter != iter) {
            rhs_iter = rhs.rule(iter, end);
            if (rhs_iter != iter) {
                return rhs_iter;
            }
        }
    }

    return iter;
}
