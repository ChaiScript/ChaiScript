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
  auto result = trim(throws_exception(x));
  if (result == desc)
  {
    // Passes
  } else {
    // Fails
    print("assert_throws failed: got '${result}' expected '${desc}'");
    exit(-1);
  }
}

