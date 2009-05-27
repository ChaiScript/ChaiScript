// This file is distributed under the BSD License.
// See LICENSE.TXT for details.

#ifndef LANGKIT_PARSER_HPP_
#define LANGKIT_PARSER_HPP_

#include <boost/function.hpp>

#include "langkit_lexer.hpp"

typedef std::vector<TokenPtr>::iterator Token_Iterator;
typedef std::tr1::shared_ptr<struct Rule> RulePtr;
typedef boost::function<std::pair<Token_Iterator, bool>(Token_Iterator iter, Token_Iterator end, TokenPtr parent)> RuleFun;

struct Rule {
    int identifier;
    RuleFun rule;
    std::pair<Token_Iterator, bool> operator()(Token_Iterator iter, Token_Iterator end, TokenPtr parent);

    Rule() : identifier(-1) {}
    Rule(int id) : identifier(id) {}
    Rule(RuleFun fun) : rule(fun) {}
};

std::pair<Token_Iterator, bool> String_Rule
    (Token_Iterator iter, Token_Iterator end, TokenPtr parent, const std::string &val, bool keep);

std::pair<Token_Iterator, bool> Type_Rule
    (Token_Iterator iter, Token_Iterator end, TokenPtr parent, const int val, bool keep);

std::pair<Token_Iterator, bool> Or_Rule
    (Token_Iterator iter, Token_Iterator end, TokenPtr parent, const RulePtr lhs, const RulePtr rhs, bool keep);

std::pair<Token_Iterator, bool> And_Rule
    (Token_Iterator iter, Token_Iterator end, TokenPtr parent, const RulePtr lhs, const RulePtr rhs, bool keep);


#endif /* LANGKIT_PARSER_HPP_ */
