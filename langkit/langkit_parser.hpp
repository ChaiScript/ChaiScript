// This file is distributed under the BSD License.
// See LICENSE.TXT for details.

#ifndef LANGKIT_PARSER_HPP_
#define LANGKIT_PARSER_HPP_

#include <boost/function.hpp>

#include "langkit_lexer.hpp"

namespace langkit
{
    struct RuleImpl;

    typedef std::vector<TokenPtr>::iterator Token_Iterator;
    typedef boost::function<std::pair<Token_Iterator, bool>(Token_Iterator, Token_Iterator, TokenPtr, bool, int)> RuleFun;
    typedef std::tr1::shared_ptr<RuleImpl> RuleImplPtr;

    struct RuleImpl {
        RuleFun rule;
        bool keep;
        int new_id;

        RuleImpl() : keep(true), new_id(-1) {}
        RuleImpl(int id) : keep(true), new_id(id) {}
        RuleImpl(RuleFun fun) : rule(fun), keep(true), new_id(-1) {}
        RuleImpl(RuleFun fun, bool keep_match) : rule(fun), keep(keep_match), new_id(-1) {}

        std::pair<Token_Iterator, bool> operator()(Token_Iterator iter, Token_Iterator end, TokenPtr parent) {
            return rule(iter, end, parent, keep, new_id);
        }
    };

    //struct Rule;

    template <typename T_Iter>
    std::pair<T_Iter, bool> String_Rule(T_Iter iter, T_Iter end, TokenPtr parent, bool keep, int new_id, const std::string &val) {
        if (iter != end) {
            if ((*iter)->text == val) {
                if (keep) {
                    parent->children.push_back(*iter);
                }
                return std::pair<T_Iter, bool>(++iter, true);
            }
        }

        return std::pair<T_Iter, bool>(iter, false);
    }

    template <typename T_Iter>
    std::pair<T_Iter, bool> Id_Rule(T_Iter iter, T_Iter end, TokenPtr parent, bool keep, int new_id, const int val) {
        if (iter != end) {
            if ((*iter)->identifier == val) {
                if (keep) {
                    parent->children.push_back(*iter);
                }
                return std::pair<T_Iter, bool>(++iter, true);
            }
        }

        return std::pair<T_Iter, bool>(iter, false);
    }

    template <typename T_Iter, typename R_Type>
    std::pair<T_Iter, bool> Or_Rule(T_Iter iter, T_Iter end, TokenPtr parent, bool keep, int new_id, R_Type lhs, R_Type rhs) {
        T_Iter new_iter;
        unsigned int prev_size;
        TokenPtr prev_parent = parent;

        if (new_id != -1) {
            parent = TokenPtr(new Token("", new_id, parent->filename));
        }

        prev_size = parent->children.size();

        if (iter != end) {
            std::pair<T_Iter, bool> result = lhs(iter, end, parent);

            if (result.second) {
                if (new_id != -1) {
                    parent->filename = (*iter)->filename;
                    parent->start = (*iter)->start;
                    if (result.first == iter) {
                        parent->end = (*iter)->start;
                    }
                    else {
                        parent->end = (*(result.first - 1))->end;
                    }
                    prev_parent->children.push_back(parent);
                }
                return std::pair<T_Iter, bool>(result.first, true);
            }
            else {
                if (parent->children.size() != prev_size) {
                    //Clear out the partial matches
                    parent->children.erase(parent->children.begin() + prev_size, parent->children.end());
                }

                result = rhs(iter, end, parent);
                if (result.second) {
                    if (new_id != -1) {

                        parent->filename = (*iter)->filename;
                        parent->start = (*iter)->start;
                        if (result.first == iter) {
                            parent->end = (*iter)->start;
                        }
                        else {
                            parent->end = (*(result.first - 1))->end;
                        }

                        prev_parent->children.push_back(parent);
                    }
                    return std::pair<T_Iter, bool>(result.first, true);
                }
            }
        }

        if (parent->children.size() != prev_size) {
            //Clear out the partial matches
            parent->children.erase(parent->children.begin() + prev_size, parent->children.end());
        }

        return std::pair<T_Iter, bool>(iter, false);
    }

    template <typename T_Iter, typename R_Type>
    std::pair<Token_Iterator, bool> And_Rule(T_Iter iter, T_Iter end, TokenPtr parent, bool keep, int new_id, R_Type lhs, R_Type rhs) {
        T_Iter lhs_iter, rhs_iter;
        unsigned int prev_size;
        TokenPtr prev_parent = parent;

        if (new_id != -1) {
            parent = TokenPtr(new Token("", new_id, parent->filename));
        }

        prev_size = parent->children.size();

        if (iter != end) {
            std::pair<T_Iter, bool> result = lhs(iter, end, parent);

            if (result.second) {
                result = rhs(result.first, end, parent);
                if (result.second) {
                    if (new_id != -1) {

                        parent->filename = (*iter)->filename;
                        parent->start = (*iter)->start;
                        if (result.first == iter) {
                            parent->end = (*iter)->start;
                        }
                        else {
                            parent->end = (*(result.first - 1))->end;
                        }

                        prev_parent->children.push_back(parent);
                    }
                    return std::pair<Token_Iterator, bool>(result.first, true);
                }
            }
        }

        if (parent->children.size() != prev_size) {
            //Clear out the partial matches
            parent->children.erase(parent->children.begin() + prev_size, parent->children.end());
        }

        return std::pair<T_Iter, bool>(iter, false);
    }

    template <typename T_Iter, typename R_Type>
    std::pair<T_Iter, bool> Kleene_Rule
        (T_Iter iter, T_Iter end, TokenPtr parent, bool keep, int new_id, R_Type rule) {

        TokenPtr prev_parent = parent;
        std::pair<T_Iter, bool> result;
        T_Iter new_iter = iter;

        if (iter != end) {
            if (new_id != -1) {
                parent = TokenPtr(new Token("", new_id, parent->filename));
            }

            result.second = true;
            while (result.second == true) {
                result = rule(new_iter, end, parent);
                new_iter = result.first;
            }

            if (new_id != -1) {

                parent->filename = (*iter)->filename;
                parent->start = (*iter)->start;
                if (result.first == iter) {
                    parent->end = (*iter)->start;
                }
                else {
                    parent->end = (*(result.first - 1))->end;
                }

                prev_parent->children.push_back(parent);
            }
            return std::pair<T_Iter, bool>(result.first, true);
        }
        else {
            return std::pair<T_Iter, bool>(iter, true);
        }
    }

    template <typename T_Iter, typename R_Type>
    std::pair<T_Iter, bool> Plus_Rule
        (T_Iter iter, T_Iter end, TokenPtr parent, bool keep, int new_id, R_Type rule) {

        unsigned int prev_size;
        TokenPtr prev_parent = parent;
        T_Iter loop_iter = iter;

        if (new_id != -1) {
            parent = TokenPtr(new Token("", new_id, parent->filename));
        }

        prev_size = parent->children.size();

        if (iter != end) {
            std::pair<T_Iter, bool> result;
            result = rule(loop_iter, end, parent);

            if (result.second == true) {
                loop_iter = result.first;
                result.second = true;
                while ((loop_iter != end) && (result.second == true)) {
                    result = rule(loop_iter, end, parent);
                    loop_iter = result.first;
                }

                if (new_id != -1) {

                    parent->filename = (*iter)->filename;
                    parent->start = (*iter)->start;
                    if (result.first == iter) {
                        parent->end = (*iter)->start;
                    }
                    else {
                        parent->end = (*(result.first - 1))->end;
                    }

                    prev_parent->children.push_back(parent);
                }

                return std::pair<T_Iter, bool>(result.first, true);
            }
        }

        if (parent->children.size() != prev_size) {
            //Clear out the partial matches
            parent->children.erase(parent->children.begin() + prev_size, parent->children.end());
        }

        return std::pair<T_Iter, bool>(iter, false);
    }

    template <typename T_Iter, typename R_Type>
    std::pair<T_Iter, bool> Optional_Rule
        (T_Iter iter, T_Iter end, TokenPtr parent, bool keep, int new_id, R_Type rule) {

        TokenPtr prev_parent = parent;
        T_Iter new_iter = iter;

        if (iter != end) {
            if (new_id != -1) {
                parent = TokenPtr(new Token("", new_id, parent->filename));
            }

            std::pair<T_Iter, bool> result;
            result.second = true;
            if ((new_iter != end) && (result.second == true)) {
                result = rule(new_iter, end, parent);
                new_iter = result.first;
            }

            if (new_id != -1) {

                parent->filename = (*iter)->filename;
                parent->start = (*iter)->start;
                if (result.first == iter) {
                    parent->end = (*iter)->start;
                }
                else {
                    parent->end = (*(result.first - 1))->end;
                }

                prev_parent->children.push_back(parent);
            }
            return std::pair<T_Iter, bool>(result.first, true);
        }
        else {
            return std::pair<T_Iter, bool>(iter, true);
        }
    }

    template <typename T_Iter, typename R_Type>
    std::pair<T_Iter, bool> Epsilon_Rule
        (T_Iter iter, T_Iter end, TokenPtr parent, bool keep, int new_id, R_Type rule) {

        TokenPtr prev_parent = parent;
        T_Iter new_iter = iter;

        if (new_id != -1) {
            parent = TokenPtr(new Token("", new_id, parent->filename));
        }

        std::pair<T_Iter, bool> result;
        if ((new_iter != end)) {
            result = rule(new_iter, end, parent);
            new_iter = result.first;
        }

        if (new_id != -1) {
            parent->filename = (*iter)->filename;
            parent->start = (*iter)->start;
            if (result.first == iter) {
                parent->end = (*iter)->start;
            }
            else {
                parent->end = (*(result.first - 1))->end;
            }

            prev_parent->children.push_back(parent);
        }

        return std::pair<T_Iter, bool>(iter, result.second);
    }

    template <typename T_Iter, typename R_Type>
    std::pair<T_Iter, bool> Wrap_Rule
        (T_Iter iter, T_Iter end, TokenPtr parent, bool keep, int new_id, R_Type rule) {

        TokenPtr prev_parent = parent;
        T_Iter new_iter = iter;

        if (new_id != -1) {
            parent = TokenPtr(new Token("", new_id, parent->filename));
        }

        std::pair<T_Iter, bool> result;
        if ((new_iter != end)) {
            result = rule(new_iter, end, parent);
            new_iter = result.first;
        }

        if (new_id != -1) {
            parent->filename = (*iter)->filename;
            parent->start = (*iter)->start;
            if (result.first == iter) {
                parent->end = (*iter)->start;
            }
            else {
                parent->end = (*(result.first - 1))->end;
            }

            prev_parent->children.push_back(parent);
        }

        return std::pair<T_Iter, bool>(result.first, result.second);
    }

    template <typename T_Iter, typename R_Type>
    std::pair<T_Iter, bool> Ignore_Rule
        (T_Iter iter, T_Iter end, TokenPtr parent, bool keep, int new_id, R_Type rule) {

        rule.impl->keep = false;

        return rule(iter, end, parent);
    }

    struct Rule {
        RuleImplPtr impl;

        Rule() : impl(new RuleImpl()) {}
        Rule(int id) : impl(new RuleImpl(id)) {}
        Rule(RuleFun fun) : impl(new RuleImpl(fun)) {}
        Rule(RuleFun fun, bool keep) : impl(new RuleImpl(fun, keep)) {}

        std::pair<Token_Iterator, bool> operator()(Token_Iterator iter, Token_Iterator end, TokenPtr parent) {
            return (*impl)(iter, end, parent);
        }

        Rule &operator=(const Rule &rule) {
            int prev_id = impl->new_id;
            *impl = *(rule.impl);
            impl->new_id = prev_id;

            return *this;
        }

    };

    inline Rule operator>>(const Rule &lhs, const Rule &rhs) {
        return Rule(boost::bind(And_Rule<Token_Iterator, Rule>, _1, _2, _3, _4, _5, lhs, rhs));
    }

    inline Rule operator|(const Rule &lhs, const Rule &rhs) {
        return Rule(boost::bind(Or_Rule<Token_Iterator, Rule>, _1, _2, _3, _4, _5, lhs, rhs));
    }

    inline Rule operator*(const Rule &operand) {
        return Rule(boost::bind(Kleene_Rule<Token_Iterator, Rule>, _1, _2, _3, _4, _5, operand));
    }

    inline Rule operator+(const Rule &operand) {
        return Rule(boost::bind(Plus_Rule<Token_Iterator, Rule>, _1, _2, _3, _4, _5, operand));
    }

    inline Rule operator~(const Rule &operand) {
        return Rule(boost::bind(Optional_Rule<Token_Iterator, Rule>, _1, _2, _3, _4, _5, operand));
    }


    template<typename ItrType, typename ParamType,
        std::pair<ItrType,bool> (*Function)(ItrType, ItrType, TokenPtr, bool, int, ParamType)>
    struct Rule_Builder
    {
        Rule_Builder(ParamType p, bool t_keep = true)
        : m_p(p), m_keep(t_keep)
        {

        }

        // Auto conversion operator is the glue here.
        // In one sense this option cleans up the impl quite a bit, with much fewer code
        // repeats in all the rule builders.
        // In another sense, it might take a couple of tries to get it right.
        operator Rule() {
            return Rule(boost::bind(Function, _1, _2, _3, _4, _5, m_p), m_keep);
        }

        ParamType m_p;
        bool m_keep;
    };


    typedef Rule_Builder<Token_Iterator, Rule, &Epsilon_Rule<Token_Iterator, Rule> > Epsilon;
    typedef Rule_Builder<Token_Iterator, Rule, &Wrap_Rule<Token_Iterator, Rule> > Wrap;
    typedef Rule_Builder<Token_Iterator, Rule, &Ignore_Rule<Token_Iterator, Rule> > Ign;
    typedef Rule_Builder<Token_Iterator, int, &Id_Rule<Token_Iterator> > Id;
    typedef Rule_Builder<Token_Iterator, const std::string&, &String_Rule<Token_Iterator> > Str;
}

#endif /* LANGKIT_PARSER_HPP_ */
