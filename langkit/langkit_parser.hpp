// This file is distributed under the BSD License.
// See LICENSE.TXT for details.

#ifndef LANGKIT_PARSER_HPP_
#define LANGKIT_PARSER_HPP_

#include <boost/function.hpp>

#include "langkit_lexer.hpp"

typedef std::vector<Token>::iterator Token_Iterator;

struct Rule {
    boost::function<Token_Iterator(Token_Iterator iter, Token_Iterator end)> rule;
};

#endif /* LANGKIT_PARSER_HPP_ */
