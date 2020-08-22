# ChaiScript Versioning

ChaiScript tries to follow the [Semantic Versioning](http://semver.org/) scheme. This basically means:

  * Major Version Number: API changes / breaking changes
  * Minor Version Number: New Features
  * Patch Version Number: Minor changes / enhancements



# Initializing ChaiScript

```
chaiscript::ChaiScript chai; // initializes ChaiScript, adding the standard ChaiScript types (map, string, ...)
```

Note that ChaiScript cannot be used as a global / static object unless it is being compiled with `CHAISCRIPT_NO_THREADS`.

# Adding Things To The Engine

## Adding a Function / Method / Member

### General

```cpp
chai.add(chaiscript::fun(&function_name), "function_name");
chai.add(chaiscript::fun(&Class::method_name), "method_name");
chai.add(chaiscript::fun(&Class::member_name), "member_name");
```

### Bound Member Functions

```cpp
chai.add(chaiscript::fun(&Class::method_name, Class_instance_ptr), "method_name");
chai.add(chaiscript::fun(&Class::member_name, Class_instance_ptr), "member_name");
```

### With Overloads

#### Preferred

```cpp
chai.add(chaiscript::fun<ReturnType (ParamType1, ParamType2)>(&function_with_overloads), "function_name");
```

#### Alternative

```cpp
chai.add(chaiscript::fun(std::static_cast<ReturnType (*)(ParamType1, ParamType2)>(&function_with_overloads)), "function_name");
```
This overload technique is also used when exposing base member using derived type

```cpp
struct Base
{
  int data;
};

struct Derived : public Base
{};

chai.add(chaiscript::fun(static_cast<int(Derived::*)>(&Derived::data)), "data");
```

### Lambda

```cpp
chai.add(
  chaiscript::fun<std::function<std::string (bool)>>(
    [](bool type) {
      if (type) { return "x"; }
      else { return "y"; }
    }), "function_name");
```

### Constructors

```cpp
chai.add(chaiscript::constructor<MyType ()>(), "MyType");
chai.add(chaiscript::constructor<MyType (const MyType &)>(), "MyType");
```

## Adding Types

It's not strictly necessary to add types, but it helps with many things. Cloning, better errors, etc.

```cpp
chai.add(chaiscript::user_type<MyClass>(), "MyClass");
```

## Adding Type Conversions

User defined type conversions are possible, defined in either script or in C++.



### ChaiScript Defined Conversions

Function objects (including lambdas) can be used to add type conversions
from inside of ChaiScript:

```
add_type_conversion(type("string"), type("Type_Info"), fun(s) { return type(s); });
```

### C++ Defined Conversions

Invoking a C++ type conversion possible with `static_cast`

```cpp
chai.add(chaiscript::type_conversion<T, bool>());
```

Calling a user defined type conversion that takes a lambda

```cpp
chai.add(chaiscript::type_conversion<TestBaseType, Type2>([](const TestBaseType &t_bt) { /* return converted thing */ }));
```

### Class Hierarchies

If you want objects to be convertable between base and derived classes, you must tell ChaiScript about the relationship.

```cpp
chai.add(chaiscript::base_class<Base, Derived>());
```

If you have multiple classes in your inheritance graph, you will probably want to tell ChaiScript about all relationships.

```cpp
chai.add(chaiscript::base_class<Base, Derived>());
chai.add(chaiscript::base_class<Derived, MoreDerived>());
chai.add(chaiscript::base_class<Base, MoreDerived>());
```

### Helpers

A helper function exists for strongly typed and ChaiScript `Vector` function conversion definition:

```
chai.add(chaiscript::vector_conversion<std::vector<int>>());
```

A helper function also exists for strongly typed and ChaiScript `Map` function conversion definition:

```
chai.add(chaiscript::map_conversion<std::map<std::string, int>>());
```



This allows you to pass a ChaiScript function to a function requiring `std::vector<int>`

## Adding Objects

```
chai.add(chaiscript::var(somevar), "somevar"); // copied in
chai.add(chaiscript::var(std::ref(somevar), "somevar"); // by reference, shared between C++ and chai
auto shareddouble = std::make_shared<double>(4.3);
chai.add(chaiscript::var(shareddouble), "shareddouble"); // by shared_ptr, shared between c++ and chai
chai.add(chaiscript::const_var(somevar), "somevar"); // copied in and made const
chai.add_global_const(chaiscript::const_var(somevar), "somevar"); // global const. Throws if value is non-const, throws if object exists
chai.add_global(chaiscript::var(somevar), "somevar"); // global non-const, throws if object exists
chai.set_global(chaiscript::var(somevar), "somevar"); // global non-const, overwrites existing object
```

## Adding Namespaces

Namespaces will not be populated until `import` is called.
This saves memory and computing costs if a namespace is not imported into every ChaiScript instance.

```cpp
chai.register_namespace([](chaiscript::Namespace& math) {
    math["pi"] = chaiscript::const_var(3.14159);
    math["sin"] = chaiscript::var(chaiscript::fun([](const double x) { return sin(x); })); },
    "math");
```

Import namespace in ChaiScript
```
import("math")
print(math.pi) // prints 3.14159
```

# Using STL
ChaiScript recognize many types from STL, but you have to add specific instantiation yourself.

```cpp
typedef std::vector<std::pair<int, std::string>> data_list;
data_list my_list{ make_pair(0, "Hello"), make_pair(1, "World") };
chai.add(chaiscript::bootstrap::standard_library::vector_type<data_list>("DataList"));
chai.add(chaiscript::bootstrap::standard_library::pair_type<data_list::value_type>("DataElement"));
chai.add(chaiscript::var(&my_list), "data_list");
chai.eval(R"_(
    for(var i=0; i<data_list.size(); ++i)
    {
      print(to_string(data_list[i].first) + " " + data_list[i].second)
    }
  )_");
```

# Executing Script

## General

```cpp
chai.eval("print(\"Hello World\")");
chai.eval(R"(print("Hello World"))");
```

## Unboxing Return Values

Returns values are of the type `Boxed_Value` which is meant to be opaque to the programmer. Use one of the unboxing methods to access the internal data.

### Prefered

```cpp
chai.eval<double>("5.3 + 2.1"); // returns 7.4 as a C++ double
```

### Alternative

```cpp
auto v = chai.eval("5.3 + 2.1");
chai.boxed_cast<double>(v); // extracts double value from boxed_value and applies known conversions
chaiscript::boxed_cast<double>(v); // free function version, does not know about conversions
```

### Converting Between Algebraic Types

```cpp
chaiscript::Boxed_Number(chai.eval("5.3 + 2.1")).get_as<int>(); // works with any number type
// which is equivalent to, but much more automatic than:
static_cast<int>(chai.eval<double>("5.3+2.1")); // this version only works if we know that it's a double
```

### Conversion Caveats

Conversion to `std::shared_ptr<T> &` is supported for function calls, but if you attempt to keep a reference to a `shared_ptr<>` you might invoke undefined behavior

```cpp
// ok this is supported, you can register it with chaiscript engine
void nullify_shared_ptr(std::shared_ptr<int> &t) {
  t = nullptr
}
```

```cpp
int main()
{
  // do some stuff and create a chaiscript instance
  std::shared_ptr<int> &ptr = chai.eval<std::shared_ptr<int> &>(somevalue);
  // DO NOT do this. Taking a non-const reference to a shared_ptr is not 
  // supported and causes undefined behavior in the chaiscript engine
}
```


## Sharing Values

```cpp
double &d = chai.eval("var i = 5.2; i"); // d is now a reference to i in the script
std::shared_ptr<double> d = chai.eval("var i = 5.2; i"); // same result but reference counted

d = 3;
chai.eval("print(i)"); // prints 3
```

## Catching Eval Errors

```cpp
try {
  chai.eval("2.3 + \"String\"");
} catch (const chaiscript::exception::eval_error &e) {
  std::cout << "Error\n" << e.pretty_print() << '\n';
}
```

## Catching Errors Thrown From Script

```cpp
try {
  chai.eval("throw(runtime_error(\"error\"))", chaiscript::exception_specification<int, double, float, const std::string &, const std::exception &>());
} catch (const double e) {
} catch (int) {
} catch (float) {
} catch (const std::string &) {
} catch (const std::exception &e) {
  // This is the one what will be called in the specific throw() above
}
```

## Sharing Functions


```cpp
auto p = chai.eval<std::function<std::string (double)>>("to_string"); 
p(5); // calls chaiscript's 'to_string' function, returning std::string("5")
```

Note: backtick treats operators as normal functions

```cpp
auto p = chai.eval<std::function<int (int, int)>>(`+`); 
p(5, 6); // calls chaiscript's '+' function, returning 11
```

```cpp
auto p = chai.eval<std::function<std::string (int, double)>>("fun(x,y) { to_string(x) + to_string(y); }");
p(3,4.2); // evaluates the lambda function, returning the string "34.2" to C++
```

# Language Reference

## Variables

```
var i; // uninitialized variable, can take any value on first assignment;
auto j; // equiv to var

var k = 5; // initialized to 5 (integer)
var l := k; // reference to k
auto &m = k; // reference to k

global g = 5; // creates a global variable. If global already exists, it is not re-added
global g = 2; // global 'g' now equals 2

global g2;
if (g2.is_var_undef()) { g2 = 4; } // only initialize g2 once, if global decl hit more than once

GLOBAL g3; // all upper case version also accepted
```

## Looping

```
// c-style for loops
for (var i = 0; i < 100; ++i) { print(i); }
```

```
// while
while (some_condition()) { /* do something */ }
```

```
// ranged for
for (x : [1,2,3]) { print(i); }
```

Each of the loop styles can be broken using the `break` statement. For example:

```
while (some_condition()) {
  /* do something */
  if (another_condition()) { break; }
}
```

## Conditionals

```
if (expression) { }
```

```
// C++17-style init-if blocks
// Value of 'statement' is scoped for entire `if` block
if (statement; expression) { }
```

## Switch Statements

``` chaiscript
var myvalue = 2
switch (myvalue) {
    case (1) {
        print("My Value is 1");
        break;
    }
    case (2) {
        print("My Value is 2");
        break;
    }
    default {
        print("My Value is something else.";
    }
}
```

## Built in Types

There are a number of build-in types that are part of ChaiScript.

### Vectors and Maps

```
var v = [1,2,3u,4ll,"16", `+`]; // creates vector of heterogenous values
var m = ["a":1, "b":2]; // map of string:value pairs

// Add a value to the vector by value.
v.push_back(123);

// Add an object to the vector by reference.
v.push_back_ref(m);
```

### Numbers

Floating point values default to `double` type and integers default to `int` type. All C++ suffixes
such as `f`, `ll`, `u` as well as scientific notation are supported

```
1.0 // double
1.0f // float
1.0l // long double
1 // int
1u // unsigned int
1ul // unsigned long
1ull // unsigned long long
```

Literals are automatically sized, just as in C++. For example: `10000000000` is > 32bits and the appropriate type is used to hold it
on your platform.


## Functions

Note that any type of ChaiScript function can be passed freely to C++ and automatically
converted into an `std::function` object.

### General 

```
def myfun(x, y) { x + y; } // last statement in body is the return value
def myfun(x, y) { return x + y; } // equiv 
```

### Optionally Typed

```
def myfun(x, int y) { x + y; } // requires y to be an int
```

### With Guards

```
def myfun(x, int y) : y > 5 { x - y; } // only called if y > 5
```

### Methods

Methods and functions are mostly equivalent

```
def string::add(int y) { this + to_string(y); }
def add(string s, int y) { s + to_string(y); } //equiv functionality

// calling new function/method
"a".add(1); // returns a1
add("a", 1); // returns a1, either calling syntax works with either def above
```

### Lambdas

```
var l = fun(x) { x * 15; }
l(2) // returns 30

var a = 13
var m = fun[a](x) { x * a; } 
m(3); // a was captured (by reference), returns 39

var n = bind(fun(x,y) { x * y; }, _, 10);
n(2); // returns 20 
```



## ChaiScript Defined Types

Define a type called "MyType" with one member value "a" and a getter

### Preferred

```
class MyType {
  var value;
  def MyType() { this.value = "a"; }
  def get_value() { "Value Is: " + this.value; }
};
```

### Alternative 

```
attr MyType::value;
def MyType::MyType() { this.value = "a"; }
def MyType::get_value() { "Value Is: " + this.value; }
```

### Using

```
var m = MyType(); // calls constructor
print(m.get_value()); // prints "Value Is: a"
print(get_value(m)); // prints "Value Is: a"
```

## Dynamic Objects

All ChaiScript defined types and generic Dynamic_Object support dynamic parameters

```
var o = Dynamic_Object();
o.f = fun(x) { print(x); }
o.f(3); // prints "3"
```

Implicit 'this' is allowed:

```
var o = Dynamic_Object();
o.x = 3;
o.f = fun(y) { print(this.x + y); }
o.f(10); // prints 13
```

## Namespaces

Namespaces in ChaiScript are Dynamic Objects with global scope

```
namespace("math") // create a new namespace

math.square = fun(x) { x * x } // add a function to the "math" namespace
math.sum_squares = fun(x, y) { math.square(x) + math.square(y) }

print(math.square(4)) // prints 16
print(math.sum_squares(2, 5)) // prints 29
```

### Option Explicit

If you want to disable dynamic parameter definitions, you can `set_explicit`.

```
class My_Class {
  def My_Class() {
    this.set_explicit(true);
    this.x = 2; // this would fail with explicit set to true
  }
};
```

## method_missing

A function of the signature `method_missing(object, name, param1, param2, param3)` will be called if an appropriate
method cannot be found

```
def method_missing(int i, string name, Vector v) {
  print("method_missing(${i}, ${name}), ${v.size()} params");
}

5.bob(1,2,3); // prints "method_missing(5, bob, 3 params)"
```

`method_missing` signature can be either 2 parameters or 3 parameters. If the signature contains two parameters
it is treated as a property. If the property contains a function then additional parameters are passed to
the contained function.

If both a 2 parameter and a 3 parameter signature match, the 3 parameter function always wins.

## Context

 * `__LINE__` Current file line number
 * `__FILE__` Full path of current file
 * `__CLASS__` Name of current class
 * `__FUNC__` Name of current function


# Built In Functions

## Evaluation

```
eval("4 + 5") // dynamically eval script string and returns value of last statement
eval_file("filename") // evals file and returns value of last statement
use("filename") // evals file exactly once and returns value of last statement
                // if the file had already been 'used' nothing happens and undefined is returned
```

Both `use` and `eval_file` search the 'usepaths' passed to the ChaiScript constructor

## JSON

 * `from_json` converts a JSON string into its strongly typed (map, vector, int, double, string) representations
 * `to_json` converts a ChaiScript object (either a `Object` or one of map, vector, int, double, string) tree into its JSON string representation
 
## Extras
ChaiScript itself does not provide a link to the math functions defined in `<cmath>`. You can either add them yourself, or use the [ChaiScript_Extras](https://github.com/ChaiScript/ChaiScript_Extras) helper library. (Which also provides some additional string functions.)
