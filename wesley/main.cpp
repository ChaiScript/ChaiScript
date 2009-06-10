/*
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>

#include <iostream>
#include <map>
#include <fstream>
*/

#include <iostream>

#include "wesley.hpp"

int main(int argc, char *argv[]) {
    std::string input;

    Wesley_Engine we;

    if (argc < 2) {
        std::cout << "eval> ";
        std::getline(std::cin, input);
        while (input != "quit") {
            Boxed_Value val;
            try {
                val = we.evaluate_string(input);
            }
            catch (const ReturnValue &rv) {
                val = rv.retval;
            }
            if (val.get_type_info().m_bare_type_info && *(val.get_type_info().m_bare_type_info) != typeid(void)) {
                try {
                    Boxed_Value printeval = dispatch(we.get_eval_engine().get_function("to_string"), Param_List_Builder() << val);
                    std::cout << "result: ";
                    dispatch(we.get_eval_engine().get_function("print"), Param_List_Builder() << printeval);
                } catch (const std::runtime_error &e) {
                    //std::cout << "unhandled type: " << val.get_type_info().m_type_info->name() << std::endl;
                }
            }
            std::cout << "eval> ";
            std::getline(std::cin, input);
        }
    }
    else {
        for (int i = 1; i < argc; ++i) {
            //Boxed_Value val = we.evaluate_string(we.load_file(argv[i]), argv[i]);
            try {
                Boxed_Value val = we.evaluate_file(argv[i]);
            }
            catch (std::exception &e) {
                std::cerr << "Could not open: " << argv[i] << std::endl;
                exit(1);
            }
        }
    }
}

