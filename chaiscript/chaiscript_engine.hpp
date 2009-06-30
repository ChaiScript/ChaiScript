// This file is distributed under the BSD License.
// See LICENSE.TXT for details.

#ifndef CHAISCRIPT_ENGINE_HPP_
#define CHAISCRIPT_ENGINE_HPP_

#include <exception>
#include <fstream>


#include "chaiscript_prelude.hpp"

namespace chaiscript
{
    template <typename Eval_Engine>
    class ChaiScript_System {
        Eval_Engine engine;

        std::string::iterator input_pos, input_end;
        int line, col;
        std::string multiline_comment_begin, multiline_comment_end;
        std::string singleline_comment;
        const char *filename;
        std::vector<TokenPtr> match_stack;

    public:
        ChaiScript_System()  {

        }

        Eval_Engine &get_eval_engine() {
            return engine;
        }

        void debug_print(TokenPtr t, std::string prepend = "") {
            std::cout << prepend << "(" << token_type_to_string(t->identifier) << ") " << t->text << " : " << t->start.line << ", " << t->start.column << std::endl;
            for (unsigned int j = 0; j < t->children.size(); ++j) {
                debug_print(t->children[j], prepend + "  ");
            }
        }

        const dispatchkit::Boxed_Value eval(const std::vector<dispatchkit::Boxed_Value> &vals) {
            std::string val;

            try {
                val = dispatchkit::boxed_cast<std::string &>(vals[0]);
            }
            catch (EvalError &ee) {
                throw EvalError("Can not evaluate string: " + val + " reason: " + ee.reason, TokenPtr());
            }
            catch (std::exception &e) {
                throw EvalError("Can not evaluate string: " + val, TokenPtr());
            }
            return evaluate_string(val);
        }

        std::string load_file(const char *filename) {
            std::ifstream infile (filename, std::ios::in | std::ios::ate);

            if (!infile.is_open()) {
                std::string fname = filename;
                throw std::runtime_error("Can not open: " + fname);
            }

            std::streampos size = infile.tellg();
            infile.seekg(0, std::ios::beg);

            std::vector<char> v(size);
            infile.read(&v[0], size);

            std::string ret_val (v.empty() ? std::string() : std::string (v.begin(), v.end()).c_str());

            return ret_val;
        }

        void build_eval_system() {
            multiline_comment_begin = "/*";
            multiline_comment_end = "*/";
            singleline_comment = "//";

            dispatchkit::Bootstrap::bootstrap(engine);
            dispatchkit::bootstrap_vector<std::vector<dispatchkit::Boxed_Value> >(engine, "Vector");
            dispatchkit::bootstrap_map<std::map<std::string, dispatchkit::Boxed_Value> >(engine, "Map");
            dispatchkit::bootstrap_pair<std::pair<dispatchkit::Boxed_Value, dispatchkit::Boxed_Value > >(engine, "Pair");

            engine.register_function(boost::shared_ptr<dispatchkit::Proxy_Function>(
                  new dispatchkit::Dynamic_Proxy_Function(boost::bind(&ChaiScript_System<Eval_Engine>::eval, boost::ref(*this), _1), 1)), "eval");

            evaluate_string(chaiscript_prelude);
        }

        dispatchkit::Boxed_Value evaluate_string(const std::string &input, const char *filename = "__EVAL__") {
            //debug_print(tokens);
            dispatchkit::Boxed_Value value;

            try {
                clear_match_stack();
                if (parse(input, filename)) {
                    //show_match_stack();
                    value = eval_token<Eval_Engine>(engine, match_stack.front());
                    //clear_match_stack();
                }
            }
            catch (const ReturnValue &rv) {
                value = rv.retval;
            }
            catch (Parse_Error &pe) {
                std::cout << pe.reason << " in " << pe.filename << " at " << pe.position.line << ", " << pe.position.column << std::endl;
                clear_match_stack();
            }
            catch (EvalError &ee) {
                if (filename != std::string("__EVAL__")) {
                    std::cout << "Eval error: \"" << ee.reason << "\" in '" << ee.location->filename << "' line: " << ee.location->start.line+1 << std::endl;
                }
                else {
                    std::cout << "Eval error: \"" << ee.reason << "\"" << std::endl;
                }
            }
            catch (std::exception &e) {
                std::cout << "Exception: " << e.what() << std::endl;
            }

            return value;
        }

        dispatchkit::Boxed_Value evaluate_file(const char *filename) {
            return evaluate_string(load_file(filename), filename);
        }

        void show_match_stack() {
            for (unsigned int i = 0; i < match_stack.size(); ++i) {
                debug_print(match_stack[i]);
            }
        }

        void clear_match_stack() {
            match_stack.clear();
        }

        void build_match(Token_Type::Type match_type, int match_start) {
            //so we want to take everything to the right of this and make them children
            if (match_start != (int)match_stack.size()) {
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
                    if (Eol_()) {
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

        bool Int_() {
            bool retval = false;
            if ((input_pos != input_end) && (*input_pos >= '0') && (*input_pos <= '9')) {
                retval = true;
                while ((input_pos != input_end) && (*input_pos >= '0') && (*input_pos <= '9')) {
                    ++input_pos;
                    ++col;
                }
            }

            return retval;
        }

        bool Float_() {
            bool retval = false;
            if ((input_pos != input_end) && (*input_pos >= '0') && (*input_pos <= '9')) {
                while ((input_pos != input_end) && (*input_pos >= '0') && (*input_pos <= '9')) {
                    ++input_pos;
                    ++col;
                }
                if (*input_pos == '.') {
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

        bool Num(bool capture = false) {
            SkipWS();

            if (!capture) {
                return Float_();
            }
            else {
                std::string::iterator start = input_pos;
                int prev_col = col;
                int prev_line = line;
                if ((input_pos != input_end) && (*input_pos >= '0') && (*input_pos <= '9')) {
                    if (Float_()) {
                        std::string match(start, input_pos);
                        TokenPtr t(new Token(match, Token_Type::Float, filename, prev_line, prev_col, line, col));
                        match_stack.push_back(t);
                        return true;
                    }
                    else {
                        std::string match(start, input_pos);
                        TokenPtr t(new Token(match, Token_Type::Int, filename, prev_line, prev_col, line, col));
                        match_stack.push_back(t);
                        return true;
                    }
                }
                else {
                    return false;
                }
            }
        }

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

            return retval;
        }

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
                    std::string match(start, input_pos);
                    TokenPtr t(new Token(match, Token_Type::Id, filename, prev_line, prev_col, line, col));
                    match_stack.push_back(t);
                    return true;
                }
                else {
                    return false;
                }
            }
        }

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
                    throw Parse_Error("Unclosed quoted string", File_Position(line, col), filename);
                }
            }
            return retval;
        }

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
                                    default: throw Parse_Error("Unknown escaped sequence in string", File_Position(prev_line, prev_col), filename);
                                }
                            }
                            else {
                                match.push_back(*s);
                            }
                            is_escaped = false;
                        }
                    }
                    TokenPtr t(new Token(match, Token_Type::Quoted_String, filename, prev_line, prev_col, line, col));
                    match_stack.push_back(t);
                    return true;
                }
                else {
                    return false;
                }
            }
        }

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
                    throw Parse_Error("Unclosed single-quoted string", File_Position(line, col), filename);
                }
            }
            return retval;
        }

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
                                    default: throw Parse_Error("Unknown escaped sequence in string", File_Position(prev_line, prev_col), filename);
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


        bool Char_(char c) {
            bool retval = false;
            if ((input_pos != input_end) && (*input_pos == c)) {
                ++input_pos;
                ++col;
                retval = true;
            }

            return retval;
        }

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

        bool Symbol(const char *s, bool capture = false) {
            SkipWS();

            if (!capture) {
                std::string::iterator start = input_pos;
                int prev_col = col;
                int prev_line = line;
                bool retval = Symbol_(s);
                if (retval) {
                    //todo: fix this.  Hacky workaround for preventing substring matches
                    if ((input_pos != input_end) && ((*input_pos == '+') || (*input_pos == '-') || (*input_pos == '*') || (*input_pos == '/') || (*input_pos == '='))) {
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
                    if ((input_pos != input_end) && ((*input_pos == '+') || (*input_pos == '-') || (*input_pos == '*') || (*input_pos == '/') || (*input_pos == '='))) {
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

        bool Arg_List() {
            bool retval = false;

            int prev_stack_top = match_stack.size();

            if (Expression()) {
                retval = true;
                if (Char(',')) {
                    do {
                        if (!Expression()) {
                            throw Parse_Error("Unexpected value in parameter list", match_stack.back());
                        }
                    } while (retval && Char(','));
                }
                build_match(Token_Type::Arg_List, prev_stack_top);
            }

            return retval;

        }

        bool Lambda() {
            bool retval = false;

            int prev_stack_top = match_stack.size();

            if (Keyword("fun")) {
                retval = true;

                if (Char('(')) {
                    if (!(Arg_List() && Char(')'))) {
                        throw Parse_Error("Incomplete anonymous function", File_Position(line, col), filename);
                    }
                }

                while (Eol());

                if (!Block()) {
                    throw Parse_Error("Incomplete anonymous function", File_Position(line, col), filename);
                }

                build_match(Token_Type::Lambda, prev_stack_top);
            }

            return retval;
        }

        bool Def() {
            bool retval = false;

            int prev_stack_top = match_stack.size();

            if (Keyword("def")) {
                retval = true;

                if (!Id(true)) {
                    throw Parse_Error("Missing function name in definition", File_Position(line, col), filename);
                }

                if (Char('(')) {
                    if (!(Arg_List() && Char(')'))) {
                        throw Parse_Error("Incomplete function definition", File_Position(line, col), filename);
                    }
                }

                while (Eol());

                if (!Block()) {
                    throw Parse_Error("Incomplete function definition", File_Position(line, col), filename);
                }

                build_match(Token_Type::Def, prev_stack_top);
            }

            return retval;
        }

        bool If() {
            bool retval = false;

            int prev_stack_top = match_stack.size();

            if (Keyword("if")) {
                retval = true;

                if (!Char('(')) {
                    throw Parse_Error("Incomplete if expression", File_Position(line, col), filename);
                }

                if (!(Expression() && Char(')'))) {
                    throw Parse_Error("Incomplete if expression", File_Position(line, col), filename);
                }

                while (Eol());

                if (!Block()) {
                    throw Parse_Error("Incomplete if block", File_Position(line, col), filename);
                }

                build_match(Token_Type::If, prev_stack_top);
            }

            return retval;
        }

        bool While() {
            bool retval = false;

            int prev_stack_top = match_stack.size();

            if (Keyword("while")) {
                retval = true;

                if (!Char('(')) {
                    throw Parse_Error("Incomplete while expression", File_Position(line, col), filename);
                }

                if (!(Expression() && Char(')'))) {
                    throw Parse_Error("Incomplete while expression", File_Position(line, col), filename);
                }

                while (Eol());

                if (!Block()) {
                    throw Parse_Error("Incomplete while block", File_Position(line, col), filename);
                }

                build_match(Token_Type::While, prev_stack_top);
            }

            return retval;
        }

        bool Block() {
            bool retval = false;

            int prev_stack_top = match_stack.size();

            if (Char('{')) {
                retval = true;

                Statements();
                if (!Char('}')) {
                    throw Parse_Error("Incomplete block", File_Position(line, col), filename);
                }

                build_match(Token_Type::Block, prev_stack_top);
            }

            return retval;

        }

        bool Return() {
            bool retval = false;

            int prev_stack_top = match_stack.size();

            if (Keyword("return")) {
                retval = true;

                Expression();
                build_match(Token_Type::Return, prev_stack_top);
            }

            return retval;
        }

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
                            throw Parse_Error("Incomplete function call", File_Position(line, col), filename);
                        }

                        build_match(Token_Type::Fun_Call, prev_stack_top);
                    }
                    else if (Char('[')) {
                        has_more = true;

                        if (!(Expression() && Char(']'))) {
                            throw Parse_Error("Incomplete array access", File_Position(line, col), filename);
                        }

                        build_match(Token_Type::Array_Call, prev_stack_top);
                    }
                }
            }

            return retval;
        }

        /*
         * TODO: Look into if we need an explicit LHS for equations
        bool LHS() {
            if (Var_Decl() || Id()) {
                return true;
            }
            else {
                return false;
            }
        }
        */

        bool Var_Decl() {
            bool retval = false;

            int prev_stack_top = match_stack.size();

            if (Keyword("var")) {
                retval = true;

                if (!Id(true)) {
                    throw Parse_Error("Incomplete variable declaration", File_Position(line, col), filename);
                }

                build_match(Token_Type::Var_Decl, prev_stack_top);
            }

            return retval;

        }

        bool Paren_Expression() {
            bool retval = false;

            if (Char('(')) {
                retval = true;
                if (!Expression()) {
                    throw Parse_Error("Incomplete expression", File_Position(line, col), filename);
                }
                if (!Char(')')) {
                    throw Parse_Error("Missing closing parenthesis", File_Position(line, col), filename);
                }
            }
            return retval;
        }

        bool Inline_Container() {
            bool retval = false;

            unsigned int prev_stack_top = match_stack.size();

            if (Char('[')) {
                retval = true;
                Arg_List();
                if (!Char(']')) {
                    throw Parse_Error("Missing closing square bracket", File_Position(line, col), filename);
                }
                build_match(Token_Type::Inline_Array, prev_stack_top);
            }

            return retval;
        }

        bool Id_Literal() {
            bool retval = false;

            SkipWS();

            if ((input_pos != input_end) && (*input_pos == '`')) {
                retval = true;

                int prev_col = col;
                int prev_line = line;

                ++col;
                ++input_pos;

                std::string::iterator start = input_pos;

                while ((input_pos != input_end) && (*input_pos != '`')) {
                    if (Eol()) {
                        throw Parse_Error("Carriage return in identifier literal", File_Position(line, col), filename);
                    }
                    else {
                        ++input_pos;
                        ++col;
                    }
                }

                if (start == input_pos) {
                    throw Parse_Error("Missing contents of identifier literal", File_Position(line, col), filename);
                }
                else if (input_pos == input_end) {
                    throw Parse_Error("Incomplete identifier literal", File_Position(line, col), filename);
                }

                ++col;
                std::string match(start, input_pos);
                TokenPtr t(new Token(match, Token_Type::Id, filename, prev_line, prev_col, line, col));
                match_stack.push_back(t);
                ++input_pos;

            }

            return retval;
        }

        bool Value() {
            if (Var_Decl() || Lambda() || Id_Fun_Array() || Num(true) || Negate() || Not() || Quoted_String(true) || Single_Quoted_String(true) ||
                    Paren_Expression() || Inline_Container() || Id_Literal()) {
                return true;
            }
            else {
                return false;
            }
        }

        bool Negate() {
            bool retval = false;

            int prev_stack_top = match_stack.size();

            if (Symbol("-")) {
                retval = true;

                if (!Additive()) {
                    throw Parse_Error("Incomplete negation expression", File_Position(line, col), filename);
                }

                build_match(Token_Type::Negate, prev_stack_top);
            }

            return retval;

        }

        bool Not() {
            bool retval = false;

            int prev_stack_top = match_stack.size();

            if (Symbol("!")) {
                retval = true;

                if (!Expression()) {
                    throw Parse_Error("Incomplete '!' expression", File_Position(line, col), filename);
                }

                build_match(Token_Type::Not, prev_stack_top);
            }

            return retval;

        }

        bool Comparison() {
            bool retval = false;

            int prev_stack_top = match_stack.size();

            if (Additive()) {
                retval = true;
                if (Symbol(">=", true) || Symbol(">", true) || Symbol("<=", true) || Symbol("<", true) || Symbol("==", true) || Symbol("!=", true)) {
                    do {
                        if (!Additive()) {
                            throw Parse_Error("Incomplete comparison expression", File_Position(line, col), filename);
                        }
                    } while (retval && (Symbol(">=", true) || Symbol(">", true) || Symbol("<=", true) || Symbol("<", true) || Symbol("==", true) || Symbol("!=", true)));

                    build_match(Token_Type::Comparison, prev_stack_top);
                }
            }

            return retval;

        }

        bool Additive() {
            bool retval = false;

            int prev_stack_top = match_stack.size();

            if (Multiplicative()) {
                retval = true;
                if (Symbol("+", true) || Symbol("-", true)) {
                    do {
                        if (!Multiplicative()) {
                            throw Parse_Error("Incomplete math expression", File_Position(line, col), filename);
                        }
                    } while (retval && (Symbol("+", true) || Symbol("-", true)));

                    build_match(Token_Type::Additive, prev_stack_top);
                }
            }

            return retval;
        }

        bool Multiplicative() {
            bool retval = false;

            int prev_stack_top = match_stack.size();

            if (Dot_Access()) {
                retval = true;
                if (Symbol("*", true) || Symbol("/", true)) {
                    do {
                        if (!Dot_Access()) {
                            throw Parse_Error("Incomplete math expression", File_Position(line, col), filename);
                        }
                    } while (retval && (Symbol("*", true) || Symbol("/", true)));

                    build_match(Token_Type::Multiplicative, prev_stack_top);
                }
            }

            return retval;

        }

        bool Dot_Access() {
            bool retval = false;

            int prev_stack_top = match_stack.size();

            if (Value()) {
                retval = true;
                if (Symbol(".")) {
                    do {
                        if (!Value()) {
                            throw Parse_Error("Incomplete dot notation", File_Position(line, col), filename);
                        }
                    } while (retval && Symbol("."));

                    build_match(Token_Type::Dot_Access, prev_stack_top);
                }
            }

            return retval;
        }

        bool Expression() {
            bool retval = false;

            int prev_stack_top = match_stack.size();

            if (Comparison()) {
                retval = true;
                if (Symbol("&&", true) || Symbol("||", true)) {
                    do {
                        if (!Comparison()) {
                            throw Parse_Error("Incomplete  expression", File_Position(line, col), filename);
                        }
                    } while (retval && (Symbol("&&", true) || Symbol("||", true)));

                    build_match(Token_Type::Expression, prev_stack_top);
                }
            }

            return retval;
        }

        bool Equation() {
            bool retval = false;

            int prev_stack_top = match_stack.size();

            if (Expression()) {
                retval = true;
                if (Symbol("=", true) || Symbol("+=", true) || Symbol("-=", true) || Symbol("*=", true) || Symbol("/=", true)) {
                    if (!Equation()) {
                        throw Parse_Error("Incomplete equation", match_stack.back());
                    }

                    build_match(Token_Type::Equation, prev_stack_top);
                }
            }

            return retval;

        }

        bool Statement() {
            if (Return() || Equation()) {
                return true;
            }
            else {
                return false;
            }
        }

        bool Statements() {
            bool retval = false;

            bool has_more = true;
            bool saw_eol = true;

            while (has_more) {
                has_more = false;
                if (Def()) {
                    if (!saw_eol) {
                        throw Parse_Error("Two function definitions missing line separator", match_stack.back());
                    }
                    has_more = true;
                    retval = true;
                    saw_eol = false;
                }
                else if (If()) {
                    if (!saw_eol) {
                        throw Parse_Error("Two function definitions missing line separator", match_stack.back());
                    }
                    has_more = true;
                    retval = true;
                    saw_eol = false;
                }
                else if (While()) {
                    if (!saw_eol) {
                        throw Parse_Error("Two function definitions missing line separator", match_stack.back());
                    }
                    has_more = true;
                    retval = true;
                    saw_eol = false;
                }
                else if (Statement()) {
                    if (!saw_eol) {
                        throw Parse_Error("Two expressions missing line separator", match_stack.back());
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
                else {
                    has_more = false;
                }
            }

            return retval;
        }

        bool parse(std::string input, const char *fname) {
            input_pos = input.begin();
            input_end = input.end();
            line = 1; col = 1;
            filename = fname;

            if (Statements()) {
                if (input_pos != input_end) {
                    throw Parse_Error("Unparsed input", File_Position(line, col), fname);
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

    typedef ChaiScript_System<dispatchkit::Dispatch_Engine> ChaiScript_Engine;
}
#endif /* CHAISCRIPT_ENGINE_HPP_ */

