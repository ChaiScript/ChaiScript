// This file is distributed under the BSD License.
// See "license.txt" for details.
// Copyright 2009-2010, Jonathan Turner (jturner@minnow-lang.org)
// and Jason Turner (lefticus@gmail.com)
// http://www.chaiscript.com

#include <iostream>

#include <list>

#define _CRT_SECURE_NO_WARNINGS

#ifdef READLINE_AVAILABLE
#include <readline/readline.h>
#include <readline/history.h>
#endif

#include <chaiscript/chaiscript.hpp>


void print_help() {
    std::cout << "ChaiScript evaluator.  To evaluate an expression, type it and press <enter>." << std::endl;
    std::cout << "Additionally, you can inspect the runtime system using:" << std::endl;
    std::cout << "  dump_system() - outputs all functions registered to the system" << std::endl;
    std::cout << "  dump_object(x) - dumps information about the given symbol" << std::endl;
}


bool throws_exception(const chaiscript::Proxy_Function &f)
{
  try {
    chaiscript::functor<void ()>(f)();
  } catch (...) {
    return true;
  }

  return false;
}


std::string get_next_command() {
#ifdef READLINE_AVAILABLE
    char *input_raw;
    input_raw = readline("eval> ");
    add_history(input_raw);
    return std::string(input_raw);
#else
    std::string retval;
    std::cout << "eval> ";
    std::getline(std::cin, retval);
    return retval;
#endif
}

int main(int argc, char *argv[]) {
    std::string input;

    std::vector<std::string> usepaths;
    std::vector<std::string> modulepaths;

    const char *usepath = getenv("CHAI_USE_PATH");
    const char *modulepath = getenv("CHAI_MODULE_PATH");
   
    usepaths.push_back("");

    if (usepath)
    {
      usepaths.push_back(usepath);
    }

    modulepaths.push_back("");

    if (modulepath)
    {
      modulepaths.push_back(modulepath);
    }


    chaiscript::ChaiScript chai(modulepaths,usepaths);

    chai.add(chaiscript::fun(&exit), "exit");
    chai.add(chaiscript::fun(&throws_exception), "throws_exception");

    if (argc < 2) {
#ifdef READLINE_AVAILABLE
        using_history();
#endif
        input = get_next_command();
        while (input != "quit") {

            chaiscript::Boxed_Value val;

            if (input == "help") {
                print_help();
            }
            else {
                try {
                    //First, we evaluate it
                    val = chai.eval(input);

                    //Then, we try to print the result of the evaluation to the user
                    if (!val.get_type_info().bare_equal(chaiscript::user_type<void>())) {
                        try {
                            chaiscript::dispatch(chai.get_eval_engine().get_function("print"), chaiscript::Param_List_Builder() << val);
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

            input = get_next_command();
        }
    }
    else {
        for (int i = 1; i < argc; ++i) {
            try {
              chaiscript::Boxed_Value val = chai.eval_file(argv[i]);
            }
            catch (std::exception &e) {
                std::cout << e.what() << std::endl;
                return EXIT_FAILURE;
            }
        }
    }

    return EXIT_SUCCESS;
}

