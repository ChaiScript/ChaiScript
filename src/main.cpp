// This file is distributed under the BSD License.
// See "license.txt" for details.
// Copyright 2009-2012, Jonathan Turner (jonathan@emptycrate.com)
// Copyright 2009-2017, Jason Turner (jason@emptycrate.com)
// http://www.chaiscript.com

// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com


#include <iostream>
#include <list>
#include <regex>

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <chaiscript/chaiscript_basic.hpp>
#include "../static_libs/chaiscript_parser.hpp"
#include "../static_libs/chaiscript_stdlib.hpp"


#ifdef READLINE_AVAILABLE
#include <readline/readline.h>
#include <readline/history.h>
#else

char *mystrdup (const char *s) {
  const size_t len = strlen(s); // Space for length plus nul
  char *d = static_cast<char*>(malloc (len+1));
  if (d == nullptr) { return nullptr; }         // No memory
#ifdef CHAISCRIPT_MSVC
  strcpy_s(d, len+1, s);                        // Copy the characters
#else
  strncpy(d,s,len);                        // Copy the characters
#endif
  d[len] = '\0';
  return d;                            // Return the new string
}

char* readline(const char* p)
{
  std::string retval;
  std::cout << p ;
  std::getline(std::cin, retval);
  return std::cin.eof() ? nullptr : mystrdup(retval.c_str());
}


void add_history(const char* /*unused*/){}
void using_history(){}
#endif



void *cast_module_symbol(std::vector<std::string> (*t_path)())
{
  union cast_union
  {
    std::vector<std::string> (*in_ptr)();
    void *out_ptr;
  };

  cast_union c;
  c.in_ptr = t_path;
  return c.out_ptr;
}

std::vector<std::string> default_search_paths()
{
  std::vector<std::string> paths;

#ifndef CHAISCRIPT_NO_DYNLOAD
#ifdef CHAISCRIPT_WINDOWS  // force no unicode
  CHAR path[4096];
  int size = GetModuleFileNameA(nullptr, path, sizeof(path)-1);

  std::string exepath(path, size);

  const size_t lastslash = exepath.rfind('\\');
  const size_t secondtolastslash = exepath.rfind('\\', lastslash - 1);
  if (lastslash != std::string::npos)
  {
    paths.push_back(exepath.substr(0, lastslash));
  }

  if (secondtolastslash != std::string::npos)
  {
    return {exepath.substr(0, secondtolastslash) + R"(\lib\chaiscript\)"};
  }
#else

  std::string exepath;

  std::vector<char> buf(2048);
  ssize_t size = -1;

  if ((size = readlink("/proc/self/exe", &buf.front(), buf.size())) >= 0)
  {
    exepath = std::string(&buf.front(), static_cast<size_t>(size));
  }

  if (exepath.empty())
  {
    if ((size = readlink("/proc/curproc/file", &buf.front(), buf.size())) >= 0)
    {
      exepath = std::string(&buf.front(), static_cast<size_t>(size));
    }
  }

  if (exepath.empty())
  {
    if ((size = readlink("/proc/self/path/a.out", &buf.front(), buf.size())) >= 0)
    {
      exepath = std::string(&buf.front(), static_cast<size_t>(size));
    }
  }

  if (exepath.empty())
  {
    Dl_info rInfo;
    memset( &rInfo, 0, sizeof(rInfo) ); 
    if ( dladdr(cast_module_symbol(&default_search_paths), &rInfo) == 0 || rInfo.dli_fname == nullptr ) { 
      return paths;
    }

    exepath = std::string(rInfo.dli_fname);
  }

  const size_t lastslash = exepath.rfind('/');

  size_t secondtolastslash = exepath.rfind('/', lastslash - 1);
  if (lastslash != std::string::npos)
  {
    paths.push_back(exepath.substr(0, lastslash+1));
  }

  if (secondtolastslash != std::string::npos)
  {
    paths.push_back(exepath.substr(0, secondtolastslash) + "/lib/chaiscript/");
  }
#endif
#endif // ifndef CHAISCRIPT_NO_DYNLOAD

  return paths;
}

void help(int n) {
  if ( n >= 0 ) {
    std::cout << "ChaiScript evaluator.  To evaluate an expression, type it and press <enter>.\n";
    std::cout << "Additionally, you can inspect the runtime system using:\n";
    std::cout << "  dump_system() - outputs all functions registered to the system\n";
    std::cout << "  dump_object(x) - dumps information about the given symbol\n";
  } else {
    std::cout << "usage : chai [option]+\n";
    std::cout << "option:"                << '\n';
    std::cout << "   -h | --help"         << '\n';
    std::cout << "   -i | --interactive"  << '\n';
    std::cout << "   -c | --command cmd"  << '\n';
    std::cout << "   -v | --version"      << '\n';
    std::cout << "   -    --stdin"        << '\n';
    std::cout << "   filepath"            << '\n';
  }
}

bool throws_exception(const std::function<void ()> &f)
{
  try {
    f();
  } catch (...) {
    return true;
  }

  return false;
}

chaiscript::exception::eval_error get_eval_error(const std::function<void ()> &f)
{
  try {
    f();
  } catch (const chaiscript::exception::eval_error &e) {
    return e;
  }

  throw std::runtime_error("no exception throw");
}

std::string get_next_command() {
  std::string retval("quit");
  if ( ! std::cin.eof() ) {
    char *input_raw = readline("eval> ");
    if ( input_raw != nullptr ) {
      add_history(input_raw);

      std::string val(input_raw);
      size_t pos = val.find_first_not_of("\t \n");
      if (pos != std::string::npos)
      {
        val.erase(0, pos);
      }
      pos = val.find_last_not_of("\t \n");
      if (pos != std::string::npos)
      {
        val.erase(pos+1, std::string::npos);
      }

      retval = val;

      ::free(input_raw);
    }
  }
  if(   retval == "quit"
     || retval == "exit"
     || retval == "help"
     || retval == "version") 
  {
    retval += "(0)";
  }
  return retval;
}

// We have to wrap exit with our own because Clang has a hard time with
// function pointers to functions with special attributes (system exit being marked NORETURN)
void myexit(int return_val) {
  exit(return_val);
}

void interactive(chaiscript::ChaiScript_Basic& chai)
{
  using_history();

  for (;;) {
    const std::string input = get_next_command();
    try {
      // evaluate input
      chaiscript::Boxed_Value val = chai.eval(input);

      //Then, we try to print the result of the evaluation to the user
      if (!val.get_type_info().bare_equal(chaiscript::user_type<void>())) {
        try {
          std::cout << chai.eval<std::function<std::string (const chaiscript::Boxed_Value &bv)> >("to_string")(val) << '\n';
        }
        catch (...) {} //If we can't, do nothing
      }
    }
    catch (const chaiscript::exception::eval_error &ee) {
      std::cout << ee.what();
      if ( !ee.call_stack.empty() ) {
        std::cout << "during evaluation at (" << ee.call_stack[0].start().line << ", " << ee.call_stack[0].start().column << ")";
      }
      std::cout << '\n';
    }
    catch (const std::exception &e) {
      std::cout << e.what();
      std::cout << '\n';
    }
  }
}

double now()
{
  using namespace std::chrono;
  auto now = high_resolution_clock::now();
  return duration_cast<duration<double>>(now.time_since_epoch()).count();
}

int main(int argc, char *argv[])
{

  // Disable deprecation warning for getenv call.
#ifdef CHAISCRIPT_MSVC
#pragma warning(push)
#pragma warning(disable : 4996)
#endif

  const char *usepath = getenv("CHAI_USE_PATH");
  const char *modulepath = getenv("CHAI_MODULE_PATH");

#ifdef CHAISCRIPT_MSVC
#pragma warning(pop)
#endif

  std::vector<std::string> usepaths;
  usepaths.push_back("");
  if (usepath != nullptr)
  {
    usepaths.push_back(usepath);
  }

  std::vector<std::string> modulepaths;
  std::vector<std::string> searchpaths = default_search_paths();
  modulepaths.insert(modulepaths.end(), searchpaths.begin(), searchpaths.end());
  modulepaths.push_back("");
  if (modulepath != nullptr)
  {
    modulepaths.push_back(modulepath);
  }

  chaiscript::ChaiScript_Basic chai(create_chaiscript_stdlib(),create_chaiscript_parser(),modulepaths,usepaths);

  chai.add(chaiscript::fun(&myexit), "exit");
  chai.add(chaiscript::fun(&myexit), "quit");
  chai.add(chaiscript::fun(&help), "help");
  chai.add(chaiscript::fun(&throws_exception), "throws_exception");
  chai.add(chaiscript::fun(&get_eval_error), "get_eval_error");
  chai.add(chaiscript::fun(&now), "now");

  bool eval_error_ok = false;
  bool boxed_exception_ok = false;
  bool any_exception_ok = false;

  for (int i = 0; i < argc; ++i) {
    if ( i == 0 && argc > 1 ) {
      ++i;
    }

    std::string arg( i != 0 ? argv[i] : "--interactive" );

    enum { eInteractive
         , eCommand
         , eFile
    } mode = eCommand ;

    if  ( arg == "-c" || arg == "--command" ) {
      if ( (i+1) >= argc ) {
        std::cout << "insufficient input following " << arg << '\n';
        return EXIT_FAILURE;
      } 
        arg = argv[++i];
      
    } else if ( arg == "-" || arg == "--stdin" ) {
      arg = "" ;
      std::string line;
      while ( std::getline(std::cin, line) ) {
        arg += line + '\n' ;
      }
    } else if ( arg == "-v" || arg == "--version" ) {
      arg = "print(version())" ;
    } else if ( arg == "-h" || arg == "--help" ) {
      arg = "help(-1)";
    } else if ( arg == "-e" || arg == "--evalerrorok" ) {
      eval_error_ok = true;
      continue;
    } else if ( arg == "--exception" ) {
      boxed_exception_ok = true;
      continue;
    } else if ( arg == "--any-exception" ) {
      any_exception_ok = true;
      continue;
    } else if ( arg == "-i" || arg == "--interactive" ) {
      mode = eInteractive ;
    } else if ( arg.find('-') == 0 ) {
      std::cout << "unrecognised argument " << arg << '\n';
      return EXIT_FAILURE;
    } else {
      mode = eFile;
    }

    try {
      switch ( mode ) {
        case eInteractive:
          interactive(chai);
          break;
        case eCommand:
          chai.eval(arg);
          break;
        case eFile:
          chai.eval_file(arg);
      }
    }
    catch (const chaiscript::exception::eval_error &ee) {
      std::cout << ee.pretty_print();
      std::cout << '\n';

      if (!eval_error_ok) {
        return EXIT_FAILURE;
      }
    }
    catch (const chaiscript::Boxed_Value &e) {
      std::cout << "Unhandled exception thrown of type " << e.get_type_info().name() << '\n';

      if (!boxed_exception_ok) {
        return EXIT_FAILURE;
      }
    }
    catch (const chaiscript::exception::load_module_error &e) {
      std::cout << "Unhandled module load error\n" << e.what() << '\n';
    }
    catch (std::exception &e) {
      std::cout << "Unhandled standard exception: " << e.what() << '\n';
      if (!any_exception_ok) {
        throw;
      }
    }
    catch (...) {
      std::cout << "Unhandled unknown exception" << '\n';
      if (!any_exception_ok) {
        throw;
      }
    }
  }

  return EXIT_SUCCESS;
}
