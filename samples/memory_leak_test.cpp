#include <iostream>

#include "chaiscript/chaiscript.hpp"
#ifdef READLINE_AVAILABLE
#include <readline/readline.h>
#include <readline/history.h>
#endif


using namespace chaiscript;

std::string get_next_command() {
#ifdef READLINE_AVAILABLE
  char *input_raw;
  input_raw = readline("eval> ");
  add_history(input_raw);
  return std::string(input_raw);
#else
  std::string retval;
  std::cout << "eval> ";
  std::getline(std::cin, retval);
  return retval;
#endif
}

void fuction(void)
{
  // do nothing
}

class test
{
  ChaiScript chai;
  ChaiScript::State backupState;
  public:
  test()
  {		
    backupState = chai.get_state();
  }
  ~test(){}

  void ResetState()
  {
    chai.set_state(backupState);
    chai.add(fun(&fuction),"Whatever()");
  }

  void RunFile(std::string sFile)
  {
    try {
      chaiscript::Boxed_Value val = chai.eval_file(sFile);
    }
    catch (std::exception &e) {
      std::cout << e.what() << std::endl;
    }
  }

};



int main(int /*argc*/, char * /*argv*/[]) {

  test myChai;


  std::string command = "";

  //
  // this loop increases memoryusage, if RunFile is not called (just hittin enter)
  // as soon RunFile gets called, memory will be freed.
  //
  // scenario1 - RunFile gets called every Loop: memoryusage does not change
  // scenario2 - RunFile gets never called (just hitting enter): memoryusage increases every loop
  // scenario3 - RunFile gets in changing intervals: memoryusage goes up and down, but never as 
  //            low as in case 1 scenario3 :

  while(command != "quit")
  {
    for(int i = 1; i < 200; i++)
      myChai.ResetState();

    if(command == "runfile")
      myChai.RunFile("Test.chai");

    command = get_next_command();
  }
}
