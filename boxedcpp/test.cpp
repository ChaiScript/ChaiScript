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

std::string testprint(const std::string &p)
{
  std::cout << p << std::endl;
  return p;
}

void print()
{
  std::cout << "Test void function succeeded" << std::endl;
}


//Test main
int main()
{
  BoxedCPP_System ss;
  bootstrap(ss);

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


  dump_system(ss);


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
}
  
