// This file is distributed under the BSD License.
// See LICENSE.TXT for details.

#include <boost/bind.hpp>

#include <iostream>
#include <utility>

#include "langkit_lexer.hpp"
#include "langkit_parser.hpp"

std::pair<Token_Iterator, bool> String_Rule(Token_Iterator iter, Token_Iterator end, TokenPtr parent, bool keep, int new_id, const std::string &val) {
    if (iter != end) {
        if ((*iter)->text == val) {
            if (keep) {
                parent->children.push_back(*iter);
            }
            return std::pair<Token_Iterator, bool>(++iter, true);
        }
    }

    return std::pair<Token_Iterator, bool>(iter, false);
}

std::pair<Token_Iterator, bool> Type_Rule(Token_Iterator iter, Token_Iterator end, TokenPtr parent, bool keep, int new_id, const int val) {
    if (iter != end) {
        if ((*iter)->identifier == val) {
            if (keep) {
                parent->children.push_back(*iter);
            }
            return std::pair<Token_Iterator, bool>(++iter, true);
        }
    }

    return std::pair<Token_Iterator, bool>(iter, false);
}

std::pair<Token_Iterator, bool> Or_Rule(Token_Iterator iter, Token_Iterator end, TokenPtr parent, bool keep, int new_id, Rule lhs, Rule rhs) {
    Token_Iterator new_iter;
    unsigned int prev_size = parent->children.size();
    TokenPtr prev_parent = parent;

    if (new_id != -1) {
        parent = TokenPtr(new Token("", new_id, parent->filename));
    }

    if (iter != end) {
        std::pair<Token_Iterator, bool> result = lhs(iter, end, parent);

        if (result.second) {

            parent->filename = (*iter)->filename;
            parent->start = (*iter)->start;
            parent->end = (*(result.first - 1))->end;


            return std::pair<Token_Iterator, bool>(result.first, true);
        }
        else {
            result = rhs(iter, end, parent);
            if (result.second) {
                if (new_id != -1) {

                    parent->filename = (*iter)->filename;
                    parent->start = (*iter)->start;
                    parent->end = (*(result.first - 1))->end;


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

    return std::pair<Token_Iterator, bool>(iter, false);
}

std::pair<Token_Iterator, bool> And_Rule(Token_Iterator iter, Token_Iterator end, TokenPtr parent, bool keep, int new_id, Rule lhs, Rule rhs) {
    Token_Iterator lhs_iter, rhs_iter;
    unsigned int prev_size;
    TokenPtr prev_parent = parent;

    if (new_id != -1) {
        parent = TokenPtr(new Token("", new_id, parent->filename));
    }

    prev_size = parent->children.size();

    if (iter != end) {
        std::pair<Token_Iterator, bool> result = lhs(iter, end, parent);

        if ((result.second) && (result.first != end)) {
            result = rhs(result.first, end, parent);
            if (result.second) {
                if (new_id != -1) {

                    parent->filename = (*iter)->filename;
                    parent->start = (*iter)->start;
                    parent->end = (*(result.first - 1))->end;


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

    return std::pair<Token_Iterator, bool>(iter, false);
}

std::pair<Token_Iterator, bool> Kleene_Rule
    (Token_Iterator iter, Token_Iterator end, TokenPtr parent, bool keep, int new_id, struct Rule rule) {

    TokenPtr prev_parent = parent;
    std::pair<Token_Iterator, bool> result;
    Token_Iterator new_iter = iter;

    if (new_id != -1) {
        parent = TokenPtr(new Token("", new_id, parent->filename));
    }

    result.second = true;
    while ((new_iter != end) && (result.second == true)) {
        result = rule(new_iter, end, parent);
        new_iter = result.first;
    }

    if (new_id != -1) {

        parent->filename = (*iter)->filename;
        parent->start = (*iter)->start;
        parent->end = (*(result.first - 1))->end;

        prev_parent->children.push_back(parent);
    }
    return std::pair<Token_Iterator, bool>(result.first, true);
}

std::pair<Token_Iterator, bool> Plus_Rule
    (Token_Iterator iter, Token_Iterator end, TokenPtr parent, bool keep, int new_id, struct Rule rule) {

    unsigned int prev_size;
    TokenPtr prev_parent = parent;
    Token_Iterator loop_iter = iter;

    if (new_id != -1) {
        parent = TokenPtr(new Token("", new_id, parent->filename));
    }

    prev_size = parent->children.size();

    if (iter != end) {
        std::pair<Token_Iterator, bool> result;
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
                parent->end = (*(result.first - 1))->end;


                prev_parent->children.push_back(parent);
            }

            return std::pair<Token_Iterator, bool>(result.first, true);
        }
    }

    if (parent->children.size() != prev_size) {
        //Clear out the partial matches
        parent->children.erase(parent->children.begin() + prev_size, parent->children.end());
    }

    return std::pair<Token_Iterator, bool>(iter, false);
}

std::pair<Token_Iterator, bool> Optional_Rule
    (Token_Iterator iter, Token_Iterator end, TokenPtr parent, bool keep, int new_id, struct Rule rule) {

    TokenPtr prev_parent = parent;
    Token_Iterator new_iter = iter;

    if (new_id != -1) {
        parent = TokenPtr(new Token("", new_id, parent->filename));
    }

    std::pair<Token_Iterator, bool> result;
    result.second = true;
    if ((new_iter != end) && (result.second == true)) {
        result = rule(new_iter, end, parent);
        new_iter = result.first;
    }

    if (new_id != -1) {

        parent->filename = (*iter)->filename;
        parent->start = (*iter)->start;
        parent->end = (*(result.first - 1))->end;


        prev_parent->children.push_back(parent);
    }
    return std::pair<Token_Iterator, bool>(result.first, true);
}


Rule Str(const std::string &text, bool keep) {
    return Rule(boost::bind(String_Rule, _1, _2, _3, _4, _5, text), keep);
}

Rule Id(int id, bool keep) {
    return Rule(boost::bind(Type_Rule, _1, _2, _3, _4, _5, id), keep);
}

Rule Str(const std::string &text) {
    return Rule(boost::bind(String_Rule, _1, _2, _3, _4, _5, text));
}

Rule Id(int id) {
    return Rule(boost::bind(Type_Rule, _1, _2, _3, _4, _5, id));
}

Rule Ign(Rule rule) {
    rule.impl->keep = false;

    return rule;
}
