#include <iostream>
#include <vector>
#include <fstream>
#include <stdexcept>

#include <stdlib.h>
#include <string.h>

namespace langkit {
    struct File_Position {
        int line;
        int column;
        const char *filename;

        File_Position(int file_line, int file_column, const char *file_name)
            : line(file_line), column(file_column), filename(file_name) { }

        File_Position() : line(0), column(0), filename(NULL) { }
    };

    struct Parse_Error {
        std::string reason;
        File_Position position;

        Parse_Error(const std::string &why, const File_Position &where) :
            reason(why), position(where) { }

        virtual ~Parse_Error() throw() {}
    };

    class Token {
        std::string text;

        std::vector<Token> children;
    };

    class Parser {
        std::string::iterator input_pos, input_end;
        int line, col;
        std::string multiline_comment_begin, multiline_comment_end;
        std::string singleline_comment;

    public:
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

        bool Int() {
            SkipWS();

            return Int_();
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

        bool Id() {
            SkipWS();

            return Id_();
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

        bool Char(char c) {
            SkipWS();

            return Char_(c);
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

        bool Str(const char *s) {
            SkipWS();
            return Str_(s);
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

        bool Eol() {
            SkipWS();

            return Eol_();
        }

        bool Arg_List() {
            bool retval = Id() || Int();
            while (retval && Char(',')) {
                retval = Id() || Int();
                if (!retval) {
                    throw Parse_Error("Unexpected value in parameter list", File_Position(line, col, "_INPUT_"));
                }
            }
            return retval;
        }

        bool Fun_Call() {
            bool retval = false;

            std::string::iterator prev = input_pos;
            int prev_line = line;
            int prev_col = col;

            if (Id() && Char('(')) {

                Arg_List();
                retval = Char(')');
            }

            if (!retval) {
                input_pos = prev;
                prev_line = line;
                prev_col = col;
            }

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
        }
    }
    else {
        std::cout << "eval> ";
        std::getline(std::cin, input);
        while (input != "quit") {
            try {
                std::cout << parser.parse(input) << std::endl;
            }
            catch (langkit::Parse_Error &pe) {
                std::cout << pe.reason << " at " << pe.position.line << ", " << pe.position.column << std::endl;
            }
            std::cout << "eval> ";
            std::getline(std::cin, input);
        }
    }

    return 0;
}
