#include <iostream>
#include <vector>
#include <fstream>
#include <stdexcept>
#include <tr1/memory>

#include <stdlib.h>
#include <string.h>

namespace langkit {
    struct File_Position {
        int line;
        int column;
        std::string::iterator text_pos;

        File_Position(int file_line, int file_column)
            : line(file_line), column(file_column) { }

        File_Position() : line(0), column(0) { }
    };

    struct Parse_Error {
        std::string reason;
        File_Position position;

        Parse_Error(const std::string &why, const File_Position &where) :
            reason(why), position(where) { }

        virtual ~Parse_Error() throw() {}
    };

    typedef std::tr1::shared_ptr<struct Token> TokenPtr;

    class Token_Type { public: enum Type { Internal_Match_Begin, Int, Id, Char, Str, Eol, Fun_Call, Arg_List }; };

    const char *token_type_to_string(int tokentype) {
        const char *token_types[] = { "Internal: match begin", "Int", "Id", "Char", "Str", "Eol", "Fun_Call", "Arg_List" };

        return token_types[tokentype];
    }

    struct Token {
        std::string text;
        int identifier;
        const char *filename;
        File_Position start, end;

        std::vector<TokenPtr> children;

        Token(const std::string &token_text, int id, const char *fname) : text(token_text), identifier(id), filename(fname) { }
    };

    void debug_print(TokenPtr t, std::string prepend = "") {
        std::cout << prepend << "text: " << t->text << " id: " << token_type_to_string(t->identifier) << " pos: " << t->start.line << ", " << t->start.column << std::endl;
        for (unsigned int j = 0; j < t->children.size(); ++j) {
            debug_print(t->children[j], prepend + "  ");
        }
    }

    class Parser {
        std::string::iterator input_pos, input_end;
        int line, col;
        std::string multiline_comment_begin, multiline_comment_end;
        std::string singleline_comment;
        char *filename;
        std::vector<TokenPtr> match_stack;

    public:
        void show_match_stack() {
            for (unsigned int i = 0; i < match_stack.size(); ++i) {
                debug_print(match_stack[i]);
            }
        }

        void clear_match_stack() {
            match_stack.clear();
        }

        bool Start_Parse() {
            TokenPtr t(new Token("", Token_Type::Internal_Match_Begin, filename));
            match_stack.push_back(t);
            t->start.line = line;
            t->start.column = col;
            t->start.text_pos = input_pos;

            return true;
        }

        void Fail_Parse() {
            TokenPtr t = match_stack.back(); match_stack.pop_back();
            while (t->identifier != Token_Type::Internal_Match_Begin) {
                t = match_stack.back(); match_stack.pop_back();
            }
            line = t->start.line;
            col = t->start.column;
            input_pos = t->start.text_pos;
        }

        bool Finish_Parse(int id) {
            for (int i = (int)match_stack.size() - 1; i >= 0; --i) {
                if (match_stack[i]->identifier == Token_Type::Internal_Match_Begin) {
                    //so we want to take everything to the right of this and make them children
                    match_stack[i]->children.insert(match_stack[i]->children.begin(), match_stack.begin() + (i+1), match_stack.end());
                    match_stack.erase(match_stack.begin() + (i+1), match_stack.end());
                    match_stack[i]->identifier = id;
                    return true;
                }
            }

            throw Parse_Error("Could not find internal begin parse marker", File_Position());
            return false;
        }

        bool SkipComment() {
            bool retval = false;

            if (Str_(multiline_comment_begin.c_str())) {
                while (input_pos != input_end) {
                    if (Str_(multiline_comment_end.c_str())) {
                        break;
                    }
                    else if (!Eol_()) {
                        ++col;
                        ++input_pos;
                    }
                }
                retval = true;
            }
            else if (Str_(singleline_comment.c_str())) {
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

        bool Int(bool capture = false) {
            SkipWS();

            if (!capture) {
                return Int_();
            }
            else {
                std::string::iterator start = input_pos;
                int prev_col = col;
                int prev_line = line;
                if (Int_()) {
                    std::string match(start, input_pos);
                    TokenPtr t(new Token(match, Token_Type::Int, filename));
                    t->start.column = prev_col;
                    t->start.line = prev_line;
                    t->end.column = col;
                    t->end.line = line;
                    match_stack.push_back(t);
                    return true;
                }
                else {
                    return false;
                }
            }
        }

        bool Id_() {
            bool retval = false;
            if ((input_pos != input_end) && (((*input_pos >= 'A') && (*input_pos <= 'Z')) || ((*input_pos >= 'a') && (*input_pos <= 'z')))) {
                retval = true;
                while ((input_pos != input_end) && (((*input_pos >= 'A') && (*input_pos <= 'Z')) || ((*input_pos >= 'a') && (*input_pos <= 'z')))) {
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
                    TokenPtr t(new Token(match, Token_Type::Id, filename));
                    t->start.column = prev_col;
                    t->start.line = prev_line;
                    t->end.column = col;
                    t->end.line = line;
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
                    TokenPtr t(new Token(match, Token_Type::Char, filename));
                    t->start.column = prev_col;
                    t->start.line = prev_line;
                    t->end.column = col;
                    t->end.line = line;
                    match_stack.push_back(t);
                    return true;
                }
                else {
                    return false;
                }
            }
        }

        bool Str_(const char *s) {
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

        bool Str(const char *s, bool capture = false) {
            SkipWS();

            if (!capture) {
                return Str_(s);
            }
            else {
                std::string::iterator start = input_pos;
                int prev_col = col;
                int prev_line = line;
                if (Str_(s)) {
                    std::string match(start, input_pos);
                    TokenPtr t(new Token(match, Token_Type::Str, filename));
                    t->start.column = prev_col;
                    t->start.line = prev_line;
                    t->end.column = col;
                    t->end.line = line;
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

            if ((input_pos != input_end) && (Str_("\r\n") || Char_('\n'))) {
                retval = true;
                ++line;
                col = 1;
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
                    TokenPtr t(new Token(match, Token_Type::Eol, filename));
                    t->start.column = prev_col;
                    t->start.line = prev_line;
                    t->end.column = col;
                    t->end.line = line;
                    match_stack.push_back(t);
                    return true;
                }
                else {
                    return false;
                }
            }
        }

        bool Arg_List() {
            bool retval;

            Start_Parse();

            retval = Id(true) || Int(true);
            while (retval && Char(',')) {
                retval = Id(true) || Int(true);
                if (!retval) {
                    throw Parse_Error("Unexpected value in parameter list", File_Position(line, col));
                }
            }

            if (retval) {
                Finish_Parse(Token_Type::Arg_List);
            }
            else {
                Fail_Parse();
            }
            return retval;
        }

        bool Fun_Call() {
            bool retval = false;

            Start_Parse();


            if (Id(true) && Char('(')) {

                Arg_List();
                retval = Char(')');
            }

            if (!retval) {
                Fail_Parse();
            }
            else {
                Finish_Parse(Token_Type::Fun_Call);
            }


            /*
             * The above can be shortened to this, but let's not get carried away :)
            if (!(Start_Parse() && Id(true) && Char('(') && (Arg_List() || true) && Char(')') && Finish_Parse(Token_Type::Fun_Call))) {
                Fail_Parse();
            }
            */

            return retval;
        }

        bool Fun_Calls() {
            bool retval = false;
            while ((Fun_Call() && Eol()) || (Eol())) { }

            if (input_pos == input_end) {
                retval = true;
            }

            return retval;
        }

        bool parse(std::string input) {
            input_pos = input.begin();
            input_end = input.end();
            line = 1; col = 1;
            multiline_comment_begin = "/*";
            multiline_comment_end = "*/";
            singleline_comment = "//";

            return Fun_Calls();
        }
    };
};

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

int main(int argc, char *argv[]) {
    langkit::Parser parser;
    std::string input;

    if (argc > 1) {
        try {
            std::cout << parser.parse(load_file(argv[1])) << std::endl;
        }
        catch (langkit::Parse_Error &pe) {
            std::cout << pe.reason << " at " << pe.position.line << ", " << pe.position.column << std::endl;
            parser.clear_match_stack();
        }
    }
    else {
        std::cout << "eval> ";
        std::getline(std::cin, input);
        while (input != "quit") {
            try {
                std::cout << parser.parse(input) << std::endl;
                parser.show_match_stack();
                parser.clear_match_stack();
            }
            catch (langkit::Parse_Error &pe) {
                std::cout << pe.reason << " at " << pe.position.line << ", " << pe.position.column << std::endl;
                parser.clear_match_stack();
            }
            std::cout << "eval> ";
            std::getline(std::cin, input);
        }
    }

    return 0;
}
