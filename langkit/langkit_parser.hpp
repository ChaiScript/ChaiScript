// This file is distributed under the BSD License.
// See LICENSE.TXT for details.

#ifndef LANGKIT_PARSER_HPP_
#define LANGKIT_PARSER_HPP_

#include <boost/function.hpp>

#include "langkit_lexer.hpp"

typedef std::vector<TokenPtr>::iterator Token_Iterator;

struct Rule {
    int identifier;
    boost::function<std::pair<Token_Iterator, TokenPtr>(Token_Iterator iter, Token_Iterator end)> rule;
    std::pair<Token_Iterator, TokenPtr> operator()(Token_Iterator iter, Token_Iterator end);

    Rule() : identifier(-1) {}
    Rule(int id) : identifier(id) {}
};

std::pair<Token_Iterator, TokenPtr> String_Rule(Token_Iterator iter, Token_Iterator end, const std::string &val);
std::pair<Token_Iterator, TokenPtr> Type_Rule(Token_Iterator iter, Token_Iterator end, const int val);
std::pair<Token_Iterator, TokenPtr> Or_Rule(Token_Iterator iter, Token_Iterator end, const Rule &lhs, const Rule &rhs);
std::pair<Token_Iterator, TokenPtr> And_Rule(Token_Iterator iter, Token_Iterator end, const Rule &lhs, const Rule &rhs);


#endif /* LANGKIT_PARSER_HPP_ */
