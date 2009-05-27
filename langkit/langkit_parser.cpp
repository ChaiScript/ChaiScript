// This file is distributed under the BSD License.
// See LICENSE.TXT for details.

#include <boost/bind.hpp>

#include <iostream>
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

std::pair<Token_Iterator, bool> Or_Rule(Token_Iterator iter, Token_Iterator end, TokenPtr parent, Rule lhs, Rule rhs) {
    Token_Iterator new_iter;
    unsigned int prev_size = parent->children.size();

    if (*iter != *end) {
        std::pair<Token_Iterator, bool> result = lhs(iter, end, parent);

        if (result.second) {
            return std::pair<Token_Iterator, bool>(result.first, true);
        }
        else {
            result = rhs(iter, end, parent);
            if (result.second) {
                return std::pair<Token_Iterator, bool>(result.first, true);
            }
        }
    }

    if (parent->children.size() != prev_size) {
        //Clear out the partial matches
        parent->children.erase(parent->children.begin() + prev_size, parent->children.end());
    }

    return std::pair<Token_Iterator, bool>(iter, false);
}

std::pair<Token_Iterator, bool> And_Rule(Token_Iterator iter, Token_Iterator end, TokenPtr parent, Rule lhs, Rule rhs) {
    Token_Iterator lhs_iter, rhs_iter;
    unsigned int prev_size = parent->children.size();

    if (*iter != *end) {
        std::pair<Token_Iterator, bool> result = lhs(iter, end, parent);

        if (result.second) {
            result = rhs(result.first, end, parent);
            if (result.second) {
                return std::pair<Token_Iterator, bool>(result.first, true);
            }
        }
    }

    if (parent->children.size() != prev_size) {
        //Clear out the partial matches
        parent->children.erase(parent->children.begin() + prev_size, parent->children.end());
    }

    return std::pair<Token_Iterator, bool>(iter, false);
}

std::pair<Token_Iterator, bool> Rule::operator()(Token_Iterator iter, Token_Iterator end, TokenPtr parent) {
    return impl->rule(iter, end, parent);
}

Rule Str(const std::string &text, bool keep) {
    return Rule(boost::bind(String_Rule, _1, _2, _3, text, keep));
}
Rule Id(int id, bool keep) {
    return Rule(boost::bind(Type_Rule, _1, _2, _3, id, keep));
}

Rule Str(const std::string &text) {
    return Rule(boost::bind(String_Rule, _1, _2, _3, text, false));
}

Rule Id(int id) {
    return Rule(boost::bind(Type_Rule, _1, _2, _3, id, false));
}

