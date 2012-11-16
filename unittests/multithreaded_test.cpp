// This file is distributed under the BSD License.
// See "license.txt" for details.
// Copyright 2009-2010, Jonathan Turner (jonathan@emptycrate.com)
// and Jason Turner (jason@emptycrate.com)
// http://www.chaiscript.com

#include <iostream>

#include <list>
#include <algorithm>

#include <chaiscript/chaiscript.hpp>
#include <boost/thread.hpp>

int expected_value(int num_iters)
{
  int i = 0;
  for (int k = 0; k<num_iters * 10; ++k)
  {
    i += k;
  }

  return i;
}

void do_work(chaiscript::ChaiScript &c, int id)
{
  std::stringstream ss;
  ss << "MyVar" << rand();
  c.add(chaiscript::var(5), ss.str());
  ss.str("");
  ss << id;
  c.use("multithreaded_work.inc");
  c("do_chai_work(4000, " + ss.str() + ");");
}

int main()
{
  // Disable deprecation warning for getenv call.
#ifdef BOOST_MSVC
#pragma warning(push)
#pragma warning(disable : 4996)
#endif

  const char *usepath = getenv("CHAI_USE_PATH");
  const char *modulepath = getenv("CHAI_MODULE_PATH");

#ifdef BOOST_MSVC
#pragma warning(pop)
#endif

  std::vector<std::string> usepaths;
  usepaths.push_back("");
  if (usepath)
  {
    usepaths.push_back(usepath);
  }

  std::vector<std::string> modulepaths;
  modulepaths.push_back("");
  if (modulepath)
  {
    modulepaths.push_back(modulepath);
  }

  chaiscript::ChaiScript chai(modulepaths,usepaths);

  std::vector<boost::shared_ptr<boost::thread> > threads;

  // Ensure at least two, but say only 7 on an 8 core processor
  int num_threads = std::max<unsigned int>(boost::thread::hardware_concurrency() - 1, 2u);

  for (int i = 0; i < num_threads; ++i)
  {
    threads.push_back(boost::shared_ptr<boost::thread>(new boost::thread(boost::bind(do_work, boost::ref(chai), i))));
  }

  for (int i = 0; i < num_threads; ++i)
  {
    threads[i]->join();
  }


  for (int i = 0; i < num_threads; ++i)
  {
    std::stringstream ss;
    ss << i;
    if (chai.eval<int>("getvalue(" + ss.str() + ")") != expected_value(4000))
    {
      return EXIT_FAILURE;
    }

    if (chai.eval<int>("getid(" + ss.str() + ")") != i)
    {
      return EXIT_FAILURE;
    }
  }

  return EXIT_SUCCESS;
}

