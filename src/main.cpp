// This file is distributed under the BSD License.
// See "license.txt" for details.
// Copyright 2009-2010, Jonathan Turner (jonathan@emptycrate.com)
// and Jason Turner (jason@emptycrate.com)
// http://www.chaiscript.com

#include <iostream>
#include <list>
#include <boost/algorithm/string/trim.hpp>

#define _CRT_SECURE_NO_WARNINGS
#include <chaiscript/chaiscript.hpp>

#ifdef READLINE_AVAILABLE
#include <readline/readline.h>
#include <readline/history.h>
#else
char* readline(const char* p)
{
  std::string retval;
  std::cout << p ;
  std::getline(std::cin, retval);
#ifdef BOOST_MSVC
  return std::cin.eof() ? NULL : _strdup(retval.c_str());
#else
  return std::cin.eof() ? NULL : strdup(retval.c_str());
#endif
}
void add_history(const char*){}
void using_history(){}
#endif

void help(int n) {
  if ( n >= 0 ) {
    std::cout << "ChaiScript evaluator.  To evaluate an expression, type it and press <enter>." << std::endl;
    std::cout << "Additionally, you can inspect the runtime system using:" << std::endl;
    std::cout << "  dump_system() - outputs all functions registered to the system" << std::endl;
    std::cout << "  dump_object(x) - dumps information about the given symbol" << std::endl;
  } else {
    std::cout << "usage : chai [option]+" << std::endl;
    std::cout << "option:"                << std::endl;
    std::cout << "   -h | --help"         << std::endl;
    std::cout << "   -i | --interactive"  << std::endl;
    std::cout << "   -c | --command cmd"  << std::endl;
    std::cout << "   -v | --version"      << std::endl;
    std::cout << "   -    --stdin"        << std::endl;
    std::cout << "   filepath"            << std::endl;
  }
}

void version(int){
  std::cout << "chai: compiled " << __TIME__ << " " << __DATE__ << std::endl;
}

bool throws_exception(const boost::function<void ()> &f)
{
  try {
    f();
  } catch (...) {
    return true;
  }

  return false;
}

chaiscript::exception::eval_error get_eval_error(const boost::function<void ()> &f)
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
    if ( input_raw ) {
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

void interactive(chaiscript::ChaiScript& chai)
{
  using_history();

  for (;;) {
    std::string input = get_next_command();
    try {
      // evaluate input
      chaiscript::Boxed_Value val = chai.eval(input);

      //Then, we try to print the result of the evaluation to the user
      if (!val.get_type_info().bare_equal(chaiscript::user_type<void>())) {
        try {
          std::cout << chai.eval<boost::function<std::string (const chaiscript::Boxed_Value &bv)> >("to_string")(val) << std::endl;
        }
        catch (...) {} //If we can't, do nothing
      }
    }
    catch (const chaiscript::exception::eval_error &ee) {
      std::cout << ee.what();
      if (ee.call_stack.size() > 0) {
        std::cout << "during evaluation at (" << ee.call_stack[0]->start.line << ", " << ee.call_stack[0]->start.column << ")";
      }
      std::cout << std::endl;
    }
    catch (const std::exception &e) {
      std::cout << e.what();
      std::cout << std::endl;
    }
  }
}

int main(int argc, char *argv[])
{
  std::vector<std::string> usepaths;
  std::vector<std::string> modulepaths;

  // Disable deprecation warning for getenv call.
#ifdef BOOST_MSVC
#pragma warning(push)
#pragma warning(disable : 4996)
#endif

  const char *usepath = getenv("CHAI_USE_PATH");
  const char *modulepath = getenv("CHAI_MODULE_PATH");

#ifdef BOOST_MSVC
#pragma warning(pop)
#endif

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

  chai.add(chaiscript::fun(&myexit), "exit");
  chai.add(chaiscript::fun(&myexit), "quit");
  chai.add(chaiscript::fun(&help), "help");
  chai.add(chaiscript::fun(&version), "version");
  chai.add(chaiscript::fun(&throws_exception), "throws_exception");
  chai.add(chaiscript::fun(&get_eval_error), "get_eval_error");

  for (int i = 0; i < argc; ++i) {
    if ( i == 0 && argc > 1 ) {
      ++i;
    }

    std::string arg( i ? argv[i] : "--interactive" );

    enum { eInteractive
         , eCommand
         , eFile
    } mode = eCommand ;

    if  ( arg == "-c" || arg == "--command" ) {
      if ( (i+1) >= argc ) {
        std::cout << "insufficient input following " << arg << std::endl;
        return EXIT_FAILURE;
      } else {
        arg = argv[++i];
      }
    } else if ( arg == "-" || arg == "--stdin" ) {
      arg = "" ;
      std::string line;
      while ( std::getline(std::cin, line) ) {
        arg += line + '\n' ;
      }
    } else if ( arg == "-v" || arg == "--version" ) {
      arg = "version(0)" ;
    } else if ( arg == "-h" || arg == "--help" ) {
      arg = "help(-1)";
    } else if ( arg == "-i" || arg == "--interactive" ) {
      mode = eInteractive ;
    } else if ( arg.find('-') == 0 ) {
      std::cout << "unrecognised argument " << arg << std::endl;
      return EXIT_FAILURE;
    } else {
      mode = eFile;
    }

    chaiscript::Boxed_Value val ;
    try {
      switch ( mode ) {
        case eInteractive : interactive(chai); break;
        case eCommand     : val = chai.eval(arg); break;
        case eFile        : val = chai.eval_file(arg); break;
        default           : std::cout << "Unrecognized execution mode" << std::endl; return EXIT_FAILURE;
      }
    }
    catch (const chaiscript::exception::eval_error &ee) {
      std::cout << ee.pretty_print();
      std::cout << std::endl;
      return EXIT_FAILURE;
    }
    catch (std::exception &e) {
      std::cout << e.what() << std::endl;
      return EXIT_FAILURE;
    }
  }

  return EXIT_SUCCESS;
}
