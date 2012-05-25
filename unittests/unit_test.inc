def assert_equal(x, y)
{
  if (x == y)
  {
    // Passes
  } else {
    // Fails
    print("assert_equal failure: got '" + to_string(y) + "' expected '" + to_string(x) + "'");
    exit(-1);
  }
}

def assert_false(f)
{
  if (f)
  {
    print("assert_false failure");
    exit(-1);
  }
}

def assert_true(f)
{
  if (!f)
  {
    print("assert_true failure");
    exit(-1);
  }
}

def assert_not_equal(x, y)
{
  if (!(x == y))
  {
    // Passes
  } else {
    // Fails
    print("assert_not_equal failure: got " + to_string(y) + " which was not expected.");
    exit(-1);
  }
}

def assert_throws(desc, x)
{
  if (throws_exception(x))
  {
    // Passes
  } else {
    // Fails
    print("assert_throws failure, function did not throw exception: " + to_string(desc));
    exit(-1);
  }
}
