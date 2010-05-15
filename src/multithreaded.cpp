// This file is distributed under the BSD License.
// See "license.txt" for details.
// Copyright 2009-2010, Jonathan Turner (jturner@minnow-lang.org)
// and Jason Turner (lefticus@gmail.com)
// http://www.chaiscript.com

#include <iostream>

#include <list>

#include <chaiscript/chaiscript.hpp>
#include <boost/thread.hpp>

void do_work(chaiscript::ChaiScript &c)
{
//  c("use(\"work.chai\"); do_chai_work(num_iterations);");
  c("use(\"work.chai\"); do_chai_work(10000);");
}

int main(int argc, char *argv[]) {
    std::string input;
    chaiscript::ChaiScript chai;

    //chai.add_shared_object(chaiscript::Boxed_Value(10000), "num_iterations");

    std::vector<boost::shared_ptr<boost::thread> > threads;

    for (int i = 0; i < argc - 1; ++i)
    {
      threads.push_back(boost::shared_ptr<boost::thread>(new boost::thread(boost::bind(do_work, boost::ref(chai)))));
    }

    for (int i = 0; i < argc - 1; ++i)
    {
      threads[i]->join();
    }
}

