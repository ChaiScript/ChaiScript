#include <iostream>

#include "chaiscript.hpp"

int main(int argc, char *argv[]) {
    std::string input;

    ChaiScript_Engine chai;

    if (argc < 2) {
        std::cout << "eval> ";
        std::getline(std::cin, input);
        while (input != "quit") {
            Boxed_Value val;

            val = chai.evaluate_string(input);

            if (val.get_type_info().m_bare_type_info && *(val.get_type_info().m_bare_type_info) != typeid(void)) {
                try {
                    Boxed_Value printeval = dispatch(chai.get_eval_engine().get_function("to_string"), Param_List_Builder() << val);
                    std::cout << "result: ";
                    dispatch(chai.get_eval_engine().get_function("print"), Param_List_Builder() << printeval);
                } catch (const std::runtime_error &e) {
                    std::cout << "result: object #" << &val << std::endl;
                }
            }
            std::cout << "eval> ";
            std::getline(std::cin, input);
        }
    }
    else {
        for (int i = 1; i < argc; ++i) {
            try {
                Boxed_Value val = chai.evaluate_file(argv[i]);
            }
            catch (std::exception &e) {
                std::cerr << "Could not open: " << argv[i] << std::endl;
                exit(1);
            }
        }
    }
}

