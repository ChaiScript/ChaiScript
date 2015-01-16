#include <chaiscript/chaiscript.hpp>
#include <chaiscript/chaiscript_stdlib.hpp>

template<typename T>
struct Vector2
{
  Vector2() : x(0), y(0) {}
  Vector2(T px, T py) : x(px), y(py) {}
  Vector2(const Vector2& cp) : x(cp.x), y(cp.y) {}

  Vector2& operator+=(const Vector2& vec_r)
  {
    x += vec_r.x;
    y += vec_r.y;
    return *this;
  }

  Vector2 operator+(const Vector2& vec_r)
  {
    return Vector2(*this += vec_r);
  }

  Vector2 &operator=(const Vector2& ver_r)
  {
    x = ver_r.x;
    y = ver_r.y;
    return *this;
  }


  T x;
  T y;
};

Vector2<float> GetValue()
{
  return Vector2<float>(10,15);
}

int main()
{
  chaiscript::ChaiScript _script(chaiscript::Std_Lib::library());

  //Registering stuff
  _script.add(chaiscript::user_type<Vector2<float>>(), "Vector2f");
  _script.add(chaiscript::constructor<Vector2<float> ()>(), "Vector2f");
  _script.add(chaiscript::constructor<Vector2<float> (float, float)>(), "Vector2f");
  _script.add(chaiscript::constructor<Vector2<float> (const Vector2<float>&)>(), "Vector2f");
  _script.add(chaiscript::fun(&Vector2<float>::x), "x");
  _script.add(chaiscript::fun(&Vector2<float>::y), "y");
  _script.add(chaiscript::fun(&Vector2<float>::operator +), "+");
  _script.add(chaiscript::fun(&Vector2<float>::operator +=), "+=");
  _script.add(chaiscript::fun(&Vector2<float>::operator =), "=");
  _script.add(chaiscript::fun(&GetValue), "getValue");

  _script.eval(R"(
    var test = 0.0
    var test2 = Vector2f(10,10)

    test = getValue().x
    print(test)
    print(test2.x)
    )");

  if (_script.eval<std::string>("to_string(test)") != "10") { return EXIT_FAILURE; }
  if (_script.eval<std::string>("to_string(test2.x)") != "10") { return EXIT_FAILURE; }


  //_script.eval_file("object_lifetime_test2.inc");
}
