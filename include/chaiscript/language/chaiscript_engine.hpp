// This file is distributed under the BSD License.
// See "license.txt" for details.
// Copyright 2009, Jonathan Turner (jturner@minnow-lang.org)
// and Jason Turner (lefticus@gmail.com)
// http://www.chaiscript.com

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
            build_eval_system();
        }

        /**
         * Adds an object to the system: type, function, object
         */
        template<typename T>
          void add(const T &t, const std::string &name)
          {
              engine.add(t, name);
          }
        /**
         * Helper for calling script code as if it were native C++ code
         * example:
         * boost::function<int (int, int)> f = build_functor(chai, "func(x, y){x+y}");
         * \return a boost::function representing the passed in script
         * \param[in] script Script code to build a function from
         */
        template<typename FunctionType>
          boost::function<FunctionType> functor(const std::string &script)
          {
            return chaiscript::functor<FunctionType>(evaluate_string(script));
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
        const Boxed_Value eval(const std::vector<Boxed_Value> &vals) {
            std::string val;
            val = boxed_cast<std::string &>(vals[0]);

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
            Bootstrap::bootstrap(engine);
            bootstrap_vector<std::vector<Boxed_Value> >(engine, "Vector");
            bootstrap_string<std::string>(engine, "string");
            bootstrap_map<std::map<std::string, Boxed_Value> >(engine, "Map");
            bootstrap_pair<std::pair<Boxed_Value, Boxed_Value > >(engine, "Pair");

            engine.add(Proxy_Function(
                  new Dynamic_Proxy_Function(boost::bind(&ChaiScript_System<Eval_Engine>::eval, boost::ref(*this), _1), 1)), "eval");

            evaluate_string(chaiscript_prelude, "standard prelude");
        }

        /**
         * Evaluates the given string in by parsing it and running the results through the evaluator
         */
        Boxed_Value evaluate_string(const std::string &input, const char *filename = "__EVAL__") {
            //debug_print(tokens);
            Boxed_Value value;
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
        Boxed_Value evaluate_file(const char *filename) {
            return evaluate_string(load_file(filename), filename);
        }
    };

    typedef ChaiScript_System<Dispatch_Engine> ChaiScript_Engine;
}
#endif /* CHAISCRIPT_ENGINE_HPP_ */

