// This file is distributed under the BSD License.
// See LICENSE.TXT for details.

#ifndef CHAISCRIPT_ENGINE_HPP_
#define CHAISCRIPT_ENGINE_HPP_

#include <exception>
#include <fstream>


#include "chaiscript_prelude.hpp"
#include "chaiscript_parser.hpp"

namespace chaiscript
{
    template <typename Eval_Engine>
    class ChaiScript_System {
        Eval_Engine engine;
        ChaiScript_Parser parser;

    public:
        ChaiScript_System()  {

        }

        /**
         * Returns the current evaluation engine
         */
        Eval_Engine &get_eval_engine() {
            return engine;
        }

        /**
         * Prints the contents of an AST node, including its children, recursively
         */
        void debug_print(TokenPtr t, std::string prepend = "") {
            std::cout << prepend << "(" << token_type_to_string(t->identifier) << ") " << t->text << " : " << t->start.line << ", " << t->start.column << std::endl;
            for (unsigned int j = 0; j < t->children.size(); ++j) {
                debug_print(t->children[j], prepend + "  ");
            }
        }

        /**
         * Evaluates the given boxed string, used during eval() inside of a script
         */
        const dispatchkit::Boxed_Value eval(const std::vector<dispatchkit::Boxed_Value> &vals) {
            std::string val;
            val = dispatchkit::boxed_cast<std::string &>(vals[0]);

            return evaluate_string(val);
        }

        /**
         * Helper function for loading a file
         */
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

        /**
         * Builds all the requirements for ChaiScript, including its evaluator and a run of its prelude.
         */
        void build_eval_system() {
            dispatchkit::Bootstrap::bootstrap(engine);
            dispatchkit::bootstrap_vector<std::vector<dispatchkit::Boxed_Value> >(engine, "Vector");
            dispatchkit::bootstrap_string<std::string>(engine, "string");
            dispatchkit::bootstrap_map<std::map<std::string, dispatchkit::Boxed_Value> >(engine, "Map");
            dispatchkit::bootstrap_pair<std::pair<dispatchkit::Boxed_Value, dispatchkit::Boxed_Value > >(engine, "Pair");

            engine.register_function(boost::shared_ptr<dispatchkit::Proxy_Function>(
                  new dispatchkit::Dynamic_Proxy_Function(boost::bind(&ChaiScript_System<Eval_Engine>::eval, boost::ref(*this), _1), 1)), "eval");

            evaluate_string(chaiscript_prelude);
        }

        /**
         * Evaluates the given string in by parsing it and running the results through the evaluator
         */
        dispatchkit::Boxed_Value evaluate_string(const std::string &input, const char *filename = "__EVAL__") {
            //debug_print(tokens);
            dispatchkit::Boxed_Value value;
            parser.clear_match_stack();

            try {
                if (parser.parse(input, filename)) {
                    //parser.show_match_stack();
                    value = eval_token<Eval_Engine>(engine, parser.ast());
                }
            }
            catch (const Return_Value &rv) {
                value = rv.retval;
            }

            return value;
        }

        /**
         * Loads the file specified by filename, evaluates it, and returns the result
         */
        dispatchkit::Boxed_Value evaluate_file(const char *filename) {
            return evaluate_string(load_file(filename), filename);
        }
    };

    typedef ChaiScript_System<dispatchkit::Dispatch_Engine> ChaiScript_Engine;
}
#endif /* CHAISCRIPT_ENGINE_HPP_ */

