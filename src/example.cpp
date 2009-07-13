// This file is distributed under the BSD License.
// See "license.txt" for details.
// Copyright 2009, Jonathan Turner (jturner@minnow-lang.org)
// and Jason Turner (lefticus@gmail.com)
// http://www.chaiscript.com

#include <iostream>

#include <chaiscript/chaiscript.hpp>
#include <chaiscript/dispatchkit/function_call.hpp>
#include <boost/function.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/regex.hpp>

void log(const std::string &msg)
{
  std::cout << "[" << boost::posix_time::microsec_clock::local_time() << "] " << msg << std::endl;
}

void log(const std::string &module, const std::string &msg)
{
  std::cout << "[" << boost::posix_time::microsec_clock::local_time() << "] <" << module << "> " << msg << std::endl;
}

struct System
{
  std::map<std::string, boost::function<std::string (const std::string &) > > m_callbacks;

  void add_callback(const std::string &t_name, 
      boost::shared_ptr<dispatchkit::Proxy_Function> t_func)
  {
    m_callbacks[t_name] = dispatchkit::build_function_caller<std::string (const std::string &)>(t_func);
  }


  void do_callbacks(const std::string &inp)
  {
    log("Running Callbacks: " + inp);
    for (std::map<std::string, boost::function<std::string (const std::string &)> >::iterator itr = m_callbacks.begin();
         itr != m_callbacks.end();
         ++itr)
    {
      log("Callback: " + itr->first, itr->second(inp));
    }
  }
};


int main(int argc, char *argv[]) {


  chaiscript::ChaiScript_Engine chai;

  //Create a new system object and share it with the chaiscript engine
  System system;
  chai.get_eval_engine().add_object("system", boost::ref(system));

  //Register the two methods of the System structure.
  dispatchkit::register_function(chai.get_eval_engine(), &System::add_callback, "add_callback");
  dispatchkit::register_function(chai.get_eval_engine(), &System::do_callbacks, "do_callbacks");

  // Let's use chaiscript to add a new lambda callback to our system. 
  // The function "{ 'Callback1' + x }" is created in chaiscript and passed into our C++ application
  // in the "add_callback" function of struct System the chaiscript function is converted into a 
  // boost::function, so it can be handled and called easily and type-safely
  chai.evaluate_string("system.add_callback('#1', fun(x) { 'Callback1 ' + x });");
  
  // Because we are sharing the "system" object with the chaiscript engine we have equal
  // access to it both from within chaiscript and from C++ code
  system.do_callbacks("TestString");
  chai.evaluate_string("system.do_callbacks(\"TestString\");");

  // The log function is overloaded, therefore we have to give the C++ compiler a hint as to which
  // version we want to register. One way to do this is to create a typedef of the function pointer
  // then cast your function to that typedef.
  typedef void (*PlainLog)(const std::string &);
  typedef void (*ModuleLog)(const std::string &, const std::string &);
  dispatchkit::register_function(chai.get_eval_engine(), PlainLog(&log), "log");
  dispatchkit::register_function(chai.get_eval_engine(), ModuleLog(&log), "log");

  chai.evaluate_string("log('Test Message')");
  chai.evaluate_string("log('Test Module', 'Test Message');");

  //Finally, it is possible to register any boost::function as a system function, in this 
  //way, we can, for instance add a bound member function to the system
  chai.get_eval_engine().register_function(boost::function<void ()>(boost::bind(&System::do_callbacks, boost::ref(system), "Bound Test")), "do_callbacks");

  //Call bound version of do_callbacks
  chai.evaluate_string("do_callbacks()");

}

