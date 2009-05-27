// This file is distributed under the BSD License.
// See LICENSE.TXT for details.

#ifndef LANGKIT_PARSER_HPP_
#define LANGKIT_PARSER_HPP_

#include <boost/function.hpp>

#include "langkit_lexer.hpp"

typedef std::vector<TokenPtr>::iterator Token_Iterator;
typedef boost::function<std::pair<Token_Iterator, bool>(Token_Iterator, Token_Iterator, TokenPtr, bool, int)> RuleFun;
typedef std::tr1::shared_ptr<struct RuleImpl> RuleImplPtr;

struct RuleImpl {
    RuleFun rule;
    bool keep;
    int new_id;

    RuleImpl() : keep(true), new_id(-1) { }

    std::pair<Token_Iterator, bool> operator()(Token_Iterator iter, Token_Iterator end, TokenPtr parent) {
        return rule(iter, end, parent, keep, new_id);
    }
};

std::pair<Token_Iterator, bool> String_Rule
    (Token_Iterator iter, Token_Iterator end, TokenPtr parent, bool keep, int new_id, const std::string &val);

std::pair<Token_Iterator, bool> Type_Rule
    (Token_Iterator iter, Token_Iterator end, TokenPtr parent, bool keep, int new_id, const int val);

std::pair<Token_Iterator, bool> Or_Rule
    (Token_Iterator iter, Token_Iterator end, TokenPtr parent, bool keep, int new_id, struct Rule lhs, struct Rule rhs);

std::pair<Token_Iterator, bool> And_Rule
    (Token_Iterator iter, Token_Iterator end, TokenPtr parent, bool keep, int new_id, struct Rule lhs, struct Rule rhs);

struct Rule {
    RuleImplPtr impl;

    Rule() : impl(new RuleImpl()) { }
    Rule(int id) : impl(new RuleImpl()) { impl->new_id = id; }
    Rule(RuleFun fun) : impl(new RuleImpl()) { impl->rule = fun; }
    Rule(RuleFun fun, bool keep) : impl(new RuleImpl()) { impl->rule = fun; impl->keep = keep; }

    std::pair<Token_Iterator, bool> operator()(Token_Iterator iter, Token_Iterator end, TokenPtr parent) {
        return (*impl)(iter, end, parent);
    }

    Rule &operator=(const Rule &rule) {
        //*impl = *(rule.impl);
        impl->rule = rule.impl->rule;
        impl->keep = rule.impl->keep;

        return *this;
    }

    Rule operator|(const Rule &rhs) {
        return Rule(boost::bind(Or_Rule, _1, _2, _3, _4, _5, *this, rhs));
    }

    Rule operator<<(const Rule &rhs) {
        return Rule(boost::bind(And_Rule, _1, _2, _3, _4, _5, *this, rhs));
    }

    //const RuleImplPtr get_impl() const { return impl; }

//private:
    //RuleImplPtr impl;

};


Rule Str(const std::string &text, bool keep);
Rule Id(int id, bool keep);

Rule Str(const std::string &text);
Rule Id(int id);

Rule Ign(Rule rule);

#endif /* LANGKIT_PARSER_HPP_ */
