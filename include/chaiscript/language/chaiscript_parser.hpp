// This file is distributed under the BSD License.
// See "license.txt" for details.
// Copyright 2009-2010, Jonathan Turner (jturner@minnow-lang.org)
// and Jason Turner (lefticus@gmail.com)
// http://www.chaiscript.com

#ifndef CHAISCRIPT_PARSER_HPP_
#define CHAISCRIPT_PARSER_HPP_

#include <boost/assign/std/vector.hpp>

#include <exception>
#include <fstream>
#include <sstream>

#include "chaiscript_prelude.hpp"
#include "chaiscript_common.hpp"

namespace chaiscript
{
    class ChaiScript_Parser {

        std::string::iterator input_pos, input_end;
        int line, col;
        std::string multiline_comment_begin, multiline_comment_end;
        std::string singleline_comment;
        const char *filename;
        std::vector<TokenPtr> match_stack;

        std::vector<std::vector<std::string> > operator_matches;
        std::vector<Token_Type::Type> operators;

    public:
        ChaiScript_Parser() {           
            multiline_comment_begin = "/*";
            multiline_comment_end = "*/";
            singleline_comment = "//";

            setup_operators();
        }

        ChaiScript_Parser(const ChaiScript_Parser &); // explicitly unimplemented copy constructor
        ChaiScript_Parser &operator=(const ChaiScript_Parser &); // explicitly unimplemented assignment operator

        void setup_operators() {
            using namespace boost::assign;
                        
            operators.push_back(Token_Type::Logical_Or);
            std::vector<std::string> logical_or;
            logical_or += "||";
            operator_matches.push_back(logical_or);

            operators.push_back(Token_Type::Logical_And);
            std::vector<std::string> logical_and;
            logical_and += "&&";
            operator_matches.push_back(logical_and);

            operators.push_back(Token_Type::Bitwise_Or);
            std::vector<std::string> bitwise_or;
            bitwise_or += "|";
            operator_matches.push_back(bitwise_or);

            operators.push_back(Token_Type::Bitwise_Xor);
            std::vector<std::string> bitwise_xor;
            bitwise_xor += "^";
            operator_matches.push_back(bitwise_xor);           

            operators.push_back(Token_Type::Bitwise_And);
            std::vector<std::string> bitwise_and;
            bitwise_and += "&";
            operator_matches.push_back(bitwise_and);

            operators.push_back(Token_Type::Equality);
            std::vector<std::string> equality;
            equality += "==", "!=";
            operator_matches.push_back(equality);           

            operators.push_back(Token_Type::Comparison);
            std::vector<std::string> comparison;
            comparison += "<", "<=", ">", ">=";
            operator_matches.push_back(comparison);

            operators.push_back(Token_Type::Shift);
            std::vector<std::string> shift;
            shift += "<<", ">>";
            operator_matches.push_back(shift);           

            operators.push_back(Token_Type::Additive);
            std::vector<std::string> additive;
            additive += "+", "-";
            operator_matches.push_back(additive);

            operators.push_back(Token_Type::Multiplicative);
            std::vector<std::string> multiplicative;
            multiplicative += "*", "/", "%";
            operator_matches.push_back(multiplicative);

            operators.push_back(Token_Type::Dot_Access);
            std::vector<std::string> dot_access;
            dot_access += ".";
            operator_matches.push_back(dot_access);
        }
        /**
         * Prints the parsed tokens as a tree
         */
        void debug_print(TokenPtr t, std::string prepend = "") {
            std::cout << prepend << "(" << token_type_to_string(t->identifier) << ") " << t->text << " : " << t->start.line << ", " << t->start.column << std::endl;
            for (unsigned int j = 0; j < t->children.size(); ++j) {
                debug_print(t->children[j], prepend + "  ");
            }
        }

        /**
         * Shows the current stack of matched tokens
         */
        void show_match_stack() {
            for (unsigned int i = 0; i < match_stack.size(); ++i) {
                debug_print(match_stack[i]);
            }
        }

        /**
         * Clears the stack of matched tokens
         */
        void clear_match_stack() {
            match_stack.clear();
        }

        /**
         * Returns the front-most AST node
         */
        TokenPtr ast() {
            return match_stack.front();
        }

        /**
         * Helper function that collects tokens from a starting position to the top of the stack into a new AST node
         */
        void build_match(Token_Type::Type match_type, int match_start) {
            //so we want to take everything to the right of this and make them children
            if (match_start != int(match_stack.size())) {
                TokenPtr t(new Token("", match_type, filename, match_stack[match_start]->start.line, match_stack[match_start]->start.column, line, col));
                t->children.assign(match_stack.begin() + (match_start), match_stack.end());
                match_stack.erase(match_stack.begin() + (match_start), match_stack.end());
                match_stack.push_back(t);
            }
            else {
                //todo: fix the fact that a successful match that captured no tokens does't have any real start position
                TokenPtr t(new Token("", match_type, filename, line, col, line, col));
                match_stack.push_back(t);
            }
        }

        /**
         * Skips any multi-line or single-line comment
         */
        bool SkipComment() {
            bool retval = false;

            if (Symbol_(multiline_comment_begin.c_str())) {
                while (input_pos != input_end) {
                    if (Symbol_(multiline_comment_end.c_str())) {
                        break;
                    }
                    else if (!Eol_()) {
                        ++col;
                        ++input_pos;
                    }
                }
                retval = true;
            }
            else if (Symbol_(singleline_comment.c_str())) {
                while (input_pos != input_end) {
                    if (Symbol_("\r\n") || Char_('\n')) {
                        ++line;
                        col = 1;
                        break;
                    }
                    else {
                        ++col;
                        ++input_pos;
                    }
                }
                retval = true;
            }
            return retval;
        }

        /**
         * Skips ChaiScript whitespace, which means space and tab, but not cr/lf
         */
        bool SkipWS() {
            bool retval = false;
            while (input_pos != input_end) {
                if ((*input_pos == ' ') || (*input_pos == '\t')) {
                    ++input_pos;
                    ++col;
                    retval = true;
                }
                else if (SkipComment()) {
                    retval = true;
                }
                else {
                    break;
                }
            }
            return retval;
        }

        /**
         * Reads a floating point value from input, without skipping initial whitespace
         */
        bool Float_() {
            bool retval = false;
            std::string::iterator start = input_pos;

            if ((input_pos != input_end) && (((*input_pos >= '0') && (*input_pos <= '9')) || (*input_pos == '.'))) {
                while ((input_pos != input_end) && (*input_pos >= '0') && (*input_pos <= '9')) {
                    ++input_pos;
                    ++col;
                }
                if ((input_pos != input_end) && (*input_pos == '.')) {
                    ++input_pos;
                    ++col;
                    if ((input_pos != input_end) && (*input_pos >= '0') && (*input_pos <= '9')) {
                        retval = true;
                        while ((input_pos != input_end) && (*input_pos >= '0') && (*input_pos <= '9')) {
                            ++input_pos;
                            ++col;
                        }
                    }
                    else {
                        --input_pos;
                        --col;
                    }
                }
            }
            return retval;
        }

        /**
         * Reads a floating point value from input, without skipping initial whitespace
         */
        bool Hex_() {
            bool retval = false;
            if ((input_pos != input_end) && (*input_pos == '0')) {
                ++input_pos;
                ++col;

                if ((input_pos != input_end) && ((*input_pos == 'x') || (*input_pos == 'X'))) {
                    ++input_pos;
                    ++col;
                    if ((input_pos != input_end) && (((*input_pos >= '0') && (*input_pos <= '9')) ||
                            ((*input_pos >= 'a') && (*input_pos <= 'f')) ||
                            ((*input_pos >= 'A') && (*input_pos <= 'F')))) {
                        retval = true;
                        while ((input_pos != input_end) && (((*input_pos >= '0') && (*input_pos <= '9')) ||
                        ((*input_pos >= 'a') && (*input_pos <= 'f')) ||
                        ((*input_pos >= 'A') && (*input_pos <= 'F')))) {
                            ++input_pos;
                            ++col;
                        }
                    }
                    else {
                        --input_pos;
                        --col;
                    }
                }
                else {
                    --input_pos;
                    --col;
                }
            }

            return retval;
        }

        /**
         * Reads a floating point value from input, without skipping initial whitespace
         */
        bool Binary_() {
            bool retval = false;
            if ((input_pos != input_end) && (*input_pos == '0')) {
                ++input_pos;
                ++col;

                if ((input_pos != input_end) && ((*input_pos == 'b') || (*input_pos == 'B'))) {
                    ++input_pos;
                    ++col;
                    if ((input_pos != input_end) && ((*input_pos >= '0') && (*input_pos <= '1'))) {
                        retval = true;
                        while ((input_pos != input_end) && ((*input_pos >= '0') && (*input_pos <= '1'))) {
                            ++input_pos;
                            ++col;
                        }
                    }
                    else {
                        --input_pos;
                        --col;
                    }
                }
                else {
                    --input_pos;
                    --col;
                }
            }

            return retval;
        }

        /**
         * Reads a number from the input, detecting if it's an integer or floating point
         */
        bool Num(bool capture = false) {
            SkipWS();

            if (!capture) {
                return Hex_() || Float_();
            }
            else {
                std::string::iterator start = input_pos;
                int prev_col = col;
                int prev_line = line;
                if ((input_pos != input_end) && (((*input_pos >= '0') && (*input_pos <= '9')) || (*input_pos == '.')) ) {
                    if (Hex_()) {
                        std::string match(start, input_pos);
                        std::stringstream ss(match);
                        unsigned int temp_int;
                        ss >> std::hex >> temp_int;

                        std::ostringstream out_int;
                        out_int << int(temp_int);
                        TokenPtr t(new Token(out_int.str(), Token_Type::Int, filename, prev_line, prev_col, line, col));
                        match_stack.push_back(t);
                        return true;
                    }
                    if (Binary_()) {
                        std::string match(start, input_pos);
                        int temp_int = 0;
                        unsigned int pos = 0, end = match.length();

                        while ((pos < end) && (pos < (2 + sizeof(int) * 8))) {
                            temp_int <<= 1;
                            if (match[pos] == '1') {
                                temp_int += 1;
                            }
                            ++pos;
                        }

                        std::ostringstream out_int;
                        out_int << temp_int;
                        TokenPtr t(new Token(out_int.str(), Token_Type::Int, filename, prev_line, prev_col, line, col));
                        match_stack.push_back(t);
                        return true;
                    }
                    if (Float_()) {
                        std::string match(start, input_pos);
                        TokenPtr t(new Token(match, Token_Type::Float, filename, prev_line, prev_col, line, col));
                        match_stack.push_back(t);
                        return true;
                    }
                    else {
                        std::string match(start, input_pos);
                        if ((match.size() > 0) && (match[0] == '0')) {
                            std::stringstream ss(match);
                            unsigned int temp_int;
                            ss >> std::oct >> temp_int;

                            std::ostringstream out_int;
                            out_int << int(temp_int);
                            TokenPtr t(new Token(out_int.str(), Token_Type::Int, filename, prev_line, prev_col, line, col));
                            match_stack.push_back(t);
                        }
                        else {
                            TokenPtr t(new Token(match, Token_Type::Int, filename, prev_line, prev_col, line, col));
                            match_stack.push_back(t);
                        }
                        return true;
                    }
                }
                else {
                    return false;
                }
            }
        }

        /**
         * Reads an identifier from input which conforms to C's identifier naming conventions, without skipping initial whitespace
         */
        bool Id_() {
            bool retval = false;
            if ((input_pos != input_end) && (((*input_pos >= 'A') && (*input_pos <= 'Z')) || (*input_pos == '_') || ((*input_pos >= 'a') && (*input_pos <= 'z')))) {
                retval = true;
                while ((input_pos != input_end) && (((*input_pos >= 'A') && (*input_pos <= 'Z')) || (*input_pos == '_') || ((*input_pos >= 'a') && (*input_pos <= 'z'))
                        || ((*input_pos >= '0') && (*input_pos <= '9')))) {
                    ++input_pos;
                    ++col;
                }
            }
            else if ((input_pos != input_end) && (*input_pos == '`')) {
                retval = true;
                ++col;
                ++input_pos;
                std::string::iterator start = input_pos;
                
                while ((input_pos != input_end) && (*input_pos != '`')) {
                    if (Eol()) {
                        throw Eval_Error("Carriage return in identifier literal", File_Position(line, col), filename);
                    }
                    else {
                        ++input_pos;
                        ++col;
                    }
                }

                if (start == input_pos) {
                    throw Eval_Error("Missing contents of identifier literal", File_Position(line, col), filename);
                }
                else if (input_pos == input_end) {
                    throw Eval_Error("Incomplete identifier literal", File_Position(line, col), filename);
                }

                ++col;
                ++input_pos;
            }
            return retval;
        }

        /**
         * Reads (and potentially captures) an identifier from input
         */
        bool Id(bool capture = false) {
            SkipWS();

            if (!capture) {
                return Id_();
            }
            else {
                std::string::iterator start = input_pos;
                int prev_col = col;
                int prev_line = line;
                if (Id_()) {
                    if (*start == '`') {
                        //Id Literal
                        std::string match(start+1, input_pos-1);
                        TokenPtr t(new Token(match, Token_Type::Id, filename, prev_line, prev_col, line, col));
                        match_stack.push_back(t);
                        return true;
                    }
                    else {
                        std::string match(start, input_pos);
                        TokenPtr t(new Token(match, Token_Type::Id, filename, prev_line, prev_col, line, col));
                        match_stack.push_back(t);
                        return true;
                    }
                }
                else {
                    return false;
                }
            }
        }

        /**
         * Checks for a node annotation of the form "#<annotation>"
         */
        bool Annotation() {
            SkipWS();
            std::string::iterator start = input_pos;
            int prev_col = col;
            int prev_line = line;
            if (Symbol_("#")) {
                do {
                    while (input_pos != input_end) {
                        if (Eol_()) {
                            break;
                        }
                        else {
                            ++col;
                            ++input_pos;
                        }
                    }
                } while (Symbol("#"));

                std::string match(start, input_pos);
                TokenPtr t(new Token(match, Token_Type::Annotation, filename, prev_line, prev_col, line, col));
                match_stack.push_back(t);
                return true;
            }
            else {
                return false;
            }
        }

        /**
         * Reads a quoted string from input, without skipping initial whitespace
         */
        bool Quoted_String_() {
            bool retval = false;
            char prev_char = 0;
            if ((input_pos != input_end) && (*input_pos == '\"')) {
                retval = true;
                prev_char = *input_pos;
                ++input_pos;
                ++col;

                while ((input_pos != input_end) && ((*input_pos != '\"') || ((*input_pos == '\"') && (prev_char == '\\')))) {
                    if (!Eol_()) {
                        if (prev_char == '\\') {
                            prev_char = 0;
                        }
                        else {
                            prev_char = *input_pos;
                        }
                        ++input_pos;
                        ++col;
                    }
                }

                if (input_pos != input_end) {
                    ++input_pos;
                    ++col;
                }
                else {
                    throw Eval_Error("Unclosed quoted string", File_Position(line, col), filename);
                }
            }
            return retval;
        }

        /**
         * Reads (and potentially captures) a quoted string from input.  Translates escaped sequences.
         */
        bool Quoted_String(bool capture = false) {
            SkipWS();

            if (!capture) {
                return Quoted_String_();
            }
            else {
                std::string::iterator start = input_pos;
                int prev_col = col;
                int prev_line = line;
                if (Quoted_String_()) {
                    std::string match;
                    bool is_escaped = false;
                    bool is_interpolated = false;
                    bool saw_interpolation_marker = false;
                    int prev_stack_top = match_stack.size();

                    //for (std::string::iterator s = start + 1, end = input_pos - 1; s != end; ++s) {
                    std::string::iterator s = start + 1, end = input_pos - 1;

                    while (s != end) {
                        if (saw_interpolation_marker) {
                            if (*s == '{') {
                                //We've found an interpolation point

                                if (is_interpolated) {
                                    //If we've seen previous interpolation, add on instead of making a new one
                                    TokenPtr plus(new Token("+", Token_Type::Str, filename, prev_line, prev_col, line, col));
                                    match_stack.push_back(plus);

                                    TokenPtr t(new Token(match, Token_Type::Quoted_String, filename, prev_line, prev_col, line, col));
                                    match_stack.push_back(t);

                                    build_match(Token_Type::Additive, prev_stack_top);
                                }
                                else {
                                    TokenPtr t(new Token(match, Token_Type::Quoted_String, filename, prev_line, prev_col, line, col));
                                    match_stack.push_back(t);
                                }

                                //We've finished with the part of the string up to this point, so clear it
                                match = "";

                                TokenPtr plus(new Token("+", Token_Type::Str, filename, prev_line, prev_col, line, col));
                                match_stack.push_back(plus);

                                std::string eval_match;

                                ++s;
                                while ((*s != '}') && (s != end)) {
                                    eval_match.push_back(*s);
                                    ++s;
                                }
                                if (*s == '}') {
                                    is_interpolated = true;
                                    ++s;

                                    int tostr_stack_top = match_stack.size();

                                    TokenPtr tostr(new Token("to_string", Token_Type::Id, filename, prev_line, prev_col, line, col));
                                    match_stack.push_back(tostr);

                                    int ev_stack_top = match_stack.size();

                                    TokenPtr ev(new Token("eval", Token_Type::Id, filename, prev_line, prev_col, line, col));
                                    match_stack.push_back(ev);

                                    int arg_stack_top = match_stack.size();

                                    TokenPtr t(new Token(eval_match, Token_Type::Quoted_String, filename, prev_line, prev_col, line, col));
                                    match_stack.push_back(t);

                                    build_match(Token_Type::Arg_List, arg_stack_top);

                                    build_match(Token_Type::Inplace_Fun_Call, ev_stack_top);

                                    build_match(Token_Type::Arg_List, ev_stack_top);

                                    build_match(Token_Type::Fun_Call, tostr_stack_top);

                                    build_match(Token_Type::Additive, prev_stack_top);
                                }
                                else {
                                    throw Eval_Error("Unclosed in-string eval", File_Position(prev_line, prev_col), filename);
                                }
                            }
                            else {
                                match.push_back('$');
                            }
                            saw_interpolation_marker = false;
                        }
                        else {
                            if (*s == '\\') {
                                if (is_escaped) {
                                    match.push_back('\\');
                                    is_escaped = false;
                                }
                                else {
                                    is_escaped = true;
                                }
                            }
                            else {
                                if (is_escaped) {
                                    switch (*s) {
                                        case ('b') : match.push_back('\b'); break;
                                        case ('f') : match.push_back('\f'); break;
                                        case ('n') : match.push_back('\n'); break;
                                        case ('r') : match.push_back('\r'); break;
                                        case ('t') : match.push_back('\t'); break;
                                        case ('\'') : match.push_back('\''); break;
                                        case ('\"') : match.push_back('\"'); break;
                                        case ('$') : match.push_back('$'); break;
                                        default: throw Eval_Error("Unknown escaped sequence in string", File_Position(prev_line, prev_col), filename);
                                    }
                                }
                                else if (*s == '$') {
                                    saw_interpolation_marker = true;
                                }
                                else {
                                    match.push_back(*s);
                                }
                                is_escaped = false;
                            }
                            ++s;
                        }
                    }
                    if (is_interpolated) {
                        TokenPtr plus(new Token("+", Token_Type::Str, filename, prev_line, prev_col, line, col));
                        match_stack.push_back(plus);

                        TokenPtr t(new Token(match, Token_Type::Quoted_String, filename, prev_line, prev_col, line, col));
                        match_stack.push_back(t);

                        build_match(Token_Type::Additive, prev_stack_top);
                    }
                    else {
                        TokenPtr t(new Token(match, Token_Type::Quoted_String, filename, prev_line, prev_col, line, col));
                        match_stack.push_back(t);
                    }
                    return true;
                }
                else {
                    return false;
                }
            }
        }

        /**
         * Reads a character group from input, without skipping initial whitespace
         */
        bool Single_Quoted_String_() {
            bool retval = false;
            char prev_char = 0;
            if ((input_pos != input_end) && (*input_pos == '\'')) {
                retval = true;
                prev_char = *input_pos;
                ++input_pos;
                ++col;

                while ((input_pos != input_end) && ((*input_pos != '\'') || ((*input_pos == '\'') && (prev_char == '\\')))) {
                    if (!Eol_()) {
                        if (prev_char == '\\') {
                            prev_char = 0;
                        }
                        else {
                            prev_char = *input_pos;
                        }
                        ++input_pos;
                        ++col;
                    }
                }

                if (input_pos != input_end) {
                    ++input_pos;
                    ++col;
                }
                else {
                    throw Eval_Error("Unclosed single-quoted string", File_Position(line, col), filename);
                }
            }
            return retval;
        }

        /**
         * Reads (and potentially captures) a char group from input.  Translates escaped sequences.
         */
        bool Single_Quoted_String(bool capture = false) {
            SkipWS();

            if (!capture) {
                return Single_Quoted_String_();
            }
            else {
                std::string::iterator start = input_pos;
                int prev_col = col;
                int prev_line = line;
                if (Single_Quoted_String_()) {
                    std::string match;
                    bool is_escaped = false;
                    for (std::string::iterator s = start + 1, end = input_pos - 1; s != end; ++s) {
                        if (*s == '\\') {
                            if (is_escaped) {
                                match.push_back('\\');
                                is_escaped = false;
                            }
                            else {
                                is_escaped = true;
                            }
                        }
                        else {
                            if (is_escaped) {
                                switch (*s) {
                                    case ('b') : match.push_back('\b'); break;
                                    case ('f') : match.push_back('\f'); break;
                                    case ('n') : match.push_back('\n'); break;
                                    case ('r') : match.push_back('\r'); break;
                                    case ('t') : match.push_back('\t'); break;
                                    case ('\'') : match.push_back('\''); break;
                                    case ('\"') : match.push_back('\"'); break;
                                    default: throw Eval_Error("Unknown escaped sequence in string", File_Position(prev_line, prev_col), filename);
                                }
                            }
                            else {
                                match.push_back(*s);
                            }
                            is_escaped = false;
                        }
                    }
                    TokenPtr t(new Token(match, Token_Type::Single_Quoted_String, filename, prev_line, prev_col, line, col));
                    match_stack.push_back(t);
                    return true;
                }
                else {
                    return false;
                }
            }
        }

        /**
         * Reads a char from input if it matches the parameter, without skipping initial whitespace
         */
        bool Char_(char c) {
            bool retval = false;
            if ((input_pos != input_end) && (*input_pos == c)) {
                ++input_pos;
                ++col;
                retval = true;
            }

            return retval;
        }

        /**
         * Reads (and potentially captures) a char from input if it matches the parameter
         */
        bool Char(char c, bool capture = false) {
            SkipWS();

            if (!capture) {
                return Char_(c);
            }
            else {
                std::string::iterator start = input_pos;
                int prev_col = col;
                int prev_line = line;
                if (Char_(c)) {
                    std::string match(start, input_pos);
                    TokenPtr t(new Token(match, Token_Type::Char, filename, prev_line, prev_col, line, col));
                    match_stack.push_back(t);
                    return true;
                }
                else {
                    return false;
                }
            }
        }

        /**
         * Reads a string from input if it matches the parameter, without skipping initial whitespace
         */
        bool Keyword_(const char *s) {
            bool retval = false;
            int len = strlen(s);

            if ((input_end - input_pos) >= len) {
                std::string::iterator tmp = input_pos;
                for (int i = 0; i < len; ++i) {
                    if (*tmp != s[i]) {
                        return false;
                    }
                    ++tmp;
                }
                retval = true;
                input_pos = tmp;
                col += len;
            }

            return retval;
        }

        /**
         * Reads (and potentially captures) a string from input if it matches the parameter
         */
        bool Keyword(const char *s, bool capture = false) {
            SkipWS();

            if (!capture) {
                std::string::iterator start = input_pos;
                int prev_col = col;
                int prev_line = line;
                bool retval = Keyword_(s);
                if (retval) {
                    //todo: fix this.  Hacky workaround for preventing substring matches
                    if ((input_pos != input_end) && (((*input_pos >= 'A') && (*input_pos <= 'Z')) || (*input_pos == '_') || ((*input_pos >= 'a') && (*input_pos <= 'z'))
                            || ((*input_pos >= '0') && (*input_pos <= '9')))) {
                        input_pos = start;
                        col = prev_col;
                        line = prev_line;
                        return false;
                    }
                    return true;
                }
                else {
                    return retval;
                }
            }
            else {
                std::string::iterator start = input_pos;
                int prev_col = col;
                int prev_line = line;
                if (Keyword_(s)) {
                    //todo: fix this.  Hacky workaround for preventing substring matches
                    if ((input_pos != input_end) && (((*input_pos >= 'A') && (*input_pos <= 'Z')) || (*input_pos == '_') || ((*input_pos >= 'a') && (*input_pos <= 'z'))
                            || ((*input_pos >= '0') && (*input_pos <= '9')))) {
                        input_pos = start;
                        col = prev_col;
                        line = prev_line;
                        return false;
                    }
                    std::string match(start, input_pos);
                    TokenPtr t(new Token(match, Token_Type::Str, filename, prev_line, prev_col, line, col));
                    match_stack.push_back(t);
                    return true;
                }
                else {
                    return false;
                }
            }
        }

        /**
         * Reads a symbol group from input if it matches the parameter, without skipping initial whitespace
         */
        bool Symbol_(const char *s) {
            bool retval = false;
            int len = strlen(s);

            if ((input_end - input_pos) >= len) {
                std::string::iterator tmp = input_pos;
                for (int i = 0; i < len; ++i) {
                    if (*tmp != s[i]) {
                        return false;
                    }
                    ++tmp;
                }
                retval = true;
                input_pos = tmp;
                col += len;
            }

            return retval;
        }

        /**
         * Reads (and potentially captures) a symbol group from input if it matches the parameter
         */
        bool Symbol(const char *s, bool capture = false, bool disallow_prevention=false) {
            SkipWS();

            if (!capture) {
                std::string::iterator start = input_pos;
                int prev_col = col;
                int prev_line = line;
                bool retval = Symbol_(s);
                if (retval) {
                    //todo: fix this.  Hacky workaround for preventing substring matches
                    if ((input_pos != input_end) && (disallow_prevention == false) && ((*input_pos == '+') || (*input_pos == '-') || (*input_pos == '*') || (*input_pos == '/') 
                            || (*input_pos == '|') || (*input_pos == '&') || (*input_pos == '^') || (*input_pos == '=') || (*input_pos == '.') || (*input_pos == '<') || (*input_pos == '>'))) {
                        input_pos = start;
                        col = prev_col;
                        line = prev_line;
                        return false;
                    }
                    return true;
                }
                else {
                    return retval;
                }
            }
            else {
                std::string::iterator start = input_pos;
                int prev_col = col;
                int prev_line = line;
                if (Symbol_(s)) {
                    //todo: fix this.  Hacky workaround for preventing substring matches
                    if ((input_pos != input_end) && (disallow_prevention == false) && ((*input_pos == '+') || (*input_pos == '-') || (*input_pos == '*') || (*input_pos == '/')
                            || (*input_pos == '|') || (*input_pos == '&') || (*input_pos == '^') || (*input_pos == '=') || (*input_pos == '.') || (*input_pos == '<') || (*input_pos == '>'))) {
                        input_pos = start;
                        col = prev_col;
                        line = prev_line;
                        return false;
                    }
                    else {
                        std::string match(start, input_pos);
                        TokenPtr t(new Token(match, Token_Type::Str, filename, prev_line, prev_col, line, col));
                        match_stack.push_back(t);
                        return true;
                    }
                }
                else {
                    return false;
                }
            }
        }

        /**
         * Reads an end-of-line group from input, without skipping initial whitespace
         */
        bool Eol_() {
            bool retval = false;

            if ((input_pos != input_end) && (Symbol_("\r\n") || Char_('\n'))) {
                retval = true;
                ++line;
                col = 1;
            }
            else if ((input_pos != input_end) && Char_(';')) {
                retval = true;
            }

            return retval;
        }

        /**
         * Reads (and potentially captures) an end-of-line group from input
         */
        bool Eol(bool capture = false) {
            SkipWS();

            if (!capture) {
                return Eol_();
            }
            else {
                std::string::iterator start = input_pos;
                int prev_col = col;
                int prev_line = line;
                if (Eol_()) {
                    std::string match(start, input_pos);
                    TokenPtr t(new Token(match, Token_Type::Eol, filename, prev_line, prev_col, line, col));
                    match_stack.push_back(t);
                    return true;
                }
                else {
                    return false;
                }
            }
        }

        /**
         * Reads a comma-separated list of values from input
         */
        bool Arg_List() {
            bool retval = false;

            int prev_stack_top = match_stack.size();

            if (Equation()) {
                retval = true;
                while (Eol());
                if (Char(',')) {
                    do {
                        while (Eol());
                        if (!Equation()) {
                            throw Eval_Error("Unexpected value in parameter list", match_stack.back());
                        }
                    } while (retval && Char(','));
                }
                build_match(Token_Type::Arg_List, prev_stack_top);
            }

            return retval;
        }

        /**
         * Reads possible special container values, including ranges and map_pairs
         */
        bool Container_Arg_List() {
            bool retval = false;

            int prev_stack_top = match_stack.size();

            if (Value_Range()) {
                retval = true;
                build_match(Token_Type::Arg_List, prev_stack_top);
            }
            else if (Map_Pair()) {
                retval = true;
                while (Eol());
                if (Char(',')) {
                    do {
                        while (Eol());
                        if (!Map_Pair()) {
                            throw Eval_Error("Unexpected value in container", match_stack.back());
                        }
                    } while (retval && Char(','));
                }
                build_match(Token_Type::Arg_List, prev_stack_top);
            }

            return retval;

        }

        /**
         * Reads a lambda (anonymous function) from input
         */
        bool Lambda() {
            bool retval = false;

            int prev_stack_top = match_stack.size();

            if (Keyword("fun")) {
                retval = true;

                if (Char('(')) {
                    Arg_List();
                    if (!Char(')')) {
                        throw Eval_Error("Incomplete anonymous function", File_Position(line, col), filename);
                    }
                }

                while (Eol());

                if (!Block()) {
                    throw Eval_Error("Incomplete anonymous function", File_Position(line, col), filename);
                }

                build_match(Token_Type::Lambda, prev_stack_top);
            }

            return retval;
        }

        /**
         * Reads a function definition from input
         */
        bool Def() {
            bool retval = false;
            bool is_annotated = false;
            bool is_method = false;
            TokenPtr annotation;

            if (Annotation()) {
                while (Eol_());
                annotation = match_stack.back();
                match_stack.pop_back();
                is_annotated = true;
            }

            int prev_stack_top = match_stack.size();

            if (Keyword("def")) {
                retval = true;

                if (!Id(true)) {
                    throw Eval_Error("Missing function name in definition", File_Position(line, col), filename);
                }

                if (Symbol("::", false)) {
                    //We're now a method
                    is_method = true;

                    if (!Id(true)) {
                        throw Eval_Error("Missing method name in definition", File_Position(line, col), filename);
                    }
                }

                if (Char('(')) {
                    Arg_List();
                    if (!Char(')')) {
                        throw Eval_Error("Incomplete function definition", File_Position(line, col), filename);
                    }
                }

                while (Eol());

                if (Char(':')) {
                    if (!Operator()) {
                        throw Eval_Error("Missing guard expression for function", File_Position(line, col), filename);
                    }
                }

                while (Eol());
                if (!Block()) {
                    throw Eval_Error("Incomplete function definition", File_Position(line, col), filename);
                }

                if (is_method) {
                    build_match(Token_Type::Method, prev_stack_top);
                }
                else {
                    build_match(Token_Type::Def, prev_stack_top);
                }

                if (is_annotated) {
                    match_stack.back()->annotation = annotation;
                }
            }

            return retval;
        }

        /**
         * Reads a function definition from input
         */
        bool Try() {
            bool retval = false;

            int prev_stack_top = match_stack.size();

            if (Keyword("try")) {
                retval = true;

                while (Eol());

                if (!Block()) {
                    throw Eval_Error("Incomplete 'try' block", File_Position(line, col), filename);
                }

                bool has_matches = true;
                while (has_matches) {
                    while (Eol());
                    has_matches = false;
                    if (Keyword("catch", false)) {
                        int catch_stack_top = match_stack.size();
                        if (Char('(')) {
                            if (!(Id(true) && Char(')'))) {
                                throw Eval_Error("Incomplete 'catch' expression", File_Position(line, col), filename);
                            }
                            if (Char(':')) {
                                if (!Operator()) {
                                    throw Eval_Error("Missing guard expression for catch", File_Position(line, col), filename);
                                }
                            }
                        }

                        while (Eol());

                        if (!Block()) {
                            throw Eval_Error("Incomplete 'catch' block", File_Position(line, col), filename);
                        }
                        build_match(Token_Type::Catch, catch_stack_top);
                        has_matches = true;
                    }
                }
                while (Eol());
                if (Keyword("finally", false)) {
                    int finally_stack_top = match_stack.size();

                    while (Eol());

                    if (!Block()) {
                        throw Eval_Error("Incomplete 'finally' block", File_Position(line, col), filename);
                    }
                    build_match(Token_Type::Finally, finally_stack_top);
                }

                build_match(Token_Type::Try, prev_stack_top);
            }

            return retval;
        }

        /**
         * Reads an if/elseif/else block from input
         */
        bool If() {
            bool retval = false;

            int prev_stack_top = match_stack.size();

            if (Keyword("if")) {
                retval = true;

                if (!Char('(')) {
                    throw Eval_Error("Incomplete 'if' expression", File_Position(line, col), filename);
                }

                if (!(Operator() && Char(')'))) {
                    throw Eval_Error("Incomplete 'if' expression", File_Position(line, col), filename);
                }

                while (Eol());

                if (!Block()) {
                    throw Eval_Error("Incomplete 'if' block", File_Position(line, col), filename);
                }

                bool has_matches = true;
                while (has_matches) {
                    while (Eol());
                    has_matches = false;
                    if (Keyword("else", true)) {
                        if (Keyword("if")) {
                            match_stack.back()->text = "else if";
                            if (!Char('(')) {
                                throw Eval_Error("Incomplete 'else if' expression", File_Position(line, col), filename);
                            }

                            if (!(Operator() && Char(')'))) {
                                throw Eval_Error("Incomplete 'else if' expression", File_Position(line, col), filename);
                            }

                            while (Eol());

                            if (!Block()) {
                                throw Eval_Error("Incomplete 'else if' block", File_Position(line, col), filename);
                            }
                            has_matches = true;
                        }
                        else {
                            while (Eol());

                            if (!Block()) {
                                throw Eval_Error("Incomplete 'else' block", File_Position(line, col), filename);
                            }
                            has_matches = true;
                        }
                    }
                }

                build_match(Token_Type::If, prev_stack_top);
            }

            return retval;
        }

        /**
         * Reads a while block from input
         */
        bool While() {
            bool retval = false;

            int prev_stack_top = match_stack.size();

            if (Keyword("while")) {
                retval = true;

                if (!Char('(')) {
                    throw Eval_Error("Incomplete 'while' expression", File_Position(line, col), filename);
                }

                if (!(Operator() && Char(')'))) {
                    throw Eval_Error("Incomplete 'while' expression", File_Position(line, col), filename);
                }

                while (Eol());

                if (!Block()) {
                    throw Eval_Error("Incomplete 'while' block", File_Position(line, col), filename);
                }

                build_match(Token_Type::While, prev_stack_top);
            }

            return retval;
        }

        /**
         * Reads the C-style for conditions from input
         */
        bool For_Guards() {
            Equation();

            if (Char(';') && Operator() && Char(';') && Equation()) {
                return true;
            }
            else {
                throw Eval_Error("Incomplete conditions in 'for' loop", File_Position(line, col), filename);
            }
        }

        /**
         * Reads a for block from input
         */
        bool For() {
            bool retval = false;

            int prev_stack_top = match_stack.size();

            if (Keyword("for")) {
                retval = true;

                if (!Char('(')) {
                    throw Eval_Error("Incomplete 'for' expression", File_Position(line, col), filename);
                }

                if (!(For_Guards() && Char(')'))) {
                    throw Eval_Error("Incomplete 'for' expression", File_Position(line, col), filename);
                }

                while (Eol());

                if (!Block()) {
                    throw Eval_Error("Incomplete 'for' block", File_Position(line, col), filename);
                }

                build_match(Token_Type::For, prev_stack_top);
            }

            return retval;
        }

        /**
         * Reads a curly-brace C-style block from input
         */
        bool Block() {
            bool retval = false;

            int prev_stack_top = match_stack.size();

            if (Char('{')) {
                retval = true;

                Statements();
                if (!Char('}')) {
                    throw Eval_Error("Incomplete block", File_Position(line, col), filename);
                }

                build_match(Token_Type::Block, prev_stack_top);
            }

            return retval;
        }

        /**
         * Reads a return statement from input
         */
        bool Return() {
            bool retval = false;

            int prev_stack_top = match_stack.size();

            if (Keyword("return")) {
                retval = true;

                Operator();
                build_match(Token_Type::Return, prev_stack_top);
            }

            return retval;
        }

        /**
         * Reads a break statement from input
         */
        bool Break() {
            bool retval = false;

            int prev_stack_top = match_stack.size();

            if (Keyword("break")) {
                retval = true;

                build_match(Token_Type::Break, prev_stack_top);
            }

            return retval;
        }

        /**
         * Reads an identifier, then proceeds to check if it's a function or array call
         */
        bool Id_Fun_Array() {
            bool retval = false;
            std::string::iterator prev_pos = input_pos;

            unsigned int prev_stack_top = match_stack.size();
            if (Id(true)) {
                retval = true;
                bool has_more = true;

                while (has_more) {
                    has_more = false;

                    if (Char('(')) {
                        has_more = true;

                        Arg_List();
                        if (!Char(')')) {
                            throw Eval_Error("Incomplete function call", File_Position(line, col), filename);
                        }

                        build_match(Token_Type::Fun_Call, prev_stack_top);
                    }
                    else if (Char('[')) {
                        has_more = true;

                        if (!(Operator() && Char(']'))) {
                            throw Eval_Error("Incomplete array access", File_Position(line, col), filename);
                        }

                        build_match(Token_Type::Array_Call, prev_stack_top);
                    }
                }
            }

            return retval;
        }

        /**
         * Reads a variable declaration from input
         */
        bool Var_Decl() {
            bool retval = false;

            int prev_stack_top = match_stack.size();

            if (Keyword("var")) {
                retval = true;

                if (!Id(true)) {
                    throw Eval_Error("Incomplete variable declaration", File_Position(line, col), filename);
                }

                build_match(Token_Type::Var_Decl, prev_stack_top);
            }
            else if (Keyword("attr")) {
                retval = true;

                if (!Id(true)) {
                    throw Eval_Error("Incomplete attribute declaration", File_Position(line, col), filename);
                }
                if (!Symbol("::", false)) {
                    throw Eval_Error("Incomplete attribute declaration", File_Position(line, col), filename);
                }
                if (!Id(true)) {
                    throw Eval_Error("Missing attribute name in definition", File_Position(line, col), filename);
                }


                build_match(Token_Type::Attr_Decl, prev_stack_top);
            }

            return retval;
        }

        /**
         * Reads an expression surrounded by parentheses from input
         */
        bool Paren_Expression() {
            bool retval = false;

            if (Char('(')) {
                retval = true;
                if (!Operator()) {
                    throw Eval_Error("Incomplete expression", File_Position(line, col), filename);
                }
                if (!Char(')')) {
                    throw Eval_Error("Missing closing parenthesis", File_Position(line, col), filename);
                }
            }
            return retval;
        }

        /**
         * Reads, and identifies, a short-form container initialization from input
         */
        bool Inline_Container() {
            bool retval = false;

            unsigned int prev_stack_top = match_stack.size();

            if (Char('[')) {
                retval = true;
                Container_Arg_List();
                if (!Char(']')) {
                    throw Eval_Error("Missing closing square bracket", File_Position(line, col), filename);
                }
                if ((prev_stack_top != match_stack.size()) && (match_stack.back()->children.size() > 0)) {
                    if (match_stack.back()->children[0]->identifier == Token_Type::Value_Range) {
                        build_match(Token_Type::Inline_Range, prev_stack_top);
                    }
                    else if (match_stack.back()->children[0]->identifier == Token_Type::Map_Pair) {
                        build_match(Token_Type::Inline_Map, prev_stack_top);
                    }
                    else {
                        build_match(Token_Type::Inline_Array, prev_stack_top);
                    }
                }
                else {
                    build_match(Token_Type::Inline_Array, prev_stack_top);
                }
            }

            return retval;
        }

        /**
         * Reads a unary prefixed expression from input
         */
        bool Prefix() {
            bool retval = false;

            int prev_stack_top = match_stack.size();

            if (Symbol("++", true)) {
                retval = true;

                if (!Operator(operators.size()-1)) {
                    throw Eval_Error("Incomplete '++' expression", File_Position(line, col), filename);
                }

                build_match(Token_Type::Prefix, prev_stack_top);
            }
            else if (Symbol("--", true)) {
                retval = true;

                if (!Operator(operators.size()-1)) {
                    throw Eval_Error("Incomplete '--' expression", File_Position(line, col), filename);
                }

                build_match(Token_Type::Prefix, prev_stack_top);
            }
            else if (Char('-', true)) {
                retval = true;

                if (!Operator(operators.size()-1)) {
                    throw Eval_Error("Incomplete unary '-' expression", File_Position(line, col), filename);
                }

                build_match(Token_Type::Prefix, prev_stack_top);
            }
            else if (Char('+', true)) {
                retval = true;

                if (!Operator(operators.size()-1)) {
                    throw Eval_Error("Incomplete unary '+' expression", File_Position(line, col), filename);
                }

                build_match(Token_Type::Prefix, prev_stack_top);
            }
            else if (Char('!', true)) {
                retval = true;

                if (!Operator(operators.size()-1)) {
                    throw Eval_Error("Incomplete '!' expression", File_Position(line, col), filename);
                }

                build_match(Token_Type::Prefix, prev_stack_top);
            }
            else if (Char('~', true)) {
                retval = true;

                if (!Operator(operators.size()-1)) {
                    throw Eval_Error("Incomplete '~' expression", File_Position(line, col), filename);
                }

                build_match(Token_Type::Prefix, prev_stack_top);
            }

            return retval;
        }

        /**
         * Parses any of a group of 'value' style token groups from input
         */
        bool Value() {
            if (Var_Decl() || Lambda() || Id_Fun_Array() || Num(true) || Prefix() || Quoted_String(true) || Single_Quoted_String(true) ||
                    Paren_Expression() || Inline_Container()) {
                return true;
            }
            else {
                return false;
            }
        }
        
        bool Operator_Helper(int precedence) {
            for (unsigned int i = 0; i < operator_matches[precedence].size(); ++i) {
                if (Symbol(operator_matches[precedence][i].c_str(), true)) {
                    return true;
                }
            }
            return false;
        }

        bool Operator(unsigned int precedence = 0) {
            bool retval = false;

            int prev_stack_top = match_stack.size();

            if (precedence < operators.size()) {
                if (Operator(precedence+1)) {
                    retval = true;
                    if (Operator_Helper(precedence)) {
                        do {
                            if (!Operator(precedence+1)) {
                                throw Eval_Error("Incomplete " + std::string(token_type_to_string(operators[precedence])) + " expression",
                                        File_Position(line, col), filename);
                            }
                        } while (Operator_Helper(precedence));

                        build_match(operators[precedence], prev_stack_top);
                    }
                }
            }
            else {
                return Value();
            }

            return retval;
        }

        /**
         * Reads a pair of values used to create a map initialization from input
         */
        bool Map_Pair() {
            bool retval = false;

            int prev_stack_top = match_stack.size();

            if (Operator()) {
                retval = true;
                if (Symbol(":")) {
                    do {
                        if (!Operator()) {
                            throw Eval_Error("Incomplete map pair", File_Position(line, col), filename);
                        }
                    } while (retval && Symbol(":"));

                    build_match(Token_Type::Map_Pair, prev_stack_top);
                }
            }

            return retval;
        }

        /**
         * Reads a pair of values used to create a range initialization from input
         */
        bool Value_Range() {
            bool retval = false;

            unsigned int prev_stack_top = match_stack.size();
            std::string::iterator prev_pos = input_pos;
            int prev_col = col;

            if (Operator()) {
                if (Symbol("..")) {
                    retval = true;
                    if (!Operator()) {
                        throw Eval_Error("Incomplete value range", File_Position(line, col), filename);
                    }

                    build_match(Token_Type::Value_Range, prev_stack_top);
                }
                else {
                    input_pos = prev_pos;
                    col = prev_col;
                    while (prev_stack_top != match_stack.size()) {
                        match_stack.pop_back();
                    }
                }
            }

            return retval;
        }

        /**
         * Parses a string of binary equation operators
         */
        bool Equation() {
            bool retval = false;

            int prev_stack_top = match_stack.size();

            if (Operator()) {
                retval = true;
                if (Symbol("=", true, true) || Symbol(":=", true, true) || Symbol("+=", true, true) ||
                        Symbol("-=", true, true) || Symbol("*=", true, true) || Symbol("/=", true, true) ||
                        Symbol("%=", true, true) || Symbol("<<=", true, true) || Symbol(">>=", true, true) ||
                        Symbol("&=", true, true) || Symbol("^=", true, true) || Symbol("|=", true, true)) {
                    if (!Equation()) {
                        throw Eval_Error("Incomplete equation", match_stack.back());
                    }

                    build_match(Token_Type::Equation, prev_stack_top);
                }
            }

            return retval;
        }

        /**
         * Top level parser, starts parsing of all known parses
         */
        bool Statements() {
            bool retval = false;

            bool has_more = true;
            bool saw_eol = true;

            while (has_more) {
                has_more = false;
                if (Def()) {
                    if (!saw_eol) {
                        throw Eval_Error("Two function definitions missing line separator", match_stack.back());
                    }
                    has_more = true;
                    retval = true;
                    saw_eol = true;
                }
                else if (Try()) {
                    if (!saw_eol) {
                        throw Eval_Error("Two function definitions missing line separator", match_stack.back());
                    }
                    has_more = true;
                    retval = true;
                    saw_eol = true;
                }
                else if (If()) {
                    if (!saw_eol) {
                        throw Eval_Error("Two function definitions missing line separator", match_stack.back());
                    }
                    has_more = true;
                    retval = true;
                    saw_eol = true;
                }
                else if (While()) {
                    if (!saw_eol) {
                        throw Eval_Error("Two function definitions missing line separator", match_stack.back());
                    }
                    has_more = true;
                    retval = true;
                    saw_eol = true;
                }
                else if (For()) {
                    if (!saw_eol) {
                        throw Eval_Error("Two function definitions missing line separator", match_stack.back());
                    }
                    has_more = true;
                    retval = true;
                    saw_eol = true;
                }
                else if (Return()) {
                    if (!saw_eol) {
                        throw Eval_Error("Two expressions missing line separator", match_stack.back());
                    }
                    has_more = true;
                    retval = true;
                    saw_eol = false;
                }
                else if (Break()) {
                    if (!saw_eol) {
                        throw Eval_Error("Two expressions missing line separator", match_stack.back());
                    }
                    has_more = true;
                    retval = true;
                    saw_eol = false;
                }
                else if (Equation()) {
                    if (!saw_eol) {
                        throw Eval_Error("Two expressions missing line separator", match_stack.back());
                    }
                    has_more = true;
                    retval = true;
                    saw_eol = false;
                }
                else if (Eol()) {
                    has_more = true;
                    retval = true;
                    saw_eol = true;
                }
                else if (Block()) {
                    has_more = true;
                    retval = true;
                    saw_eol = true;
                }
                else {
                    has_more = false;
                }
            }

            return retval;
        }

        /**
         * Parses the given input string, tagging parsed tokens with the given filename.
         */
        bool parse(std::string input, const char *fname) {
            input_pos = input.begin();
            input_end = input.end();
            line = 1; col = 1;
            filename = fname;

            if ((input.size() > 1) && (input[0] == '#') && (input[1] == '!')) {
                while ((input_pos != input_end) && (!Eol())) {
                    ++input_pos;
                }
            }

            if (Statements()) {
                if (input_pos != input_end) {
                    throw Eval_Error("Unparsed input", File_Position(line, col), fname);
                }
                else {
                    build_match(Token_Type::File, 0);
                    return true;
                }
            }
            else {
                return false;
            }
        }

    };
}


#endif /* CHAISCRIPT_PARSER_HPP_ */
