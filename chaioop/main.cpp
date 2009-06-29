#include <iostream>
#include <vector>
#include <fstream>
#include <stdexcept>
#include <tr1/memory>

#include <stdlib.h>
#include <string.h>

#include "chaiscript.hpp"

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
    chaiscript::Parser parser;
    std::string input;

    if (argc > 1) {
        try {
            std::cout << parser.parse(load_file(argv[1])) << std::endl;
            parser.show_match_stack();
        }
        catch (chaiscript::Parse_Error &pe) {
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
            catch (chaiscript::Parse_Error &pe) {
                std::cout << pe.reason << " at " << pe.position.line << ", " << pe.position.column << std::endl;
                parser.clear_match_stack();
            }
            std::cout << "eval> ";
            std::getline(std::cin, input);
        }
    }

    return 0;
}
