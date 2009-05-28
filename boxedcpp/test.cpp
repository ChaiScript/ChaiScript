#include <iostream>
#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>

#include "boxedcpp.hpp"

struct Test
{
  Test(const std::string &s)
    : message(s)
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

  std::string message;
};


//A function that prints any string passed to it
void print(const std::string &s)
{
  std::cout << "Printed: " << s << std::endl;
}


// A function that takes a dynamic list of params
// and calls a bunch of conversion functions on them and 
// returns the result as a boxed_value
Boxed_Value dynamic_function(BoxedCPP_System &ss, const std::string &name, 
    const std::vector<Boxed_Value> &params)
{
  if (name == "concat_string")
  {
    Boxed_Value result;

    //Return a void if there is nothing in the array
    if (params.size() == 0)
    {
      return result;
    } else {
      //else, prepopulate the result with a string conversion of the first
      //param
      result = 
        dispatch(ss.get_function("to_string"), Param_List_Builder() << params[0]);
    }

    //Then, loop over all remaining params, converting them to strings and adding
    //them to the result. This example maybe bette served with a string += operator
    //implementation, but it works.
    for (size_t i = 1; i < params.size(); ++i)
    {
      result = 
        dispatch(ss.get_function("+"), Param_List_Builder() << result << 
            dispatch(ss.get_function("to_string"), Param_List_Builder() << params[i]));
    }

    return result;
  } else {
    throw std::runtime_error("Unknown function call");
  }
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

  // Now we are going to register a new dynamic function,
  // when this function is called the objects are not unboxed, but passed
  // in in their boxed state
  ss.register_function(boost::shared_ptr<Proxy_Function>(new Dynamic_Proxy_Function(boost::bind(&dynamic_function, boost::ref(ss), "concat_string", _1))), "concat_string");


  // Call our newly defined dynamic function with 10 parameters, then send
  // its output to the "print" function
  dispatch(ss.get_function("print"),
      Param_List_Builder() << dispatch(ss.get_function("concat_string"),
        Param_List_Builder() << std::string("\n\t") << std::string("The Value Was: ") 
                             << double(42.5) << std::string(".")
                             << '\n'
                             << '\t' << std::string("The old value was: ")
                             << addresult << '.' << '\n' ));




  //Register some local methods of the "Test" class
  ss.register_function(build_constructor<Test, const std::string &>(), "Test");
  ss.register_function(boost::function<std::string &(Test *)>(&Test::get_message), "get_message");
  ss.register_function(boost::function<void (Test *)>(&Test::show_message), "show_message");

  //Create a new object using the "Test" constructor, passing the param "Yo".
  //Then, add the new object to the system with the name "testobj2"
  ss.add_object("testobj2", 
      dispatch(ss.get_function("Test"), Param_List_Builder() << std::string("Yo")));

  // Look up and store a reference to our new object
  std::vector<Boxed_Value> sos;
  sos.push_back(ss.get_object("testobj2"));

  //Print the message the object was created with
  dispatch(ss.get_function("show_message"), sos);

  //Now, get a reference to the object's stored message
  Boxed_Value stringref = dispatch(ss.get_function("get_message"), sos);

  //Unbox it using Cast_Helper
  std::string &sr = Cast_Helper<std::string &>()(stringref);

  //Update the value of the reference
  sr = "Bob Updated The message";

  //Now, prove that the reference was successfully acquired
  //and we are able to peek into the boxed types
  dispatch(ss.get_function("show_message"), sos);
}

