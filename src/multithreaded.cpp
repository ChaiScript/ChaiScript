// This file is distributed under the BSD License.
// See "license.txt" for details.
// Copyright 2009-2010, Jonathan Turner (jonathan@emptycrate.com)
// and Jason Turner (jason@emptycrate.com)
// http://www.chaiscript.com

#include <iostream>

#include <list>

#include <chaiscript/chaiscript.hpp>
#include <thread>




void do_work(chaiscript::ChaiScript &c)
{
//  c("use(\"work.chai\"); do_chai_work(num_iterations);"); 
  std::stringstream ss;
  ss << "MyVar" << rand();
  c.add(chaiscript::var(5), ss.str());
  c("use(\"work.chai\"); do_chai_work(10000);");
}

int main(int argc, char *argv[]) {
  std::string input;
  chaiscript::ChaiScript chai;

  //chai.add_shared_object(chaiscript::Boxed_Value(10000), "num_iterations");

  std::vector<std::shared_ptr<std::thread> > threads;

  for (int i = 0; i < argc - 1; ++i)
  {
//    std::thread t(&do_work, std::ref(chai));
    threads.push_back(std::shared_ptr<std::thread>(new std::thread(&do_work, std::ref(chai))));
  }

  for (int i = 0; i < argc - 1; ++i)
  {
    threads[i]->join();
  }
}

