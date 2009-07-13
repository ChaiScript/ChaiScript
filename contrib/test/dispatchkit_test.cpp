// This file is distributed under the BSD License.
// See "license.txt" for details.
// Copyright 2009, Jonathan Turner (jturner@minnow-lang.org)
// and Jason Turner (lefticus@gmail.com)
// http://www.chaiscript.com

#include <iostream>
#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>

#include <chaiscript/dispatchkit/dispatchkit.hpp>
#include <chaiscript/dispatchkit/bootstrap.hpp>
#include <chaiscript/dispatchkit/bootstrap_stl.hpp>
#include <chaiscript/dispatchkit/function_call.hpp>

using namespace dispatchkit;

struct Test
{
  Test(const std::string &s)
    : number(-25), message(s)
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

  int number;

  std::string message;
};



Boxed_Value named_func_call(Dispatch_Engine &ss,
    const std::string &nametocall, const std::vector<Boxed_Value> &params)
{
  if (params.size() == 2)
  {
    return dispatch(ss.get_function(nametocall), params);
  } else {
    throw std::runtime_error("Invalid num params");
  }
}

// A function that takes a dynamic list of params
// and calls a bunch of conversion functions on them and 
// returns the result as a boxed_value
Boxed_Value dynamic_function(Dispatch_Engine &ss, const std::string &name, 
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

void test(const std::string &p)
{
  std::cout << "Test: " << p << std::endl;
}

//Test main
int main()
{
  Dispatch_Engine ss;
  Bootstrap::bootstrap(ss);
  bootstrap_vector<std::vector<int> >(ss, "VectorInt");
  dump_system(ss);

  //Calling a function by name and allowing the built in dispatch mechanism to
  //choose the most appropriate version of the function
  Boxed_Value addresult = dispatch(ss.get_function("+"), Param_List_Builder() << double(5.1) << double(10.3));

  //Using the cast to unbox the resultant value and output it
  std::cout << boxed_cast<double>(addresult) << std::endl;

  //Using the Boxed_Value as input to another function, again with automatic dispatch.
  //This time we will not bother saving the result and will instead send it straight out
  std::cout << boxed_cast<double>(
      dispatch(ss.get_function("*"), Param_List_Builder() << 2 << addresult)
      ) << std::endl;

  //Register a new function, this one with typing for us, so we don't have to ubox anything
  //right here

  //Now we have a print method, let's try to print out the earlier example:
  //so, we dispatch the to_string and pass its result as a param to "print"
  //In this example we don't bother with temporaries and we don't have to know
  //anything about types
  dispatch(ss.get_function("print_string"),
      Param_List_Builder() << dispatch(ss.get_function("to_string"), Param_List_Builder() << addresult));

  // Now we are going to register a new dynamic function,
  // when this function is called the objects are not unboxed, but passed
  // in in their boxed state
  ss.register_function(boost::shared_ptr<Proxy_Function>(new Dynamic_Proxy_Function(boost::bind(&dynamic_function, boost::ref(ss), "concat_string", _1))), "concat_string");


  // Call our newly defined dynamic function with 10 parameters, then send
  // its output to the "print" function
  dispatch(ss.get_function("print_string"),
      Param_List_Builder() << dispatch(ss.get_function("concat_string"),
        Param_List_Builder() << std::string("\n\t") << std::string("The Value Was: ") 
                             << double(42.5) << std::string(".")
                             << '\n'
                             << '\t' << std::string("The old value was: ")
                             << addresult << '.' << '\n' ));




  //Register some local methods of the "Test" class
  ss.register_function(build_constructor<Test, const std::string &>(), "Test");

  register_function(ss, &Test::get_message, "get_message");
  register_function(ss, &Test::show_message, "show_message");
  register_member(ss, &Test::number, "number");

  //Create a new object using the "Test" constructor, passing the param "Yo".
  //Then, add the new object to the system with the name "testobj2"
  ss.add_object("testobj2", 
      dispatch(ss.get_function("Test"), Param_List_Builder() << std::string("Yo")));

  // Look up and store a reference to our new object
  std::vector<Boxed_Value> sos;
  sos.push_back(ss.get_object("testobj2"));

  //Build a bound function proxy for calling the script handled function
  boost::function<void (Test &)> show_message = 
    build_function_caller<void (Test &)>(ss.get_function("show_message"));

  Test &t = boxed_cast<Test &>(ss.get_object("testobj2"));

  //Print the message the object was created with
  show_message(t);

  //Now, get a reference to the object's stored message
  Boxed_Value stringref = dispatch(ss.get_function("get_message"), sos);

  //Unbox it using boxed_cast
  std::string &sr = boxed_cast<std::string &>(stringref);

  //Update the value of the reference
  sr = "Bob Updated The message";

  //Now, get a reference to the object's stored number
  Boxed_Value numberref= dispatch(ss.get_function("number"), sos);

  //Unbox it using boxed_cast
  int &ir = boxed_cast<int &>(numberref);

  std::cout << "Number: " << ir << std::endl;

  //Now, prove that the reference was successfully acquired
  //and we are able to peek into the boxed types
  show_message(t);


  // Finally, we are going to register some named function aliases, for 
  // the fun of it
  ss.register_function(boost::shared_ptr<Proxy_Function>(
        new Dynamic_Proxy_Function(boost::bind(&named_func_call, boost::ref(ss), "+", _1))), "add");

  //Call our newly named "add" function (which in turn dispatches +)
  
  std::cout << "Result of add function: " << 
    boxed_cast<int>(dispatch(ss.get_function("add"), Param_List_Builder() << 5 << 2))
    << std::endl; 


  ss.set_object("myfunc", boost::shared_ptr<Proxy_Function>(new Proxy_Function_Impl<boost::function<void (const std::string &)> >(&test)));

  dispatch(ss.get_function("myfunc"), Param_List_Builder() << std::string("hello function variable"));

}

