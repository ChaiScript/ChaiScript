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

        /**
         * Evaluates the given string in by parsing it and running the results through the evaluator
         */
        Boxed_Value do_eval(const std::string &input, const char *filename = "__EVAL__") {
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
         * Evaluates the given boxed string, used during eval() inside of a script
         */
        const Boxed_Value internal_eval(const std::vector<Boxed_Value> &vals) {
            return do_eval(boxed_cast<std::string>(vals.at(0)));
        }


    public:
        ChaiScript_System()  {
            build_eval_system();
        }

        /**
         * Adds an object to the system: type, function, object
         */
        template<typename T>
          ChaiScript_System &add(const T &t, const std::string &name)
          {
              engine.add(t, name);
              return *this;
          }

        /**
         * Adds a module object to the system
         */
        ChaiScript_System &add(const ModulePtr &p)
        {
            engine.add(p);

            return *this;
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
              return chaiscript::functor<FunctionType>(eval(script));
          }

        /**
         * Evaluate a string via eval method
         */
        Boxed_Value operator()(const std::string &script)
        {
            return do_eval(script);
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
            engine.add(Bootstrap::bootstrap());

            engine.add(fun(boost::function<void ()>(boost::bind(&dump_system, boost::ref(engine)))), "dump_system");
            engine.add(fun(boost::function<void (Boxed_Value)>(boost::bind(&dump_object, _1, boost::ref(engine)))), "dump_object");
            engine.add(fun(boost::function<bool (Boxed_Value, const std::string &)>(boost::bind(&is_type, boost::ref(engine), _2, _1))),
                "is_type");


            engine.add(vector_type<std::vector<Boxed_Value> >("Vector"));
            engine.add(string_type<std::string>("string"));
            engine.add(map_type<std::map<std::string, Boxed_Value> >("Map"));
            engine.add(pair_type<std::pair<Boxed_Value, Boxed_Value > >("Pair"));

            engine.add(Proxy_Function(
                  new Dynamic_Proxy_Function(boost::bind(&ChaiScript_System<Eval_Engine>::internal_eval, boost::ref(*this), _1), 1)), "eval");

            do_eval(chaiscript_prelude, "standard prelude");
        }

        template<typename T>
          T eval(const std::string &input)
          {
              return boxed_cast<T>(do_eval(input));
          }

        Boxed_Value eval(const std::string &input)
        {
            return do_eval(input);
        }

        /**
         * Loads the file specified by filename, evaluates it, and returns the result
         */
        Boxed_Value eval_file(const char *filename) {
            return do_eval(load_file(filename), filename);
        }

        /**
         * Loads the file specified by filename, evaluates it, and returns the as the specified type
         */
        template<typename T>
        T eval_file(const char *filename) {
            return boxed_cast<T>(do_eval(load_file(filename), filename));
        }
    };

    typedef ChaiScript_System<Dispatch_Engine> ChaiScript;
}
#endif /* CHAISCRIPT_ENGINE_HPP_ */

