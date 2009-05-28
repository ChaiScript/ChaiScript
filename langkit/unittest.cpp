#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE boxedcpp_unittests
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE( add_operators )
{
  BOOST_CHECK_EQUAL(2, 2);
}
