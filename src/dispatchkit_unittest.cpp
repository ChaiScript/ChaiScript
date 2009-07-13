// This file is distributed under the BSD License.
// See "license.txt" for details.
// Copyright 2009, Jonathan Turner (jturner@minnow-lang.org)
// and Jason Turner (lefticus@gmail.com)
// http://www.chaiscript.com

#include "dispatchkit.hpp"
#include "bootstrap.hpp"

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE boxedcpp_unittests
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE( add_operators )
{
  using namespace dispatchkit;

  Dispatch_Engine ss;
  Bootstrap::bootstrap(ss);
  dump_system(ss);

  BOOST_CHECK_EQUAL(boxed_cast<int>(dispatch(ss.get_function("+"), Param_List_Builder() << double(5.1) << double(10.3))), 15.4);
}
