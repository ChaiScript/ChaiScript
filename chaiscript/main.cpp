#include <iostream>

#include "chaiscript.hpp"

int main(int argc, char *argv[]) {

    std::string input;

    chaiscript::ChaiScript_Engine chai;
    chai.build_eval_system();

    if (argc < 2) {
        std::cout << "eval> ";
        std::getline(std::cin, input);
        while (input != "quit") {
            dispatchkit::Boxed_Value val;

            val = chai.evaluate_string(input);

            if (val.get_type_info().m_bare_type_info && *(val.get_type_info().m_bare_type_info) != typeid(void)) {
                try {
                    dispatchkit::Boxed_Value printeval
                      = dispatchkit::dispatch(chai.get_eval_engine().get_function("to_string"),
                          dispatchkit::Param_List_Builder() << val);
                    std::cout << "result: ";
                    dispatchkit::dispatch(chai.get_eval_engine().get_function("print"),
                        dispatchkit::Param_List_Builder() << printeval);
                } catch (const std::runtime_error &e) {
                    std::cout << "result: object #" << &val << " Error: " << e.what() << std::endl;
                }
            }
            std::cout << "eval> ";
            std::getline(std::cin, input);
        }
    }
    else {
        for (int i = 1; i < argc; ++i) {
            try {
              dispatchkit::Boxed_Value val = chai.evaluate_file(argv[i]);
            }
            catch (std::exception &e) {
                std::cerr << "Could not open: " << argv[i] << std::endl;
                exit(1);
            }
        }
    }
}

