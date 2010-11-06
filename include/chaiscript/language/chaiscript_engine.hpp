// This file is distributed under the BSD License.
// See "license.txt" for details.
// Copyright 2009-2010, Jonathan Turner (jonathan@emptycrate.com)
// and Jason Turner (jason@emptycrate.com)
// http://www.chaiscript.com

#ifndef CHAISCRIPT_ENGINE_HPP_
#define CHAISCRIPT_ENGINE_HPP_

#include <exception>
#include <fstream>

#include <chaiscript/language/chaiscript_common.hpp>

#ifdef _POSIX_VERSION
#include <dlfcn.h>
#else
#ifdef BOOST_WINDOWS
#define VC_EXTRA_LEAN
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif
#endif

#include <chaiscript/language/chaiscript_prelude.hpp>
#include <chaiscript/language/chaiscript_parser.hpp>

namespace chaiscript
{
  struct load_module_error : std::runtime_error
  {
    load_module_error(const std::string &t_reason) throw()
      : std::runtime_error(t_reason)
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
    static std::wstring towstring(const T &t_str) 
    {
      return std::wstring(t_str.begin(), t_str.end());
    }

    template<typename T>
    static std::string tostring(const T &t_str)
    {
      return std::string(t_str.begin(), t_str.end());
    }

#ifdef _UNICODE
    template<typename T>
    static std::wstring toproperstring(const T &t_str)
    {
      return towstring(t_str);
    }
#else
    template<typename T>
    static std::string toproperstring(const T &t_str)
    {
      return tostring(t_str);
    }
#endif

    static std::string GetErrorMessage(DWORD t_err)
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
          t_err,
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

  class ChaiScript {
#ifndef CHAISCRIPT_NO_THREADS
    mutable boost::shared_mutex m_mutex;
    mutable boost::recursive_mutex m_use_mutex;
#endif

    std::set<std::string> m_used_files;
    std::map<std::string, Loadable_Module_Ptr> m_loaded_modules;
    std::set<std::string> m_active_loaded_modules;

    std::vector<std::string> m_modulepaths;
    std::vector<std::string> m_usepaths;

    Dispatch_Engine m_engine;


    /**
     * Evaluates the given string in by parsing it and running the results through the evaluator
     */
    Boxed_Value do_eval(const std::string &t_input, const std::string &t_filename = "__EVAL__", bool /* t_internal*/  = false) 
    {
      try {
        ChaiScript_Parser parser;
        if (parser.parse(t_input, t_filename)) {
          //parser.show_match_stack();
          return parser.ast()->eval(m_engine);
        } else {
          return Boxed_Value();
        }
      }
      catch (const Return_Value &rv) {
        return rv.retval;
      }
    }


    const Boxed_Value internal_eval_ast(const AST_NodePtr &t_ast)
    {
      return t_ast->eval(m_engine);
    }

    /**
     * Evaluates the given boxed string, used during eval() inside of a script
     */
    const Boxed_Value internal_eval(const std::string &t_e) {
      return do_eval(t_e, "__EVAL__", true);
    }

    void use(const std::string &t_filename)
    {
      for (size_t i = 0; i < m_usepaths.size(); ++i)
      {

        try {
          const std::string appendedpath = m_usepaths[i] + t_filename;

#ifndef CHAISCRIPT_NO_THREADS
          boost::lock_guard<boost::recursive_mutex> l(m_use_mutex);
          boost::shared_lock<boost::shared_mutex> l2(m_mutex);
#endif

          if (m_used_files.count(appendedpath) == 0)
          {
#ifndef CHAISCRIPT_NO_THREADS
            m_used_files.insert(appendedpath);
            l2.unlock();
#endif
            eval_file(appendedpath);
          }
        } catch (const File_Not_Found_Error &) {
          if (i == m_usepaths.size() - 1)
          {
            throw File_Not_Found_Error(t_filename);
          }

          // failed to load, try the next path
        }
      }
    }


  public:
    ChaiScript(const std::vector<std::string> &t_modulepaths = std::vector<std::string>(),
                      const std::vector<std::string> &t_usepaths = std::vector<std::string>())
      : m_modulepaths(t_modulepaths), m_usepaths(t_usepaths) 
    {
      if (m_modulepaths.empty())
      {
        m_modulepaths.push_back("");
      }

      if (m_usepaths.empty())
      {
        m_usepaths.push_back("");
      }

      build_eval_system();
    }

    /**
     * Adds a shared object, that can be used by all threads, to the system
     */
    ChaiScript &add_global_const(const Boxed_Value &t_bv, const std::string &t_name)
    {
      m_engine.add_global_const(t_bv, t_name);
      return *this;
    }

    struct State
    {
      std::set<std::string> used_files;
      Dispatch_Engine::State engine_state;
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
      boost::lock_guard<boost::recursive_mutex> l(m_use_mutex);
      boost::shared_lock<boost::shared_mutex> l2(m_mutex);
#endif

      State s;
      s.used_files = m_used_files;
      s.engine_state = m_engine.get_state();
      s.active_loaded_modules = m_active_loaded_modules;
      return s;
    }

    /**
     * Restores the state from a saved State object.
     */
    void set_state(const State &t_state)
    {
#ifndef CHAISCRIPT_NO_THREADS
      boost::lock_guard<boost::recursive_mutex> l(m_use_mutex);
      boost::shared_lock<boost::shared_mutex> l2(m_mutex);
#endif

      m_used_files = t_state.used_files;
      m_active_loaded_modules = t_state.active_loaded_modules;
      m_engine.set_state(t_state.engine_state);
    }

    /**
     * Adds an object to the system: type, function, object
     */
    template<typename T>
    ChaiScript &add(const T &t_t, const std::string &t_name)
    {
      m_engine.add(t_t, t_name);
      return *this;
    }

    /**
     * Adds a module object to the system
     */
    ChaiScript &add(const ModulePtr &t_p)
    {
      t_p->apply(*this, this->get_eval_engine());
      return *this;
    }

    /**
     * Load a dynamic library containing a chaiscript module
     */
    void load_module(const std::string &t_module_name)
    {
      std::vector<load_module_error> errors;

      std::vector<std::string> prefixes;
      prefixes.push_back("lib");
      prefixes.push_back("");

      std::vector<std::string> postfixes;
      postfixes.push_back(".dll");
      postfixes.push_back(".so");
      postfixes.push_back("");

      for (size_t i = 0; i < m_modulepaths.size(); ++i)
        {
          for (size_t j = 0; j < prefixes.size(); ++j)
            {
              for (size_t k = 0; k < postfixes.size(); ++k)
                {
                  try {
                    std::string name = m_modulepaths[i] + prefixes[j] + t_module_name + postfixes[k];
                    load_module(t_module_name, name);
                    return;
                  } catch (const load_module_error &e) {
                    errors.push_back(e);
                    // Try next set
                  }
                }
            }
        }

      std::string errstring;

      for (std::vector<load_module_error>::const_iterator itr = errors.begin();
           itr != errors.end();
           ++itr)
      {
        if (!errstring.empty())
        {
          errstring += "; ";
        }

        errstring += itr->what();
      }

      throw load_module_error("Unable to find module: " + t_module_name + " Errors: " + errstring);
    }

    /**
     * Load a dynamic library and provide the file name to load it from
     */
    void load_module(const std::string &t_module_name, const std::string &t_filename)
    {
#ifndef CHAISCRIPT_NO_THREADS
      boost::lock_guard<boost::recursive_mutex> l(m_use_mutex);
#endif

      if (m_loaded_modules.count(t_module_name) == 0)
      {
        Loadable_Module_Ptr lm(new Loadable_Module(t_module_name, t_filename));
        m_loaded_modules[t_module_name] = lm;
        m_active_loaded_modules.insert(t_module_name);
        add(lm->m_moduleptr);
      } else if (m_active_loaded_modules.count(t_module_name) == 0) {
        m_active_loaded_modules.insert(t_module_name);
        add(m_loaded_modules[t_module_name]->m_moduleptr);
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
    boost::function<FunctionType> functor(const std::string &t_script)
    {
      return chaiscript::functor<FunctionType>(eval(t_script));
    }

    /**
     * Evaluate a string via eval method
     */
    Boxed_Value operator()(const std::string &t_script)
    {
      return do_eval(t_script);
    }


    /**
     * Returns the current evaluation m_engine
     */
    Dispatch_Engine &get_eval_engine() {
      return m_engine;
    }

    /**
     * Helper function for loading a file
     */
    std::string load_file(const std::string &t_filename) {
      std::ifstream infile(t_filename.c_str(), std::ios::in | std::ios::ate);

      if (!infile.is_open()) {
        throw File_Not_Found_Error(t_filename);
      }

      std::streampos size = infile.tellg();
      infile.seekg(0, std::ios::beg);

      assert(size >= 0);
      std::vector<char> v(static_cast<unsigned int>(size));
      infile.read(&v[0], size);

      return std::string(v.begin(), v.end());
    }

    /**
     * Builds all the requirements for ChaiScript, including its evaluator and a run of its prelude.
     */
    void build_eval_system() {
      using namespace bootstrap;
      m_engine.add_reserved_word("def");
      m_engine.add_reserved_word("fun");
      m_engine.add_reserved_word("while");
      m_engine.add_reserved_word("for");
      m_engine.add_reserved_word("if");
      m_engine.add_reserved_word("else");
      m_engine.add_reserved_word("&&");
      m_engine.add_reserved_word("||");
      m_engine.add_reserved_word(",");
      m_engine.add_reserved_word(":=");
      m_engine.add_reserved_word("var");
      m_engine.add_reserved_word("return");
      m_engine.add_reserved_word("break");
      m_engine.add_reserved_word("true");
      m_engine.add_reserved_word("false");
      m_engine.add_reserved_word("_");

      add(Bootstrap::bootstrap());

      m_engine.add(fun(&Dispatch_Engine::dump_system, boost::ref(m_engine)), "dump_system");
      m_engine.add(fun(&Dispatch_Engine::dump_object, boost::ref(m_engine)), "dump_object");
      m_engine.add(fun(&Dispatch_Engine::is_type, boost::ref(m_engine)), "is_type");
      m_engine.add(fun(&Dispatch_Engine::type_name, boost::ref(m_engine)), "type_name");
      m_engine.add(fun(&Dispatch_Engine::function_exists, boost::ref(m_engine)), "function_exists");

      m_engine.add(fun(&Dispatch_Engine::get_type_name, boost::ref(m_engine)), "name");


      typedef void (ChaiScript::*load_mod_1)(const std::string&);
      typedef void (ChaiScript::*load_mod_2)(const std::string&, const std::string&);

      m_engine.add(fun(static_cast<load_mod_1>(&ChaiScript::load_module), this), "load_module");
      m_engine.add(fun(static_cast<load_mod_2>(&ChaiScript::load_module), this), "load_module");

      add(vector_type<std::vector<Boxed_Value> >("Vector"));
      add(string_type<std::string>("string"));
      add(map_type<std::map<std::string, Boxed_Value> >("Map"));
      add(pair_type<std::pair<Boxed_Value, Boxed_Value > >("Pair"));

      m_engine.add(fun(&ChaiScript::use, this), "use");
      m_engine.add(fun(&ChaiScript::internal_eval, this), "eval");
      m_engine.add(fun(&ChaiScript::internal_eval_ast, this), "eval");

      do_eval(chaiscript_prelude, "standard prelude");
    }

    template<typename T>
    T eval(const std::string &t_input)
    {
      return boxed_cast<T>(do_eval(t_input));
    }

    Boxed_Value eval(const std::string &t_input)
    {
      return do_eval(t_input);
    }

    /**
     * Loads the file specified by filename, evaluates it, and returns the result
     */
    Boxed_Value eval_file(const std::string &t_filename) {
      return do_eval(load_file(t_filename), t_filename);
    }

    /**
     * Loads the file specified by filename, evaluates it, and returns the as the specified type
     */
    template<typename T>
    T eval_file(const std::string &t_filename) {
      return boxed_cast<T>(do_eval(load_file(t_filename), t_filename));
    }
  };

}
#endif /* CHAISCRIPT_ENGINE_HPP_ */

