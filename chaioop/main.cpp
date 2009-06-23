#include <iostream>
#include <vector>

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

        bool Arg_List() {
            bool retval = Id() || Int();
            while (retval && Char(',')) {
                retval = Id() || Int();
            }
            return retval;
        }

        bool parse(std::string input) {
            input_pos = input.begin();
            input_end = input.end();

            return Id() && Char('(') && Arg_List() && Char(')');
        }
    };
};

int main() {
    langkit::Parser parser;

    std::cout << "Hello, world" << std::endl;

    std::cout << parser.parse("e(x,10)") << std::endl;

    return 0;
}
