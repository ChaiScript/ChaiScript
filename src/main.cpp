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

    chai.build_eval_system();

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
                catch (chaiscript::Parse_Error &pe) {
                    std::cout << pe.reason << " in " << pe.filename << " at " << pe.position.line << ", " << pe.position.column << std::endl;
                }
                catch (chaiscript::Eval_Error &ee) {
                    std::cout << ee.reason << std::endl;
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
            catch (chaiscript::Parse_Error &pe) {
                if (filename != std::string("__EVAL__")) {
                    std::cout << pe.reason << " in " << pe.filename << " at " << pe.position.line << ", " << pe.position.column << std::endl;
                }
                else {
                    std::cout << pe.reason << std::endl;
                }
            }
            catch (chaiscript::Eval_Error &ee) {
                if (filename != std::string("__EVAL__")) {
                    std::cout << ee.reason << " in '" << ee.location->filename << "' at " << ee.location->start.line  << ", " << ee.location->start.column << std::endl;
                }
                else {
                    std::cout << ee.reason << std::endl;
                }
            }
            catch (std::exception &e) {
                std::cout << e.what() << std::endl;
            }
        }
    }
}

