#include "dispatchkit.hpp"
#include "bootstrap.hpp"

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE boxedcpp_unittests
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE( add_operators )
{
  Dispatch_Engine ss;
  Bootstrap::bootstrap(ss);
  dump_system(ss);

  BOOST_CHECK_EQUAL(Cast_Helper<int>()(dispatch(ss.get_function("+"), Param_List_Builder() << double(5.1) << double(10.3))), 15.4);
}
