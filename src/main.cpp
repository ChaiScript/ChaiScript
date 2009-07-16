// This file is distributed under the BSD License.
// See "license.txt" for details.
// Copyright 2009, Jonathan Turner (jturner@minnow-lang.org)
// and Jason Turner (lefticus@gmail.com)
// http://www.chaiscript.com

#include <iostream>

#include <chaiscript/chaiscript.hpp>

void print_help() {
    std::cout << "ChaiScript evaluator.  To evaluate and expression, type it and press <enter>." << std::endl;
    std::cout << "Additionally, you can inspect the runtime system using:" << std::endl;
    std::cout << "  dump_system() - outputs all functions registered to the system" << std::endl;
    std::cout << "  dump_object(x) - dumps information about the given symbol" << std::endl;
}

int main(int argc, char *argv[]) {
    std::string input;
    chaiscript::ChaiScript_Engine chai;

    if (argc < 2) {
        std::cout << "eval> ";
        std::getline(std::cin, input);
        while (input != "quit") {

            dispatchkit::Boxed_Value val;

            if (input == "help") {
                print_help();
            }
            else {
                try {
                    //First, we evaluate it
                    val = chai.evaluate_string(input);

                    //Then, we try to print the result of the evaluation to the user
                    if (val.get_type_info().m_bare_type_info && *(val.get_type_info().m_bare_type_info) != typeid(void)) {
                        try {
                            dispatchkit::dispatch(chai.get_eval_engine().get_function("print"), dispatchkit::Param_List_Builder() << val);
                        }
                        catch (...) {
                            //If we can't, do nothing
                        }
                    }
                }
                catch (std::exception &e) {
                    std::cout << e.what() << std::endl;
                }
            }

            std::cout << "eval> ";
            std::getline(std::cin, input);
        }
    }
    else {
        for (int i = 1; i < argc; ++i) {
            std::string filename(argv[i]);
            try {
              dispatchkit::Boxed_Value val = chai.evaluate_file(argv[i]);
            }
            catch (std::exception &e) {
                std::cout << e.what() << std::endl;
            }
        }
    }
}

