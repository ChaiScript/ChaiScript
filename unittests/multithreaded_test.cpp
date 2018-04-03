#include <iostream>

#include <algorithm>

#ifdef CHAISCRIPT_NO_DYNLOAD
#include <chaiscript/chaiscript.hpp>
#endif
#include <chaiscript/chaiscript_basic.hpp>
#include <chaiscript/language/chaiscript_parser.hpp>

int expected_value(int num_iters)
{
  int i = 0;
  for (int k = 0; k<num_iters * 10; ++k)
  {
    i += k;
  }

  return i;
}

void do_work(chaiscript::ChaiScript_Basic &c, const size_t id)
{
  try{
    std::stringstream ss;
    ss << "MyVar" << rand();
    c.add(chaiscript::var(5), ss.str());
    ss.str("");
    ss << id;
    c.use("multithreaded_work.inc");
    c("do_chai_work(4000, " + ss.str() + ");");
  } catch (const std::exception &e) {
    std::cout << "exception: " << e.what() << " thread:  " << id;
  }
}

int main()
{
  // Disable deprecation warning for getenv call.
#ifdef CHAISCRIPT_MSVC
#ifdef max // Why Microsoft? why?
#undef max
#endif
#pragma warning(push)
#pragma warning(disable : 4996)
#endif

  const char *usepath = getenv("CHAI_USE_PATH");
  const char *modulepath = getenv("CHAI_MODULE_PATH");
  const char *this_might_affect_test_result = "This might have adverse effect on the result of the test.";
  
  if(usepath == nullptr)
  {
    std::cout << "Warning: environmental variable CHAI_USE_PATH not set! " << this_might_affect_test_result << "\n";
  }
  
  if(modulepath == nullptr)
  {
    std::cout << "Warning: environmental variable CHAI_MODULE_PATH not set! " << this_might_affect_test_result << "\n";
  }

#ifdef CHAISCRIPT_MSVC
#pragma warning(pop)
#endif

  std::vector<std::string> usepaths;
  usepaths.emplace_back("");
  if (usepath)
  {
    usepaths.emplace_back(usepath);
  }

  std::vector<std::string> modulepaths;

#ifdef CHAISCRIPT_NO_DYNLOAD
  chaiscript::ChaiScript chai(/* unused */modulepaths, usepaths);
#else
  modulepaths.emplace_back("");
  if (modulepath)
  {
    modulepaths.emplace_back(modulepath);
  }
  
  // For this test we are going to load the dynamic stdlib
  // to make sure it continues to work
  chaiscript::ChaiScript_Basic chai(
      std::make_unique<chaiscript::parser::ChaiScript_Parser<chaiscript::eval::Noop_Tracer, chaiscript::optimizer::Optimizer_Default>>(),
      modulepaths,usepaths);
#endif

  std::vector<std::shared_ptr<std::thread> > threads;

  // Ensure at least two, but say only 7 on an 8 core processor
  size_t num_threads = static_cast<size_t>(std::max(static_cast<int>(std::thread::hardware_concurrency()) - 1, 2));

  std::cout << "Num threads: " << num_threads << '\n';

  for (size_t i = 0; i < num_threads; ++i)
  {
    threads.push_back(std::make_shared<std::thread>(do_work, std::ref(chai), i));
  }

  for (size_t i = 0; i < num_threads; ++i)
  {
    threads[i]->join();
  }



  for (size_t i = 0; i < num_threads; ++i)
  {
    std::stringstream ss;
    ss << i;
    if (chai.eval<int>("getvalue(" + ss.str() + ")") != expected_value(4000))
    {
      return EXIT_FAILURE;
    }

    if (chai.eval<int>("getid(" + ss.str() + ")") != static_cast<int>(i))
    {
      return EXIT_FAILURE;
    }
  }

  return EXIT_SUCCESS;
}

