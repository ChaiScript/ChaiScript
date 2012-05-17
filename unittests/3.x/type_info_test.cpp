// Tests to make sure that the order in which function dispatches occur is correct

#include <chaiscript/dispatchkit/type_info.hpp>

void test_type(const chaiscript::Type_Info &ti, bool t_is_const, bool t_is_pointer, bool t_is_reference, bool t_is_void,
    bool t_is_undef)
{
  if (ti.is_const() == t_is_const
      && ti.is_pointer() == t_is_pointer
      && ti.is_reference() == t_is_reference
      && ti.is_void() == t_is_void
      && ti.is_undef() == t_is_undef)
  {
    return;
  } else {
    exit(EXIT_FAILURE);
  }
}


int main()
{
  test_type(chaiscript::user_type<void>(), false, false, false, true, false);
  test_type(chaiscript::user_type<const int>(), true, false, false, false, false);
  test_type(chaiscript::user_type<const int &>(), true, false, true, false, false);
  test_type(chaiscript::user_type<int>(), false, false, false, false, false);
  test_type(chaiscript::user_type<int *>(), false, true, false, false, false);
  test_type(chaiscript::user_type<const int *>(), true, true, false, false, false);
  test_type(chaiscript::Type_Info(), false, false, false, false, true);

  return EXIT_SUCCESS;
}
