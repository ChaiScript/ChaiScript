#include <iostream>

#include "chaiscript.hpp"

dispatchkit::Boxed_Value evoke_fn(chaiscript::ChaiScript_Engine &chai, std::string name, dispatchkit::Param_List_Builder &plb) {
    dispatchkit::Dispatch_Engine ss = chai.get_eval_engine();
    dispatchkit::Dispatch_Engine::Stack prev_stack = ss.get_stack();
    dispatchkit::Dispatch_Engine::Stack new_stack;
    new_stack.push_back(dispatchkit::Dispatch_Engine::Scope());
    ss.set_stack(new_stack);
    /*
    try {
        dispatchkit::Boxed_Value retval = dispatchkit::dispatch(ss.get_function(name), plb);
        ss.set_stack(prev_stack);
        return retval;
    }
    catch(...) {
        ss.set_stack(prev_stack);
        std::cout << "Exception caught" << std::endl;
        throw;
    }
    */
    dispatchkit::Boxed_Value retval;
    try {

        //fn = ss.get_function(node->children[0]->text);
        ss.set_stack(new_stack);
        //retval = dispatch(fn, plb);
        //retval = dispatch
        //retval = (*dispatchkit::boxed_cast<boost::shared_ptr<dispatchkit::Proxy_Function> >(fn))(plb);
        retval = dispatchkit::dispatch(ss.get_function(name), plb);
        ss.set_stack(prev_stack);
    }
    catch(chaiscript::EvalError &ee) {
        ss.set_stack(prev_stack);
        //throw chaiscript::EvalError(ee.reason, node->children[0]);
        throw;
    }
    catch(const dispatchkit::dispatch_error &e){
        ss.set_stack(prev_stack);
        throw;
    }
    catch(chaiscript::ReturnValue &rv) {
        ss.set_stack(prev_stack);
        retval = rv.retval;
    }
    catch(...) {
        ss.set_stack(prev_stack);
        throw;
    }
    return retval;
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

            val = chai.evaluate_string(input);

            if (val.get_type_info().m_bare_type_info && *(val.get_type_info().m_bare_type_info) != typeid(void)) {

                try {
                    dispatchkit::Boxed_Value printeval = evoke_fn(chai, "to_string", dispatchkit::Param_List_Builder() << val);
                    std::cout << "result: ";
                    evoke_fn(chai, "print", dispatchkit::Param_List_Builder() << printeval);
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

