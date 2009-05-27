// This file is distributed under the BSD License.
// See LICENSE.TXT for details.

#ifndef LANGKIT_PARSER_HPP_
#define LANGKIT_PARSER_HPP_

#include <boost/function.hpp>

#include "langkit_lexer.hpp"

typedef std::vector<TokenPtr>::iterator Token_Iterator;
typedef std::tr1::shared_ptr<struct RuleImpl> RuleImplPtr;
typedef boost::function<std::pair<Token_Iterator, bool>(Token_Iterator iter, Token_Iterator end, TokenPtr parent)> RuleFun;

struct RuleImpl {
    int identifier;
    RuleFun rule;

    RuleImpl() : identifier(-1) {}
    RuleImpl(int id) : identifier(id) {}
    RuleImpl(RuleFun fun) : rule(fun) {}
};

std::pair<Token_Iterator, bool> String_Rule
    (Token_Iterator iter, Token_Iterator end, TokenPtr parent, const std::string &val, bool keep);

std::pair<Token_Iterator, bool> Type_Rule
    (Token_Iterator iter, Token_Iterator end, TokenPtr parent, const int val, bool keep);

std::pair<Token_Iterator, bool> Or_Rule
    (Token_Iterator iter, Token_Iterator end, TokenPtr parent, struct Rule lhs, struct Rule rhs);

std::pair<Token_Iterator, bool> And_Rule
    (Token_Iterator iter, Token_Iterator end, TokenPtr parent, struct Rule lhs, struct Rule rhs);

struct Rule {
    Rule() : impl(new RuleImpl(-1)) {}
    Rule(int id) : impl(new RuleImpl(id)) {}
    Rule(RuleFun fun) : impl(new RuleImpl(fun)) {}

    std::pair<Token_Iterator, bool> operator()(Token_Iterator iter, Token_Iterator end, TokenPtr parent);

    Rule &operator=(const Rule &rule) {
        impl->identifier = rule.get_impl()->identifier;
        impl->rule = rule.get_impl()->rule;

        return *this;
    }

    Rule operator|(const Rule &rhs) {
        return Rule(boost::bind(Or_Rule, _1, _2, _3, *this, rhs));
    }

    Rule operator&(const Rule &rhs) {
        return Rule(boost::bind(And_Rule, _1, _2, _3, *this, rhs));
    }

    void set_rule(RuleFun fun) {
        impl->rule = fun;
    }

    const RuleImplPtr get_impl() const { return impl; }

private:
    RuleImplPtr impl;
};


Rule Str(const std::string &text, bool keep);
Rule Id(int id, bool keep);


#endif /* LANGKIT_PARSER_HPP_ */
