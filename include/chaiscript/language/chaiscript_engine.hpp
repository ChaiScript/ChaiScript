// This file is distributed under the BSD License.
// See "license.txt" for details.
// Copyright 2009-2010, Jonathan Turner (jturner@minnow-lang.org)
// and Jason Turner (lefticus@gmail.com)
// http://www.chaiscript.com

#ifndef CHAISCRIPT_ENGINE_HPP_
#define CHAISCRIPT_ENGINE_HPP_

#include <exception>
#include <fstream>

#include <chaiscript/language/chaiscript_common.hpp>

#ifdef _POSIX_VERSION
#include <dlfcn.h>
#else
#ifdef WIN32
#include <Windows.h>
#endif
#endif

#include <chaiscript/language/chaiscript_prelude.hpp>
#include <chaiscript/language/chaiscript_parser.hpp>

namespace chaiscript
{
    struct load_module_error : std::runtime_error
    {
       load_module_error(const std::string &reason) throw()
         : std::runtime_error(reason)
       {
       }

       virtual ~load_module_error() throw()
       {
       }
    };

#ifdef _POSIX_VERSION
    struct Loadable_Module
    {
        struct DLModule
        {
           DLModule(const std::string &t_filename)
             : m_data(dlopen(t_filename.c_str(), RTLD_NOW))
           {
              if (!m_data)
              {
                 throw load_module_error(dlerror());
              }
           }

           DLModule(const DLModule &); // Explicitly unimplemented copy constructor
           DLModule &operator=(const DLModule &); // Explicitly unimplemented assignment operator

           ~DLModule()
           {
              dlclose(m_data);
           }

           void *m_data;
        };

        template<typename T>
        struct DLSym
        {
            DLSym(DLModule &t_mod, const std::string &t_symbol)
              : m_symbol(reinterpret_cast<T>(dlsym(t_mod.m_data, t_symbol.c_str())))
            {
              if (!m_symbol)
              {
                 throw load_module_error(dlerror());
              }
            }

            T m_symbol;
        };

        Loadable_Module(const std::string &t_module_name, const std::string &t_filename)
          : m_dlmodule(t_filename), m_func(m_dlmodule, "create_chaiscript_module_" + t_module_name),
            m_moduleptr(m_func.m_symbol())
        {
        }

        DLModule m_dlmodule;
        DLSym<Create_Module_Func> m_func;
        ModulePtr m_moduleptr;
    };
#else

#ifdef WIN32


    struct Loadable_Module
    {
        template<typename T>
        static std::wstring towstring(const T &str) 
        {
            return std::wstring(str.begin(), str.end());
        }

        template<typename T>
        static std::string tostring(const T &str)
        {
            return std::string(str.begin(), str.end());
        }

#ifdef _UNICODE
        template<typename T>
        static std::wstring toproperstring(const T &str)
        {
            return towstring(str);
        }
#else
        template<typename T>
        static std::string toproperstring(const T &str)
        {
            return tostring(str);
        }
#endif

        static std::string GetErrorMessage(DWORD err)
        {
#ifdef _UNICODE
            typedef LPWSTR StringType;
            std::wstring retval = L"Unknown Error";
#else
            typedef LPSTR StringType;
            std::string retval = "Unknown Error";
#endif
            StringType lpMsgBuf = 0;

            FormatMessage(
                FORMAT_MESSAGE_ALLOCATE_BUFFER | 
                FORMAT_MESSAGE_FROM_SYSTEM |
                FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL,
                err,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                (StringType)&lpMsgBuf,
                0, NULL );        

            if (lpMsgBuf)
            {
              retval = lpMsgBuf;
            }

            LocalFree(lpMsgBuf);
            return tostring(retval);
        }

        struct DLModule
        {
           DLModule(const std::string &t_filename)
             : m_data(LoadLibrary(toproperstring(t_filename).c_str()))
           {
              if (!m_data)
              {
                 throw load_module_error(GetErrorMessage(GetLastError()));
              }
           }

           ~DLModule()
           {
              FreeLibrary(m_data);
           }

           HMODULE m_data;
        };

        template<typename T>
        struct DLSym
        {
            DLSym(DLModule &t_mod, const std::string &t_symbol)
              : m_symbol(reinterpret_cast<T>(GetProcAddress(t_mod.m_data, t_symbol.c_str())))
            {
              if (!m_symbol)
              {
                 throw load_module_error(GetErrorMessage(GetLastError()));
              }
            }

            T m_symbol;
        };

        Loadable_Module(const std::string &t_module_name, const std::string &t_filename)
          : m_dlmodule(t_filename), m_func(m_dlmodule, "create_chaiscript_module_" + t_module_name),
            m_moduleptr(m_func.m_symbol())
        {
        }

        DLModule m_dlmodule;
        DLSym<Create_Module_Func> m_func;
        ModulePtr m_moduleptr;
    };

#else
    struct Loadable_Module
    {
        Loadable_Module(const std::string &, const std::string &)
        {
          throw load_module_error("Loadable module support not available for your platform");
        }

        ModulePtr get()
        {
          throw load_module_error("Loadable module support not available for your platform");
        }
    };
#endif
#endif

    typedef boost::shared_ptr<Loadable_Module> Loadable_Module_Ptr;


    template <typename Eval_Engine>
    class ChaiScript_System {
#ifndef CHAISCRIPT_NO_THREADS
        mutable boost::shared_mutex mutex;
        mutable boost::recursive_mutex use_mutex;
#endif

        std::set<std::string> loaded_files;
        std::map<std::string, Loadable_Module_Ptr> loaded_modules;
        std::set<std::string> active_loaded_modules;

        std::vector<std::string> modulepaths;
        std::vector<std::string> usepaths;

        Eval_Engine engine;


        /**
         * Evaluates the given string in by parsing it and running the results through the evaluator
         */
        Boxed_Value do_eval(const std::string &input, const std::string &filename = "__EVAL__", bool internal = false) {
            ChaiScript_Parser parser;

            if (!internal)
            {
                engine.sync_cache();
            }

            //debug_print(tokens);
            Boxed_Value value;

            // Keep a cache of all loaded filenames and use the char * from this cache to pass 
            // to the parser. This is so that the parser does not have the overhead of passing 
            // around and copying strings
            // 
            if (filename != "__EVAL__")
            {
#ifndef CHAISCRIPT_NO_THREADS
                boost::unique_lock<boost::shared_mutex> l(mutex);
#endif
                loaded_files.insert(filename);
                try {
                    if (parser.parse(input, loaded_files.find(filename)->c_str())) {
#ifndef CHAISCRIPT_NO_THREADS
                        l.unlock();
#endif
                        //parser.show_match_stack();
                        value = eval_token<Eval_Engine>(engine, parser.ast());
                    }
                }
                catch (const Return_Value &rv) {
                    value = rv.retval;
                }
            } else {
                try {
#ifndef CHAISCRIPT_NO_THREADS
                    boost::shared_lock<boost::shared_mutex> l(mutex);
#endif
                    const char *fname = loaded_files.find("__EVAL__")->c_str();
#ifndef CHAISCRIPT_NO_THREADS
                    l.unlock();
#endif
 
                    if (parser.parse(input, fname)) {
                        //parser.show_match_stack();
                        value = eval_token<Eval_Engine>(engine, parser.ast());
                    }
                }
                catch (const Return_Value &rv) {
                    value = rv.retval;
                }
            }

            if (!internal)
            {
                engine.sync_cache();
            }

            return value;
        }

        /**
         * Evaluates the given boxed string, used during eval() inside of a script
         */
        const Boxed_Value internal_eval(const std::string &e) {
            return do_eval(e, "__EVAL__", true);
        }

        void use(const std::string &filename)
        {
          for (size_t i = 0; i < usepaths.size(); ++i)
          {

            try {

              const std::string appendedpath = usepaths[i] + filename;

#ifndef CHAISCRIPT_NO_THREADS
              boost::lock_guard<boost::recursive_mutex> l(use_mutex);
              boost::shared_lock<boost::shared_mutex> l2(mutex);
#endif

              if (loaded_files.count(appendedpath) == 0)
              {
#ifndef CHAISCRIPT_NO_THREADS
                l2.unlock();
#endif
                eval_file(appendedpath);
              } else {
                engine.sync_cache();
              }
            } catch (const File_Not_Found_Error &) {
              if (i == usepaths.size() - 1)
              {
                throw File_Not_Found_Error(filename);
              }

              // failed to load, try the next path
            }

          }
          
        }


    public:
        ChaiScript_System(const std::vector<std::string> &t_modulepaths = std::vector<std::string>(),
                          const std::vector<std::string> &t_usepaths = std::vector<std::string>())
          : modulepaths(t_modulepaths), usepaths(t_usepaths) {
            if (modulepaths.empty())
            {
              modulepaths.push_back("");
            }

            if (usepaths.empty())
            {
              usepaths.push_back("");
            }

            loaded_files.insert("__EVAL__"); // Make sure the default name is already registered
            build_eval_system();
        }

        /**
         * Adds a shared object, that can be used by all threads, to the system
         */
        ChaiScript_System &add_global_const(const Boxed_Value &bv, const std::string &name)
        {
            engine.add_global_const(bv, name);
            return *this;
        }

        struct State
        {
            std::set<std::string> loaded_files;
            typename Eval_Engine::State engine_state;
            std::set<std::string> active_loaded_modules;
        };

        /**
         * Returns a state object that represents the current
         * set of loaded files, the set of global variables and
         * the set of initialized functions
         */
        State get_state()
        {
#ifndef CHAISCRIPT_NO_THREADS
            boost::lock_guard<boost::recursive_mutex> l(use_mutex);
            boost::shared_lock<boost::shared_mutex> l2(mutex);
#endif

            State s;
            s.loaded_files = loaded_files;
            s.engine_state = engine.get_state();
            s.active_loaded_modules = active_loaded_modules;
            return s;
        }

        /**
         * Restores the state from a saved State object.
         */
        void set_state(const State &t_state)
        {
#ifndef CHAISCRIPT_NO_THREADS
            boost::lock_guard<boost::recursive_mutex> l(use_mutex);
            boost::shared_lock<boost::shared_mutex> l2(mutex);
#endif

            loaded_files = t_state.loaded_files;
            active_loaded_modules = t_state.active_loaded_modules;
            engine.set_state(t_state.engine_state);
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
            p->apply(*this, this->get_eval_engine());
            return *this;
        }

        /**
         * Load a dynamic library containing a chaiscript module
         */
        void load_module(const std::string &t_module_name)
        {
            std::vector<std::string> prefixes;
            prefixes.push_back("lib");
            prefixes.push_back("");

            std::vector<std::string> postfixes;
            postfixes.push_back(".dll");
            postfixes.push_back(".so");
            postfixes.push_back("");

            for (size_t i = 0; i < modulepaths.size(); ++i)
            {
              for (size_t j = 0; j < prefixes.size(); ++j)
              {
                for (size_t k = 0; k < postfixes.size(); ++k)
                {
                  try {
                    std::string name = modulepaths[i] + prefixes[j] + t_module_name + postfixes[k];
                    load_module(t_module_name, name);
                    return;
                  } catch (const load_module_error &) {
                    // Try next set
                  }
                }
              }
            }

            throw load_module_error("Unable to find module: " + t_module_name);
        }

        /**
         * Load a dynamic library and provide the file name to load it from
         */
        void load_module(const std::string &t_module_name, const std::string &t_filename)
        {
#ifndef CHAISCRIPT_NO_THREADS
            boost::lock_guard<boost::recursive_mutex> l(use_mutex);
#endif

            if (loaded_modules.count(t_module_name) == 0)
            {
                Loadable_Module_Ptr lm(new Loadable_Module(t_module_name, t_filename));
                loaded_modules[t_module_name] = lm;
                active_loaded_modules.insert(t_module_name);
                add(lm->m_moduleptr);
            } else if (active_loaded_modules.count(t_module_name) == 0) {
                active_loaded_modules.insert(t_module_name);
                add(loaded_modules[t_module_name]->m_moduleptr);
            } else {
                engine.sync_cache();
            }
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
        std::string load_file(const std::string &filename) {
            std::ifstream infile (filename.c_str(), std::ios::in | std::ios::ate);

            if (!infile.is_open()) {
                throw File_Not_Found_Error(filename);
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
            using namespace bootstrap;
            engine.add_reserved_word("def");
            engine.add_reserved_word("fun");
            engine.add_reserved_word("while");
            engine.add_reserved_word("for");
            engine.add_reserved_word("if");
            engine.add_reserved_word("else");
            engine.add_reserved_word("&&");
            engine.add_reserved_word("||");
            engine.add_reserved_word(",");
            engine.add_reserved_word(":=");
            engine.add_reserved_word("var");
            engine.add_reserved_word("return");
            engine.add_reserved_word("break");
            engine.add_reserved_word("true");
            engine.add_reserved_word("false");
            engine.add_reserved_word("_");

            add(Bootstrap::bootstrap());

            engine.add(fun(&Eval_Engine::dump_system, boost::ref(engine)), "dump_system");
            engine.add(fun(&Eval_Engine::dump_object, boost::ref(engine)), "dump_object");
            engine.add(fun(&Eval_Engine::is_type, boost::ref(engine)), "is_type");
            engine.add(fun(&Eval_Engine::type_name, boost::ref(engine)), "type_name");
            engine.add(fun(&Eval_Engine::function_exists, boost::ref(engine)), "function_exists");

            engine.add(fun(&Eval_Engine::get_type_name, boost::ref(engine)), "name");


            typedef void (ChaiScript_System<Eval_Engine>::*load_mod_1)(const std::string&);
            typedef void (ChaiScript_System<Eval_Engine>::*load_mod_2)(const std::string&, const std::string&);

            engine.add(fun(static_cast<load_mod_1>(&ChaiScript_System<Eval_Engine>::load_module), this), "load_module");
            engine.add(fun(static_cast<load_mod_2>(&ChaiScript_System<Eval_Engine>::load_module), this), "load_module");


            add(vector_type<std::vector<Boxed_Value> >("Vector"));
            add(string_type<std::string>("string"));
            add(map_type<std::map<std::string, Boxed_Value> >("Map"));
            add(pair_type<std::pair<Boxed_Value, Boxed_Value > >("Pair"));

            engine.add(fun(&ChaiScript_System<Eval_Engine>::use, this), "use");
            engine.add(fun(&ChaiScript_System<Eval_Engine>::internal_eval, this), "eval");


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
        Boxed_Value eval_file(const std::string &filename) {
            return do_eval(load_file(filename), filename);
        }

        /**
         * Loads the file specified by filename, evaluates it, and returns the as the specified type
         */
        template<typename T>
        T eval_file(const std::string &filename) {
            return boxed_cast<T>(do_eval(load_file(filename), filename));
        }
    };

    typedef ChaiScript_System<Dispatch_Engine> ChaiScript;
}
#endif /* CHAISCRIPT_ENGINE_HPP_ */

