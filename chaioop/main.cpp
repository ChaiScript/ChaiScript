#include <iostream>
#include <vector>
#include <fstream>
#include <stdexcept>

#include <stdlib.h>
#include <string.h>

namespace langkit {
    class Token {
        std::string text;

        std::vector<Token> children;
    };

    class Parser {
        std::string::iterator input_pos, input_end;

    public:
        bool Int() {
            bool retval = false;
            if ((input_pos != input_end) && (*input_pos >= '0') && (*input_pos <= '9')) {
                retval = true;
                while ((input_pos != input_end) && (*input_pos >= '0') && (*input_pos <= '9')) {
                    ++input_pos;
                }
            }

            return retval;
        }

        bool Id() {
            bool retval = false;
            if ((input_pos != input_end) && (((*input_pos >= 'A') && (*input_pos <= 'Z')) || ((*input_pos >= 'a') && (*input_pos <= 'z')))) {
                retval = true;
                while ((input_pos != input_end) && (((*input_pos >= 'A') && (*input_pos <= 'Z')) || ((*input_pos >= 'a') && (*input_pos <= 'z')))) {
                    ++input_pos;
                }
            }

            return retval;
        }

        bool Char(char c) {
            bool retval = false;
            if ((input_pos != input_end) && (*input_pos == c)) {
                ++input_pos;
                retval = true;
            }

            return retval;
        }

        bool Str(const char *s) {
            bool retval = false;
            int len = strlen(s);

            if ((input_end - input_pos) >= len) {
                std::string::iterator tmp = input_pos;
                for (int i = 0; i < len; ++i) {
                    if (*tmp != s[i]) {
                        return false;
                    }
                }
                retval = true;
                input_pos = tmp;
            }

            return retval;
        }

        bool Arg_List() {
            bool retval = Id() || Int();
            while (retval && Char(',')) {
                retval = Id() || Int();
            }
            return retval;
        }

        bool Fun_Call() {
            bool retval = false;

            std::string::iterator prev = input_pos;

            if (Id() && Char('(')) {

                Arg_List();
                retval = Char(')');
            }

            if (!retval) { input_pos = prev; }

            return retval;
        }

        bool Eol() {
            bool retval = false;

            if ((input_pos != input_end) && (Str("\r\n") || Char('\n'))) {
                retval = true;
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

    if (argc > 1) {
        std::cout << parser.parse(load_file(argv[1])) << std::endl;
    }

    return 0;
}
