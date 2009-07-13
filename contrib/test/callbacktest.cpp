// This file is distributed under the BSD License.
// See "license.txt" for details.
// Copyright 2009, Jonathan Turner (jturner@minnow-lang.org)
// and Jason Turner (lefticus@gmail.com)
// http://www.chaiscript.com

#include <iostream>

#include "function_call.hpp"
#include "chaiscript.hpp"
#include <boost/function.hpp>

struct Callback_Handler
{
  typedef std::vector<std::pair<boost::function<std::string ()>,
                        boost::function<double (int)> > > Callbacks;
  
  Callbacks m_callbacks;

  void add_callbacks(boost::shared_ptr<dispatchkit::Proxy_Function> t_name,
      boost::shared_ptr<dispatchkit::Proxy_Function> t_value)
  {
    m_callbacks.push_back(
        std::make_pair(dispatchkit::build_function_caller<std::string ()>(t_name),
                       dispatchkit::build_function_caller<double (int)>(t_value)
          )
        );
  }

  void do_callbacks()
  {
    int i=1;
    for (Callbacks::iterator itr = m_callbacks.begin();
         itr != m_callbacks.end();
         ++itr)
    {
      std::cout << "Name: " << itr->first() << "  =  " << itr->second(i) << std::endl;
      ++i;
    }
  }
};

int main(int argc, char *argv[]) {

    chaiscript::ChaiScript_Engine chai;

    Callback_Handler cb_handler;
    chai.get_eval_engine().add_object("cb_handler", boost::ref(cb_handler));
    dispatchkit::register_function(chai.get_eval_engine(), &Callback_Handler::add_callbacks, "add_callbacks");

    for (int i = 1; i < argc; ++i) {
      try {
        dispatchkit::Boxed_Value val = chai.evaluate_file(argv[i]);
      }
      catch (std::exception &e) {
        std::cerr << "Could not open: " << argv[i] << std::endl;
        exit(1);
      }
    }

    cb_handler.do_callbacks();

    boost::function<std::string (const std::string&, const std::string &)> f = 
      dispatchkit::build_functor<std::string (const std::string &, const std::string &)>
        (chai, "function(x, y) { return x + y }");

    std::cout << "Functor call: " << f("Hello", " World") << std::endl;
}

