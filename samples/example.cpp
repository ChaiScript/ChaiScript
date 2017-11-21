// This file is distributed under the BSD License.
// See "license.txt" for details.
// Copyright 2009-2010, Jonathan Turner (jonathan@emptycrate.com)
// and Jason Turner (jason@emptycrate.com)
// http://www.chaiscript.com

#include <iostream>
#include <ctime>

#include <chaiscript/chaiscript.hpp>
#include <chaiscript/dispatchkit/bootstrap_stl.hpp>
#include <chaiscript/dispatchkit/function_call.hpp>

void log(const std::string &msg)
{
  std::cout << "[" << time(nullptr) << "] " << msg << '\n';
}

void log(const std::string &module, const std::string &msg)
{
  std::cout << "[" << time(nullptr) << "] <" << module << "> " << msg << '\n';
}

void bound_log(const std::string &msg)
{
  log(msg);
}

void hello_world(const chaiscript::Boxed_Value & /*o*/)
{
  std::cout << "Hello World\n";
}

void hello_constructor(const chaiscript::Boxed_Value & /*o*/)
{
  std::cout << "Hello Constructor\n";
}


struct System
{
  std::map<std::string, std::function<std::string (const std::string &) > > m_callbacks;

  void add_callback(const std::string &t_name, 
      const std::function<std::string (const std::string &)> &t_func)
  {
    m_callbacks[t_name] = t_func;
  }


  void do_callbacks(const std::string &inp)
  {
    log("Running Callbacks: " + inp);
    for (auto & m_callback : m_callbacks)
    {
      log("Callback: " + m_callback.first, m_callback.second(inp));
    }
  }
};

void take_shared_ptr(const std::shared_ptr<const std::string> &p)
{
  std::cout << *p << '\n';
}

int main(int /*argc*/, char * /*argv*/[]) {
  using namespace chaiscript;

  ChaiScript chai;
  

  //Create a new system object and share it with the chaiscript engine
  System system;
  chai.add_global(var(&system), "system");

  //Add a bound callback method
  chai.add(fun(&System::add_callback, std::ref(system)), "add_callback_bound");

  //Register the two methods of the System structure.
  chai.add(fun(&System::add_callback), "add_callback");
  chai.add(fun(&System::do_callbacks), "do_callbacks");

  chai.add(fun(&take_shared_ptr), "take_shared_ptr");

  // Let's use chaiscript to add a new lambda callback to our system. 
  // The function "{ 'Callback1' + x }" is created in chaiscript and passed into our C++ application
  // in the "add_callback" function of struct System the chaiscript function is converted into a 
  // std::function, so it can be handled and called easily and type-safely
  chai.eval(R"(system.add_callback("#1", fun(x) { "Callback1 " + x });)");
  
  // Because we are sharing the "system" object with the chaiscript engine we have equal
  // access to it both from within chaiscript and from C++ code
  system.do_callbacks("TestString");
  chai.eval(R"(system.do_callbacks("TestString");)");

  // The log function is overloaded, therefore we have to give the C++ compiler a hint as to which
  // version we want to register. One way to do this is to create a typedef of the function pointer
  // then cast your function to that typedef.
  using PlainLog = void (*)(const std::string &);
  using ModuleLog = void (*)(const std::string &, const std::string &);
  chai.add(fun(PlainLog(&log)), "log");
  chai.add(fun(ModuleLog(&log)), "log");

  chai.eval(R"(log("Test Message"))");

  // A shortcut to using eval is just to use the chai operator()
  chai(R"(log("Test Module", "Test Message");)");

  //Finally, it is possible to register a lambda as a system function, in this 
  //way, we can, for instance add a bound member function to the system
  chai.add(fun([&system](){ return system.do_callbacks("Bound Test"); }), "do_callbacks");

  //Call bound version of do_callbacks
  chai("do_callbacks()");

  std::function<void ()> caller = chai.eval<std::function<void ()> >(
    R"(fun() { system.do_callbacks("From Functor"); })"
  );
  caller();


  //If we would like a type-safe return value from all call, we can use
  //the templated version of eval:
  int i = chai.eval<int>("5+5");

  std::cout << "5+5: " << i << '\n';

  //Add a new variable
  chai("var scripti = 15");

  //We can even get a handle to the variables in the system
  int &scripti = chai.eval<int &>("scripti");

  std::cout << "scripti: " << scripti << '\n';
  scripti *= 2;
  std::cout << "scripti (updated): " << scripti << '\n';
  chai(R"(print("Scripti from chai: " + to_string(scripti)))");

  //To do: Add examples of handling Boxed_Values directly when needed

  //Creating a functor on the stack and using it immediately 
  int x = chai.eval<std::function<int (int, int)> >("fun (x, y) { return x + y; }")(5, 6);

  std::stringstream ss;
  ss << x;
  log("Functor test output", ss.str());

  chai.add(var(std::shared_ptr<int>()), "nullvar");
  chai(R"(print("This should be true."); print(nullvar.is_var_null()))");

  // test the global const action
  chai.add_global_const(const_var(1), "constvar");
  chai("def getvar() { return constvar; }");
  chai("print( getvar() )");


  //Ability to create our own container types when needed. std::vector and std::map are
  //mostly supported currently
  chai.add(bootstrap::standard_library::vector_type<std::vector<int> >("IntVector"));


  // Test ability to register a function that excepts a shared_ptr version of a type
  chai(R"(take_shared_ptr("Hello World as a shared_ptr");)");

  chai.add(fun(&bound_log, std::string("Msg")), "BoundFun");

  //Dynamic objects test
  chai.add(chaiscript::Proxy_Function(new dispatch::detail::Dynamic_Object_Function("TestType", fun(&hello_world))), "hello_world");
  chai.add(chaiscript::Proxy_Function(new dispatch::detail::Dynamic_Object_Constructor("TestType", fun(&hello_constructor))), "TestType");
//  chai.add(fun(std::function<Boxed_Value (dispatch::Dynamic_Object &)>(std::bind(&dispatch::detail::Dynamic_Object_Attribute::func, "TestType", "attr", std::placeholders::_1))), "attr");

  chai.eval("var x = TestType()");
//  chai.eval("x.attr = \"hi\"");
//  chai.eval("print(x.attr)");
  chai.eval("x.hello_world()");
}

