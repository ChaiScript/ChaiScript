// This file is distributed under the BSD License.
// See "license.txt" for details.
// Copyright 2009-2012, Jonathan Turner (jonathan@emptycrate.com)
// Copyright 2009-2016, Jason Turner (jason@emptycrate.com)
// http://www.chaiscript.com

#ifndef CHAISCRIPT_ENGINE_HPP_
#define CHAISCRIPT_ENGINE_HPP_

#include <cassert>
#include <exception>
#include <fstream>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <stdexcept>
#include <vector>

#include "../chaiscript_defines.hpp"
#include "../chaiscript_threading.hpp"
#include "../dispatchkit/boxed_cast_helper.hpp"
#include "../dispatchkit/boxed_value.hpp"
#include "../dispatchkit/dispatchkit.hpp"
#include "../dispatchkit/type_conversions.hpp"
#include "../dispatchkit/proxy_functions.hpp"
#include "chaiscript_common.hpp"

#if defined(__linux__) || defined(__unix__) || defined(__APPLE__) || defined(__HAIKU__)
#include <unistd.h>
#endif

#if defined(_POSIX_VERSION) && !defined(__CYGWIN__) 
#include <dlfcn.h>
#else
#ifdef CHAISCRIPT_WINDOWS
#define VC_EXTRA_LEAN
#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif
#endif


#include "../dispatchkit/exception_specification.hpp"
#include "chaiscript_parser.hpp"

namespace chaiscript
{
  namespace exception
  {
    /// \brief Thrown if an error occurs while attempting to load a binary module
    struct load_module_error : std::runtime_error
    {
      load_module_error(const std::string &t_reason) CHAISCRIPT_NOEXCEPT
        : std::runtime_error(t_reason)
      {
      }

      load_module_error(const load_module_error &) = default;
      virtual ~load_module_error() CHAISCRIPT_NOEXCEPT {}
    };
  }

  namespace detail
  {
#if defined(_POSIX_VERSION) && !defined(__CYGWIN__) 
    struct Loadable_Module
    {
      struct DLModule
      {
        DLModule(const std::string &t_filename)
          : m_data(dlopen(t_filename.c_str(), RTLD_NOW))
        {
          if (!m_data)
          {
            throw chaiscript::exception::load_module_error(dlerror());
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
            : m_symbol(cast_symbol(dlsym(t_mod.m_data, t_symbol.c_str())))
          {
            if (!m_symbol)
            {
              throw chaiscript::exception::load_module_error(dlerror());
            }
          }

          static T cast_symbol(void *p)
          {
            union cast_union
            {
              T func_ptr;
              void *in_ptr;
            };

            cast_union c;
            c.in_ptr = p;
            return c.func_ptr;
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
        static std::wstring to_wstring(const T &t_str) 
        {
          return std::wstring(t_str.begin(), t_str.end());
        }

      template<typename T>
        static std::string to_string(const T &t_str)
        {
          return std::string(t_str.begin(), t_str.end());
        }

#if defined(_UNICODE) || defined(UNICODE)
        template<typename T>
        static std::wstring to_proper_string(const T &t_str)
        {
          return to_wstring(t_str);
        }
#else
      template<typename T>
        static std::string to_proper_string(const T &t_str)
        {
          return to_string(t_str);
        }
#endif

      static std::string get_error_message(DWORD t_err)
      {
        typedef LPTSTR StringType;

#if defined(_UNICODE) || defined(UNICODE)
        std::wstring retval = L"Unknown Error";
#else
        std::string retval = "Unknown Error";
#endif
        StringType lpMsgBuf = nullptr;

        if (FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | 
            FORMAT_MESSAGE_FROM_SYSTEM |
            FORMAT_MESSAGE_IGNORE_INSERTS,
            nullptr,
            t_err,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            reinterpret_cast<StringType>(&lpMsgBuf),
            0, nullptr ) != 0 && lpMsgBuf)
        {
          retval = lpMsgBuf;
          LocalFree(lpMsgBuf);
        }

        return to_string(retval);
      }

      struct DLModule
      {
        DLModule(const std::string &t_filename)
          : m_data(LoadLibrary(to_proper_string(t_filename).c_str()))
        {
          if (!m_data)
          {
            throw chaiscript::exception::load_module_error(get_error_message(GetLastError()));
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
              throw chaiscript::exception::load_module_error(get_error_message(GetLastError()));
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
        throw chaiscript::exception::load_module_error("Loadable module support not available for your platform");
      }

      ModulePtr m_moduleptr;
    };
#endif
#endif

    typedef std::shared_ptr<Loadable_Module> Loadable_Module_Ptr;
  }


  /// \brief The main object that the ChaiScript user will use.
  class ChaiScript {

    mutable chaiscript::detail::threading::shared_mutex m_mutex;
    mutable chaiscript::detail::threading::recursive_mutex m_use_mutex;

    std::set<std::string> m_used_files;
    std::map<std::string, detail::Loadable_Module_Ptr> m_loaded_modules;
    std::set<std::string> m_active_loaded_modules;

    std::vector<std::string> m_module_paths;
    std::vector<std::string> m_use_paths;

    chaiscript::detail::Dispatch_Engine m_engine;

    /// Evaluates the given string in by parsing it and running the results through the evaluator
    Boxed_Value do_eval(const std::string &t_input, const std::string &t_filename = "__EVAL__", bool /* t_internal*/  = false) 
    {
      try {
        parser::ChaiScript_Parser parser;
        if (parser.parse(t_input, t_filename)) {
          //parser.show_match_stack();
          return parser.optimized_ast()->eval(m_engine);
        } else {
          return Boxed_Value();
        }
      }
      catch (chaiscript::eval::detail::Return_Value &rv) {
        return rv.retval;
      }
    }





    /// Evaluates the given file and looks in the 'use' paths
    const Boxed_Value internal_eval_file(const std::string &t_filename) {
      for (const auto &path : m_use_paths)
      {
        try {
          const auto appendedpath = path + t_filename;
          return do_eval(load_file(appendedpath), appendedpath, true);
        } catch (const exception::file_not_found_error &) {
          // failed to load, try the next path
        } catch (const exception::eval_error &t_ee) {
          throw Boxed_Value(t_ee);
        }
      }

      // failed to load by any name
      throw exception::file_not_found_error(t_filename);

    }



    /// Evaluates the given string, used during eval() inside of a script
    const Boxed_Value internal_eval(const std::string &t_e) {
      try {
        return do_eval(t_e, "__EVAL__", true);
      } catch (const exception::eval_error &t_ee) {
        throw Boxed_Value(t_ee);
      }
    }

    /// Returns the current evaluation m_engine
    chaiscript::detail::Dispatch_Engine &get_eval_engine() {
      return m_engine;
    }

    /// Builds all the requirements for ChaiScript, including its evaluator and a run of its prelude.
    void build_eval_system(const ModulePtr &t_lib) {
      m_engine.add_reserved_word("def");
      m_engine.add_reserved_word("fun");
      m_engine.add_reserved_word("while");
      m_engine.add_reserved_word("for");
      m_engine.add_reserved_word("if");
      m_engine.add_reserved_word("else");
      m_engine.add_reserved_word("&&");
      m_engine.add_reserved_word("||");
      m_engine.add_reserved_word(",");
      m_engine.add_reserved_word("auto");
      m_engine.add_reserved_word("return");
      m_engine.add_reserved_word("break");
      m_engine.add_reserved_word("true");
      m_engine.add_reserved_word("false");
      m_engine.add_reserved_word("class");
      m_engine.add_reserved_word("attr");
      m_engine.add_reserved_word("var");
      m_engine.add_reserved_word("global");
      m_engine.add_reserved_word("GLOBAL");
      m_engine.add_reserved_word("_");

      if (t_lib)
      {
        add(t_lib);
      }

      m_engine.add(fun([this](){ m_engine.dump_system(); }), "dump_system");
      m_engine.add(fun([this](const Boxed_Value &t_bv){ m_engine.dump_object(t_bv); }), "dump_object");
      m_engine.add(fun([this](const Boxed_Value &t_bv, const std::string &t_type){ return m_engine.is_type(t_bv, t_type); }), "is_type");
      m_engine.add(fun([this](const Boxed_Value &t_bv){ return m_engine.type_name(t_bv); }), "type_name");
      m_engine.add(fun([this](const std::string &t_f){ return m_engine.function_exists(t_f); }), "function_exists");
      m_engine.add(fun([this](){ return m_engine.get_function_objects(); }), "get_functions");
      m_engine.add(fun([this](){ return m_engine.get_scripting_objects(); }), "get_objects");

      m_engine.add(
          dispatch::make_dynamic_proxy_function(
              [this](const std::vector<Boxed_Value> &t_params) {
                return m_engine.call_exists(t_params);
              })
          , "call_exists");

//      m_engine.add(fun<Boxed_Value (const dispatch::Proxy_Function_Base *, const std::vector<Boxed_Value> &)>(std::bind(&chaiscript::dispatch::Proxy_Function_Base::operator(), std::placeholders::_1, std::placeholders::_2, std::ref(m_engine.conversions()))), "call");
//
//

      m_engine.add(fun(
            [=](const dispatch::Proxy_Function_Base &t_fun, const std::vector<Boxed_Value> &t_params) -> Boxed_Value {
              Type_Conversions_State s(this->m_engine.conversions(), this->m_engine.conversions().conversion_saves());
              return t_fun(t_params, s);
            }), "call");


      m_engine.add(fun([this](const Type_Info &t_ti){ return m_engine.get_type_name(t_ti); }), "name");

      m_engine.add(fun([this](const std::string &t_type_name, bool t_throw){ return m_engine.get_type(t_type_name, t_throw); }), "type");
      m_engine.add(fun([this](const std::string &t_type_name){ return m_engine.get_type(t_type_name, true); }), "type");

      m_engine.add(fun(
            [=](const Type_Info &t_from, const Type_Info &t_to, const std::function<Boxed_Value (const Boxed_Value &)> &t_func) {
              m_engine.add(chaiscript::type_conversion(t_from, t_to, t_func));
            }
          ), "add_type_conversion");



      m_engine.add(fun([this](const std::string &t_module, const std::string &t_file){ return load_module(t_module, t_file); }), "load_module");
      m_engine.add(fun([this](const std::string &t_module){ return load_module(t_module); }), "load_module");

      m_engine.add(fun([this](const std::string &t_file){ return use(t_file); }), "use");
      m_engine.add(fun([this](const std::string &t_file){ return internal_eval_file(t_file); }), "eval_file");
      m_engine.add(fun([this](const std::string &t_str){ return internal_eval(t_str); }), "eval");
      m_engine.add(fun([this](const AST_NodePtr &t_ast){ return eval(t_ast); }), "eval");
      m_engine.add(fun(&parse), "parse");

      m_engine.add(fun(&ChaiScript::version_major), "version_major");
      m_engine.add(fun(&ChaiScript::version_minor), "version_minor");
      m_engine.add(fun(&ChaiScript::version_patch), "version_patch");
      m_engine.add(fun(&ChaiScript::version), "version");
      m_engine.add(fun(&ChaiScript::compiler_version), "compiler_version");
      m_engine.add(fun(&ChaiScript::compiler_name), "compiler_name");
      m_engine.add(fun(&ChaiScript::compiler_id), "compiler_id");
      m_engine.add(fun(&ChaiScript::debug_build), "debug_build");

      m_engine.add(fun([this](const Boxed_Value &t_bv, const std::string &t_name){ add_global_const(t_bv, t_name); }), "add_global_const");
      m_engine.add(fun([this](const Boxed_Value &t_bv, const std::string &t_name){ add_global(t_bv, t_name); }), "add_global");
      m_engine.add(fun([this](const Boxed_Value &t_bv, const std::string &t_name){ set_global(t_bv, t_name); }), "set_global");
    }


    /// Helper function for loading a file
    static std::string load_file(const std::string &t_filename) {
      std::ifstream infile(t_filename.c_str(), std::ios::in | std::ios::ate | std::ios::binary );

      if (!infile.is_open()) {
        throw chaiscript::exception::file_not_found_error(t_filename);
      }

      const auto size = infile.tellg();
      infile.seekg(0, std::ios::beg);

      assert(size >= 0);

      if (size == std::streampos(0))
      {
        return std::string();
      } else {
        std::vector<char> v(static_cast<size_t>(size));
        infile.read(&v[0], size);
        return std::string(v.begin(), v.end());
      }
    }

  public:
    /// \brief Constructor for ChaiScript
    /// \param[in] t_lib Standard library to apply to this ChaiScript instance
    /// \param[in] t_modulepaths Vector of paths to search when attempting to load a binary module
    /// \param[in] t_usepaths Vector of paths to search when attempting to "use" an included ChaiScript file
    ChaiScript(const ModulePtr &t_lib,
               std::vector<std::string> t_modulepaths = std::vector<std::string>(),
                      std::vector<std::string> t_usepaths = std::vector<std::string>())
      : m_module_paths(std::move(t_modulepaths)), m_use_paths(std::move(t_usepaths))
    {
      if (m_module_paths.empty())
      {
        m_module_paths.push_back("");
      }

      if (m_use_paths.empty())
      {
        m_use_paths.push_back("");
      }

      build_eval_system(t_lib);
    }

    /// \brief Constructor for ChaiScript.
    /// 
    /// This version of the ChaiScript constructor attempts to find the stdlib module to load
    /// at runtime generates an error if it cannot be found.
    ///
    /// \param[in] t_modulepaths Vector of paths to search when attempting to load a binary module
    /// \param[in] t_usepaths Vector of paths to search when attempting to "use" an included ChaiScript file
    ChaiScript( std::vector<std::string> t_modulepaths = std::vector<std::string>(),
                      std::vector<std::string> t_usepaths = std::vector<std::string>())
      : m_module_paths(std::move(t_modulepaths)), m_use_paths(std::move(t_usepaths))
    {
      if (m_module_paths.empty())
      {
        m_module_paths.push_back("");
      }

      if (m_use_paths.empty())
      {
        m_use_paths.push_back("");
      }

#if defined(_POSIX_VERSION) && !defined(__CYGWIN__) 
      // If on Unix, add the path of the current executable to the module search path
      // as windows would do

      union cast_union
      {
        Boxed_Value (ChaiScript::*in_ptr)(const std::string&);
        void *out_ptr;
      };

      Dl_info rInfo; 
      memset( &rInfo, 0, sizeof(rInfo) ); 
      cast_union u;
      u.in_ptr = &ChaiScript::use;
      if ( dladdr(static_cast<void*>(u.out_ptr), &rInfo) && rInfo.dli_fname ) { 
        std::string dllpath(rInfo.dli_fname);
        const size_t lastslash = dllpath.rfind('/');
        if (lastslash != std::string::npos)
        {
          dllpath.erase(lastslash);
        }

        // Let's see if this is a link that we should expand
        std::vector<char> buf(2048);
        const size_t pathlen = readlink(dllpath.c_str(), &buf.front(), buf.size());
        if (pathlen > 0 && pathlen < buf.size())
        {
          dllpath = std::string(&buf.front(), pathlen);
        }

        m_module_paths.insert(m_module_paths.begin(), dllpath+"/");
      }
#endif


      // attempt to load the stdlib
      load_module("chaiscript_stdlib-" + version());

      build_eval_system(ModulePtr());
    }


    const Boxed_Value eval(const AST_NodePtr &t_ast)
    {
      try {
        return t_ast->eval(m_engine);
      } catch (const exception::eval_error &t_ee) {
        throw Boxed_Value(t_ee);
      }
    }

    static AST_NodePtr parse(const std::string &t_input)
    {
      parser::ChaiScript_Parser parser;
      if (parser.parse(t_input, "PARSE")) {
        //parser.show_match_stack();
        return parser.optimized_ast();
      } else {
        throw chaiscript::exception::eval_error("Unknown error while parsing");
      }
    }


    static int version_major()
    {
      return chaiscript::version_major;
    }

    static int version_minor()
    {
      return chaiscript::version_minor;
    }

    static int version_patch()
    {
      return chaiscript::version_patch;
    }

    static std::string version()
    {
      return std::to_string(version_major()) + '.' + std::to_string(version_minor()) + '.' + std::to_string(version_patch());
    }

    static std::string compiler_id()
    {
      return compiler_name() + '-' + compiler_version();
    }

    static std::string build_id()
    {
      return compiler_id() + (debug_build()?"-Debug":"-Release");
    }

    static std::string compiler_version()
    {
      return chaiscript::compiler_version;
    }

    static std::string compiler_name()
    {
      return chaiscript::compiler_name;
    }

    static bool debug_build()
    {
      return chaiscript::debug_build;
    }



    std::string get_type_name(const Type_Info &ti) const
    {
      return m_engine.get_type_name(ti);
    }

    template<typename T>
    std::string get_type_name() const
    {
      return get_type_name(user_type<T>());
    }


    /// \brief Loads and parses a file. If the file is already, it is not reloaded
    /// The use paths specified at ChaiScript construction time are searched for the 
    /// requested file.
    ///
    /// \param[in] t_filename Filename to load and evaluate
    Boxed_Value use(const std::string &t_filename)
    {
      for (const auto &path : m_use_paths)
      {
        try {
          const auto appendedpath = path + t_filename;

          chaiscript::detail::threading::unique_lock<chaiscript::detail::threading::recursive_mutex> l(m_use_mutex);
          chaiscript::detail::threading::unique_lock<chaiscript::detail::threading::shared_mutex> l2(m_mutex);

          Boxed_Value retval;

          if (m_used_files.count(appendedpath) == 0)
          {
            l2.unlock();
            retval = eval_file(appendedpath);
            l2.lock();
            m_used_files.insert(appendedpath);
          }

          return retval; // return, we loaded it, or it was already loaded
        } catch (const exception::file_not_found_error &) {
          // failed to load, try the next path
        }
      }

      // failed to load by any name
      throw exception::file_not_found_error(t_filename);
    }

    /// \brief Adds a constant object that is available in all contexts and to all threads
    /// \param[in] t_bv Boxed_Value to add as a global
    /// \param[in] t_name Name of the value to add
    /// \throw chaiscript::exception::global_non_const If t_bv is not a constant object
    /// \sa Boxed_Value::is_const
    ChaiScript &add_global_const(const Boxed_Value &t_bv, const std::string &t_name)
    {
      m_engine.add_global_const(t_bv, t_name);
      return *this;
    }

    /// \brief Adds a mutable object that is available in all contexts and to all threads
    /// \param[in] t_bv Boxed_Value to add as a global
    /// \param[in] t_name Name of the value to add
    /// \warning The user is responsible for making sure the object is thread-safe if necessary
    ///          ChaiScript is thread-safe but provides no threading locking mechanism to the script
    ChaiScript &add_global(const Boxed_Value &t_bv, const std::string &t_name)
    {
      m_engine.add_global(t_bv, t_name);
      return *this;
    }

    ChaiScript &set_global(const Boxed_Value &t_bv, const std::string &t_name)
    {
      m_engine.set_global(t_bv, t_name);
      return *this;
    }

    /// \brief Represents the current state of the ChaiScript system. State and be saved and restored
    /// \warning State object does not contain the user defined type conversions of the engine. They
    ///          are left out due to performance considerations involved in tracking the state
    /// \sa ChaiScript::get_state
    /// \sa ChaiScript::set_state
    struct State
    {
      std::set<std::string> used_files;
      chaiscript::detail::Dispatch_Engine::State engine_state;
      std::set<std::string> active_loaded_modules;
    };

    /// \brief Returns a state object that represents the current state of the global system
    ///
    /// The global system includes the reserved words, global const objects, functions and types.
    /// local variables are thread specific and not included.
    ///
    /// \return Current state of the global system
    ///
    /// \b Example:
    ///
    /// \code
    /// chaiscript::ChaiScript chai;
    /// chaiscript::ChaiScript::State s = chai.get_state(); // represents bootstrapped initial state
    /// \endcode
    State get_state() const
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
    ///
    /// The global system includes the reserved words, global objects, functions and types.
    /// local variables are thread specific and not included.
    ///
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

    /// \returns All values in the local thread state, added through the add() function
    std::map<std::string, Boxed_Value> get_locals() const
    {
      return m_engine.get_locals();
    }

    /// \brief Sets all of the locals for the current thread state.
    ///
    /// \param[in] t_locals The map<name, value> set of variables to replace the current state with
    ///
    /// Any existing locals are removed and the given set of variables is added
    void set_locals(const std::map<std::string, Boxed_Value> &t_locals)
    {
      m_engine.set_locals(t_locals);
    }

    /// \brief Adds a type, function or object to ChaiScript. Objects are added to the local thread state.
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
    /// \sa \ref adding_items
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
    ChaiScript &add(const Type_Conversion &d)
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
    /// and with standard prefixes and postfixes: ("lib"|"")\<t_module_name\>(".dll"|".so"|".bundle"|"").
    ///
    /// Once the file is located, the system looks for the symbol "create_chaiscript_module_\<t_module_name\>".
    /// If no file can be found matching the search criteria and containing the appropriate entry point 
    /// (the symbol mentioned above), an exception is thrown.
    ///
    /// \throw chaiscript::exception::load_module_error In the event that no matching module can be found.
    std::string load_module(const std::string &t_module_name)
    {
      std::vector<exception::load_module_error> errors;
      std::string version_stripped_name = t_module_name;
      size_t version_pos = version_stripped_name.find("-"+version());
      if (version_pos != std::string::npos)
      {
        version_stripped_name.erase(version_pos);
      }

      std::vector<std::string> prefixes{"lib", "cyg", ""};

      std::vector<std::string> postfixes{".dll", ".so", ".bundle", ""};

      for (auto & elem : m_module_paths)
      {
        for (auto & prefix : prefixes)
        {
          for (auto & postfix : postfixes)
          {
            try {
              const auto name = elem + prefix + t_module_name + postfix;
              // std::cerr << "trying location: " << name << '\n';
              load_module(version_stripped_name, name);
              return name;
            } catch (const chaiscript::exception::load_module_error &e) {
              // std::cerr << "error: " << e.what() << '\n';
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

      throw chaiscript::exception::load_module_error("Unable to find module: " + t_module_name + " Errors: " + errstring);
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
    /// \throw chaiscript::exception::eval_error In the case that evaluation fails.
    Boxed_Value operator()(const std::string &t_script, const Exception_Handler &t_handler = Exception_Handler())
    {
      try {
        return do_eval(t_script);
      } catch (Boxed_Value &bv) {
        if (t_handler) {
          t_handler->handle(bv, m_engine);
        }
        throw;
      }
    }

    /// \brief Evaluates a string and returns a typesafe result.
    ///
    /// \tparam T Type to extract from the result value of the script execution
    /// \param[in] t_input Script to execute
    /// \param[in] t_handler Optional Exception_Handler used for automatic unboxing of script thrown exceptions
    /// \param[in] t_filename Optional filename to report to the user for where the error occured. Useful
    ///                       in special cases where you are loading a file internally instead of using eval_file
    ///
    /// \return result of the script execution
    /// 
    /// \throw chaiscript::exception::eval_error In the case that evaluation fails.
    /// \throw chaiscript::exception::bad_boxed_cast In the case that evaluation succeeds but the result value cannot be converted
    ///        to the requested type.
    template<typename T>
    T eval(const std::string &t_input, const Exception_Handler &t_handler = Exception_Handler(), const std::string &t_filename="__EVAL__")
    {
      try {
        return m_engine.boxed_cast<T>(do_eval(t_input, t_filename));
      } catch (Boxed_Value &bv) {
        if (t_handler) {
          t_handler->handle(bv, m_engine);
        }
        throw;
      }
    }

    /// \brief casts an object while applying any Dynamic_Conversion available
    template<typename Type>
      typename detail::Cast_Helper<Type>::Result_Type boxed_cast(const Boxed_Value &bv) const
      {
        return m_engine.boxed_cast<Type>(bv);
      }
 

    /// \brief Evaluates a string.
    ///
    /// \param[in] t_input Script to execute
    /// \param[in] t_handler Optional Exception_Handler used for automatic unboxing of script thrown exceptions
    /// \param[in] t_filename Optional filename to report to the user for where the error occurred. Useful
    ///                       in special cases where you are loading a file internally instead of using eval_file
    ///
    /// \return result of the script execution
    /// 
    /// \throw exception::eval_error In the case that evaluation fails.
    Boxed_Value eval(const std::string &t_input, const Exception_Handler &t_handler = Exception_Handler(), const std::string &t_filename="__EVAL__")
    {
      try {
        return do_eval(t_input, t_filename);
      } catch (Boxed_Value &bv) {
        if (t_handler) {
          t_handler->handle(bv, m_engine);
        }
        throw;
      }
    }

    /// \brief Loads the file specified by filename, evaluates it, and returns the result.
    /// \param[in] t_filename File to load and parse.
    /// \param[in] t_handler Optional Exception_Handler used for automatic unboxing of script thrown exceptions
    /// \return result of the script execution
    /// \throw chaiscript::exception::eval_error In the case that evaluation fails.
    Boxed_Value eval_file(const std::string &t_filename, const Exception_Handler &t_handler = Exception_Handler()) {
      try {
        return do_eval(load_file(t_filename), t_filename);
      } catch (Boxed_Value &bv) {
        if (t_handler) {
          t_handler->handle(bv, m_engine);
        }
        throw;
      }
    }

    /// \brief Loads the file specified by filename, evaluates it, and returns the type safe result.
    /// \tparam T Type to extract from the result value of the script execution
    /// \param[in] t_filename File to load and parse.
    /// \param[in] t_handler Optional Exception_Handler used for automatic unboxing of script thrown exceptions
    /// \return result of the script execution
    /// \throw chaiscript::exception::eval_error In the case that evaluation fails.
    /// \throw chaiscript::exception::bad_boxed_cast In the case that evaluation succeeds but the result value cannot be converted
    ///        to the requested type.
    template<typename T>
    T eval_file(const std::string &t_filename, const Exception_Handler &t_handler = Exception_Handler()) {
      try {
        return m_engine.boxed_cast<T>(do_eval(load_file(t_filename), t_filename));
      } catch (Boxed_Value &bv) {
        if (t_handler) {
          t_handler->handle(bv, m_engine);
        }
        throw;
      }
    }
  };

}
#endif /* CHAISCRIPT_ENGINE_HPP_ */

