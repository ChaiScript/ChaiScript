#include <iostream>
#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>

#include "boxedcpp.hpp"

struct Test
{
  Test()
    : md(0)
  {
  }

  Test(const std::string &s)
    : message(s), md(0)
  {
    std::cout << "Test class constructed with value: " << s << std::endl;
  }

  void show_message()
  {
    std::cout << "Constructed Message: " << message << std::endl;
  }

  std::string &get_message()
  {
    return message;
  }

  int method(double d)
  {
    md += d;
    std::cout << "Method called " << md << std::endl;
    return int(md);
  }

  std::string message;
  double md;
};

void print(const std::string &s)
{
  std::cout << "Printed: " << s << std::endl;
}


//Test main
int main()
{
  BoxedCPP_System ss;
  bootstrap(ss);
  dump_system(ss);

  //Calling a function by name and allowing the built in dispatch mechanism to
  //choose the most appropriate version of the function
  Boxed_Value addresult = dispatch(ss.get_function("+"), Param_List_Builder() << double(5.1) << double(10.3));

  //Using the Cast_Helper to unbox the resultant value and output it
  std::cout << Cast_Helper<double>()(addresult) << std::endl;

  //Using the Boxed_Value as input to another function, again with automatic dispatch.
  //This time we will not bother saving the result and will instead send it straight out
  std::cout << Cast_Helper<double>()(
      dispatch(ss.get_function("*"), Param_List_Builder() << 2 << addresult)
      ) << std::endl;

  //Register a new function, this one with typing for us, so we don't have to ubox anything
  //right here
  ss.register_function(boost::function<void (const std::string &)>(&print), "print");

  //Now we have a print method, let's try to print out the earlier example:
  //so, we dispatch the to_string and pass its result as a param to "print"
  //In this example we don't bother with temporaries and we don't have to know
  //anything about types
  dispatch(ss.get_function("print"),
      Param_List_Builder() << dispatch(ss.get_function("to_string"), Param_List_Builder() << addresult));

  //
  //
  /* Older tests
  ss.register_type<Test>("Test");
  ss.register_function(boost::function<int (Test*, double)>(&Test::method), "method");
  ss.register_function(boost::function<std::string (const std::string &)>(&testprint), "print");
  ss.register_function(boost::function<void ()>(&print), "voidfunc");
  ss.register_function(build_constructor<Test, const std::string &>(), "Test");
  ss.register_function(boost::function<void (Test*)>(&Test::show_message), "show_message");
  ss.register_function(boost::function<std::string &(Test*)>(&Test::get_message), "get_message");


  ss.add_object("testobj", boost::shared_ptr<Test>(new Test()));
  ss.add_object("d", boost::shared_ptr<double>(new double(10.2)));
  ss.add_object("str", std::string("Hello World"));



  std::vector<Boxed_Value> sos;
  sos.push_back(ss.get_object("testobj"));
  sos.push_back(ss.get_object("d"));

  boost::shared_ptr<Proxy_Function> method1(ss.get_function("method").front().second);
  (*method1)(sos);
  (*method1)(sos);
  Boxed_Value o = (*method1)(sos);

  dispatch(ss.get_function("print"), Param_List_Builder() << ss.get_object("str"));

  //Add new dynamically created object from registered "Test" constructor
  ss.add_object("testobj2", dispatch(ss.get_function("Test"), Param_List_Builder() << std::string("Yo")));

  std::cout << Cast_Helper<int>()(o) << std::endl;

  dispatch(ss.get_function("voidfunc"), Param_List_Builder());

  std::vector<Boxed_Value> sos3;
  sos3.push_back(ss.get_object("testobj2"));
  dispatch(ss.get_function("show_message"), sos3);

  Boxed_Value stringref = dispatch(ss.get_function("get_message"), sos3);

  std::string &sr = Cast_Helper<std::string &>()(stringref);
  sr = "Bob Updated The message";

  dispatch(ss.get_function("show_message"), sos3);
  */
}

