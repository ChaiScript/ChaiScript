// This file is distributed under the BSD License.
// See "license.txt" for details.
// Copyright 2009-2011, Jonathan Turner (jonathan@emptycrate.com)
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
#include "../dispatchkit/exception_specification.hpp"

namespace chaiscript
{
  namespace exception
  {
    /// \brief Thrown if an error occurs while attempting to load a binary module
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
  }

  namespace detail
  {
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
            throw exception::load_module_error(dlerror());
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
              throw exception::load_module_error(dlerror());
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
            throw exception::load_module_error(GetErrorMessage(GetLastError()));
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
              throw exception::load_module_error(GetErrorMessage(GetLastError()));
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
        throw exception::load_module_error("Loadable module support not available for your platform");
      }

      ModulePtr m_moduleptr;
    };
#endif
#endif

    typedef boost::shared_ptr<Loadable_Module> Loadable_Module_Ptr;
  }


  /// \brief The main object that the ChaiScript user will use.
  class ChaiScript {

    mutable chaiscript::detail::threading::shared_mutex m_mutex;
    mutable chaiscript::detail::threading::recursive_mutex m_use_mutex;

    std::set<std::string> m_used_files;
    std::map<std::string, detail::Loadable_Module_Ptr> m_loaded_modules;
    std::set<std::string> m_active_loaded_modules;

    std::vector<std::string> m_modulepaths;
    std::vector<std::string> m_usepaths;

    chaiscript::detail::Dispatch_Engine m_engine;


    /**
     * Evaluates the given string in by parsing it and running the results through the evaluator
     */
    Boxed_Value do_eval(const std::string &t_input, const std::string &t_filename = "__EVAL__", bool /* t_internal*/  = false) 
    {
      try {
        parser::ChaiScript_Parser parser;
        if (parser.parse(t_input, t_filename)) {
          //parser.show_match_stack();
          return parser.ast()->eval(m_engine);
        } else {
          return Boxed_Value();
        }
      }
      catch (const chaiscript::eval::detail::Return_Value &rv) {
        return rv.retval;
      }
    }



    const Boxed_Value internal_eval_ast(const AST_NodePtr &t_ast)
    {
      return t_ast->eval(m_engine);
    }


    /**
     * Evaluates the given string, used during eval() inside of a script
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

          chaiscript::detail::threading::lock_guard<chaiscript::detail::threading::recursive_mutex> l(m_use_mutex);
          chaiscript::detail::threading::shared_lock<chaiscript::detail::threading::shared_mutex> l2(m_mutex);

          if (m_used_files.count(appendedpath) == 0)
          {
            m_used_files.insert(appendedpath);
            l2.unlock();
            eval_file(appendedpath);
          }
        } catch (const exception::file_not_found_error &) {
          if (i == m_usepaths.size() - 1)
          {
            throw exception::file_not_found_error(t_filename);
          }

          // failed to load, try the next path
        }
      }
    }

    /**
     * Returns the current evaluation m_engine
     */
    chaiscript::detail::Dispatch_Engine &get_eval_engine() {
      return m_engine;
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

      m_engine.add(fun(&chaiscript::detail::Dispatch_Engine::dump_system, boost::ref(m_engine)), "dump_system");
      m_engine.add(fun(&chaiscript::detail::Dispatch_Engine::dump_object, boost::ref(m_engine)), "dump_object");
      m_engine.add(fun(&chaiscript::detail::Dispatch_Engine::is_type, boost::ref(m_engine)), "is_type");
      m_engine.add(fun(&chaiscript::detail::Dispatch_Engine::type_name, boost::ref(m_engine)), "type_name");
      m_engine.add(fun(&chaiscript::detail::Dispatch_Engine::function_exists, boost::ref(m_engine)), "function_exists");

      m_engine.add(fun(&chaiscript::detail::Dispatch_Engine::get_type_name, boost::ref(m_engine)), "name");


      typedef void (ChaiScript::*load_mod_1)(const std::string&);
      typedef void (ChaiScript::*load_mod_2)(const std::string&, const std::string&);

      m_engine.add(fun(static_cast<load_mod_1>(&ChaiScript::load_module), this), "load_module");
      m_engine.add(fun(static_cast<load_mod_2>(&ChaiScript::load_module), this), "load_module");

      add(standard_library::vector_type<std::vector<Boxed_Value> >("Vector"));
      add(standard_library::string_type<std::string>("string"));
      add(standard_library::map_type<std::map<std::string, Boxed_Value> >("Map"));
      add(standard_library::pair_type<std::pair<Boxed_Value, Boxed_Value > >("Pair"));

      m_engine.add(fun(&ChaiScript::use, this), "use");
      m_engine.add(fun(&ChaiScript::internal_eval, this), "eval");
      m_engine.add(fun(&ChaiScript::internal_eval_ast, this), "eval");

      do_eval(chaiscript_prelude, "standard prelude");
    }

    /**
     * Helper function for loading a file
     */
    std::string load_file(const std::string &t_filename) {
      std::ifstream infile(t_filename.c_str(), std::ios::in | std::ios::ate | std::ios::binary );

      if (!infile.is_open()) {
        throw exception::file_not_found_error(t_filename);
      }

      std::streampos size = infile.tellg();
      infile.seekg(0, std::ios::beg);

      assert(size >= 0);

      if (size == std::streampos(0))
      {
        return std::string();
      } else {
        std::vector<char> v(static_cast<unsigned int>(size));
        infile.read(&v[0], size);
        return std::string(v.begin(), v.end());
      }
    }

  public:
    /// \brief Constructor for ChaiScript
    /// \param[in] t_modulepaths Vector of paths to search when attempting to load a binary module
    /// \param[in] t_usepaths Vector of paths to search when attempting to "use" an included ChaiScript file
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

    /// \brief Adds a constant object that is available in all contexts and to all threads
    /// \param[in] t_bv Boxed_Value to add as a global
    /// \param[in] t_name Name of the value to add
    /// \throw exception::global_non_const If t_bv is not a constant object
    /// \sa Boxed_Value::is_const
    ChaiScript &add_global_const(const Boxed_Value &t_bv, const std::string &t_name)
    {
      m_engine.add_global_const(t_bv, t_name);
      return *this;
    }

    /// \brief Represents the current state of the ChaiScript system. State and be saved and restored
    /// \sa ChaiScript::get_state
    /// \sa ChaiScript::set_state
    struct State
    {
      std::set<std::string> used_files;
      chaiscript::detail::Dispatch_Engine::State engine_state;
      std::set<std::string> active_loaded_modules;
    };

    /// \brief Returns a state object that represents the current state of the system
    /// \return Current state of the system
    ///
    /// \b Example:
    ///
    /// \code
    /// chaiscript::ChaiScript chai;
    /// chaiscript::ChaiScript::State s = chai.get_state(); // represents bootstrapped initial state
    /// \endcode
    State get_state()
    {
      chaiscript::detail::threading::lock_guard<chaiscript::detail::threading::recursive_mutex> l(m_use_mutex);
      chaiscript::detail::threading::shared_lock<chaiscript::detail::threading::shared_mutex> l2(m_mutex);

      State s;
      s.used_files = m_used_files;
      s.engine_state = m_engine.get_state();
      s.active_loaded_modules = m_active_loaded_modules;
      return s;
    }

    /// \brief Sets the state of the system
    /// \param[in] t_state New state to set
    ///
    /// \b Example:
    /// \code
    /// chaiscript::ChaiScript chai;
    /// chaiscript::ChaiScript::State s = chai.get_state(); // get initial state
    /// chai.add(chaiscript::fun(&somefunction), "somefunction");
    /// chai.set_state(s); // restore initial state, which does not have the recently added "somefunction"
    /// \endcode
    void set_state(const State &t_state)
    {
      chaiscript::detail::threading::lock_guard<chaiscript::detail::threading::recursive_mutex> l(m_use_mutex);
      chaiscript::detail::threading::shared_lock<chaiscript::detail::threading::shared_mutex> l2(m_mutex);

      m_used_files = t_state.used_files;
      m_active_loaded_modules = t_state.active_loaded_modules;
      m_engine.set_state(t_state.engine_state);
    }

    /// \brief Adds a type, function or object to ChaiScript
    /// \param[in] t_t Item to add
    /// \param[in] t_name Name of item to add
    /// \returns Reference to current ChaiScript object
    /// 
    /// \b Examples:
    /// \code
    /// chaiscript::ChaiScript chai;
    /// chai.add(chaiscript::user_type<MyClass>(), "MyClass"); // Add explicit type info (not strictly necessary)
    /// chai.add(chaiscript::fun(&MyClass::function), "function"); // Add a class method
    /// MyClass obj;
    /// chai.add(chaiscript::var(&obj), "obj"); // Add a pointer to a locally defined object
    /// \endcode
    ///
    /// \sa \ref addingitems
    template<typename T>
    ChaiScript &add(const T &t_t, const std::string &t_name)
    {
      m_engine.add(t_t, t_name);
      return *this;
    }

    /// \brief Add a new conversion for upcasting to a base class
    /// \sa chaiscript::base_class
    /// \param[in] d Base class / parent class 
    ///
    /// \b Example:
    /// \code
    /// chaiscript::ChaiScript chai;
    /// chai.add(chaiscript::base_class<std::runtime_error, chaiscript::dispatch_error>());
    /// \endcode
    ChaiScript &add(const Dynamic_Cast_Conversion &d)
    {
      m_engine.add(d);
      return *this;
    }

    /// \brief Adds all elements of a module to ChaiScript runtime
    /// \param[in] t_p The module to add.
    /// \sa chaiscript::Module
    ChaiScript &add(const ModulePtr &t_p)
    {
      t_p->apply(*this, this->get_eval_engine());
      return *this;
    }

    /// \brief Load a binary module from a dynamic library. Works on platforms that support
    ///        dynamic libraries.
    /// \param[in] t_module_name Name of the module to load
    ///
    /// The module is searched for in the registered module path folders (chaiscript::ChaiScript::ChaiScript)
    /// and with standard prefixes and postfixes: ("lib"|"")\<t_module_name\>(".dll"|".so"|"").
    ///
    /// Once the file is located, the system looks for the symbol "create_chaiscript_module_\<t_module_name\>".
    /// If no file can be found matching the search criteria and containing the appropriate entry point 
    /// (the symbol mentioned above), an exception is thrown.
    ///
    /// \throw exception::load_module_error In the event that no matching module can be found.
    void load_module(const std::string &t_module_name)
    {
      std::vector<exception::load_module_error> errors;

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
                  } catch (const exception::load_module_error &e) {
                    errors.push_back(e);
                    // Try next set
                  }
                }
            }
        }

      std::string errstring;

      for (std::vector<exception::load_module_error>::const_iterator itr = errors.begin();
           itr != errors.end();
           ++itr)
      {
        if (!errstring.empty())
        {
          errstring += "; ";
        }

        errstring += itr->what();
      }

      throw exception::load_module_error("Unable to find module: " + t_module_name + " Errors: " + errstring);
    }

    /// \brief Load a binary module from a dynamic library. Works on platforms that support
    ///        dynamic libraries.
    ///
    /// \param[in] t_module_name Module name to load
    /// \param[in] t_filename Ignore normal filename search process and use specific filename
    ///
    /// \sa ChaiScript::load_module(const std::string &t_module_name)
    void load_module(const std::string &t_module_name, const std::string &t_filename)
    {
      chaiscript::detail::threading::lock_guard<chaiscript::detail::threading::recursive_mutex> l(m_use_mutex);

      if (m_loaded_modules.count(t_module_name) == 0)
      {
        detail::Loadable_Module_Ptr lm(new detail::Loadable_Module(t_module_name, t_filename));
        m_loaded_modules[t_module_name] = lm;
        m_active_loaded_modules.insert(t_module_name);
        add(lm->m_moduleptr);
      } else if (m_active_loaded_modules.count(t_module_name) == 0) {
        m_active_loaded_modules.insert(t_module_name);
        add(m_loaded_modules[t_module_name]->m_moduleptr);
      } 
    }


    /// \brief Evaluates a string. Equivalent to ChaiScript::eval.
    ///
    /// \param[in] t_script Script to execute
    /// \param[in] t_handler Optional Exception_Handler used for automatic unboxing of script thrown exceptions
    ///
    /// \return result of the script execution
    /// 
    /// \throw exception::eval_error In the case that evaluation fails.
    Boxed_Value operator()(const std::string &t_script, const Exception_Handler &t_handler = Exception_Handler())
    {
      try {
        return do_eval(t_script);
      } catch (Boxed_Value &bv) {
        if (t_handler) {
          t_handler->handle(bv);
        }
        throw bv;
      }
    }

    /// \brief Evaluates a string and returns a typesafe result.
    ///
    /// \tparam T Type to extract from the result value of the script execution
    /// \param[in] t_input Script to execute
    /// \param[in] t_handler Optional Exception_Handler used for automatic unboxing of script thrown exceptions
    ///
    /// \return result of the script execution
    /// 
    /// \throw exception::eval_error In the case that evaluation fails.
    /// \throw exception::bad_boxed_cast In the case that evaluation succeeds but the result value cannot be converted
    ///        to the requested type.
    template<typename T>
    T eval(const std::string &t_input, const Exception_Handler &t_handler = Exception_Handler())
    {
      try {
        return boxed_cast<T>(do_eval(t_input));
      } catch (Boxed_Value &bv) {
        if (t_handler) {
          t_handler->handle(bv);
        }
        throw bv;
      }
    }

    /// \brief Evaluates a string.
    ///
    /// \param[in] t_input Script to execute
    /// \param[in] t_handler Optional Exception_Handler used for automatic unboxing of script thrown exceptions
    ///
    /// \return result of the script execution
    /// 
    /// \throw exception::eval_error In the case that evaluation fails.
    Boxed_Value eval(const std::string &t_input, const Exception_Handler &t_handler = Exception_Handler())
    {
      try {
        return do_eval(t_input);
      } catch (Boxed_Value &bv) {
        if (t_handler) {
          t_handler->handle(bv);
        }
        throw bv;
      }
    }

    /// \brief Loads the file specified by filename, evaluates it, and returns the result.
    /// \param[in] t_filename File to load and parse.
    /// \param[in] t_handler Optional Exception_Handler used for automatic unboxing of script thrown exceptions
    /// \return result of the script execution
    /// \throw exception::eval_error In the case that evaluation fails.
    Boxed_Value eval_file(const std::string &t_filename, const Exception_Handler &t_handler = Exception_Handler()) {
      try {
        return do_eval(load_file(t_filename), t_filename);
      } catch (Boxed_Value &bv) {
        if (t_handler) {
          t_handler->handle(bv);
        }
        throw bv;
      }
    }

    /// \brief Loads the file specified by filename, evaluates it, and returns the typesafe result.
    /// \tparam T Type to extract from the result value of the script execution
    /// \param[in] t_filename File to load and parse.
    /// \param[in] t_handler Optional Exception_Handler used for automatic unboxing of script thrown exceptions
    /// \return result of the script execution
    /// \throw exception::eval_error In the case that evaluation fails.
    /// \throw exception::bad_boxed_cast In the case that evaluation succeeds but the result value cannot be converted
    ///        to the requested type.
    template<typename T>
    T eval_file(const std::string &t_filename, const Exception_Handler &t_handler = Exception_Handler()) {
      try {
        return boxed_cast<T>(do_eval(load_file(t_filename), t_filename));
      } catch (Boxed_Value &bv) {
        if (t_handler) {
          t_handler->handle(bv);
        }
        throw bv;
      }
    }
  };

}
#endif /* CHAISCRIPT_ENGINE_HPP_ */

