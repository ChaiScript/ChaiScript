def assert_equal(x, y)
{
  if (x == y)
  {
    // Passes
  } else {
    // Fails
    print("assert_equal failure: got " + to_string(y) + " expected " + to_string(x));
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
