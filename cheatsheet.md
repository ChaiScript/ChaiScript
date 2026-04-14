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

## Engine Options (`Options`)

Engine-level options control which scripting capabilities are exposed. These are passed as a `std::vector<Options>` to the `ChaiScript` or `ChaiScript_Basic` constructor.

| Option | Effect |
|--------|--------|
| `Options::Load_Modules` | Enables `load_module()` in scripts (default) |
| `Options::No_Load_Modules` | Disables `load_module()` |
| `Options::External_Scripts` | Enables `use()` and `eval_file()` in scripts (default) |
| `Options::No_External_Scripts` | Disables `use()` and `eval_file()` |

```cpp
// Sandboxed engine: no dynamic module loading, no external script evaluation
chaiscript::ChaiScript chai({}, {},
    {chaiscript::Options::No_Load_Modules, chaiscript::Options::No_External_Scripts});
```

## Library Options (`Library_Options`)

Library-level options control which parts of the standard library are registered. These are passed as a `std::vector<Library_Options>`.

| Option | Effect |
|--------|--------|
| `Library_Options::No_Stdlib` | Disables the entire standard library (types, I/O, prelude, JSON — everything) |
| `Library_Options::No_IO` | Disables `print_string` and `println_string` (and the prelude's `print`/`puts` wrappers) |
| `Library_Options::No_Prelude` | Disables the ChaiScript prelude (`print`, `puts`, `filter`, `map`, `foldl`, `join`, etc.) |
| `Library_Options::No_JSON` | Disables `from_json` and `to_json` |

With the `ChaiScript` convenience class, pass library options as the fourth constructor parameter:

```cpp
// No I/O functions
chaiscript::ChaiScript chai({}, {}, chaiscript::default_options(),
    {chaiscript::Library_Options::No_IO});

// No JSON support
chaiscript::ChaiScript chai({}, {}, chaiscript::default_options(),
    {chaiscript::Library_Options::No_JSON});

// Completely bare engine — no stdlib at all
chaiscript::ChaiScript chai({}, {}, chaiscript::default_options(),
    {chaiscript::Library_Options::No_Stdlib});

// Combine both: no external scripts and no I/O
chaiscript::ChaiScript chai({}, {},
    {chaiscript::Options::No_Load_Modules, chaiscript::Options::No_External_Scripts},
    {chaiscript::Library_Options::No_IO});
```

With `ChaiScript_Basic`, pass library options directly to `Std_Lib::library()`:

```cpp
chaiscript::ChaiScript_Basic chai(
    chaiscript::Std_Lib::library({chaiscript::Library_Options::No_IO}),
    create_chaiscript_parser(),
    {}, {},
    {chaiscript::Options::No_Load_Modules, chaiscript::Options::No_External_Scripts});
```

Note: `No_Prelude` disables the prelude script which defines convenience functions like `print` (which wraps `print_string`). If you disable the prelude but not I/O, `print_string` and `println_string` are still available.

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
chai.add(chaiscript::fun(static_cast<ReturnType (*)(ParamType1, ParamType2)>(&function_with_overloads)), "function_name");
```
This overload technique is also used when exposing base members using derived type

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

User-defined type conversions are possible, defined in either script or in C++.



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

Calling a user-defined type conversion that takes a lambda

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

### `add` — Thread-Local Scoped Variables

`add` adds an object to the current thread's local scope. The variable is only visible in the
thread that added it. If the variable already exists in the current scope, it is overwritten.

```cpp
chai.add(chaiscript::var(somevar), "somevar");                    // copied in
chai.add(chaiscript::var(std::ref(somevar)), "somevar");          // by reference, shared between C++ and chai
auto shareddouble = std::make_shared<double>(4.3);
chai.add(chaiscript::var(shareddouble), "shareddouble");          // by shared_ptr, shared between C++ and chai
chai.add(chaiscript::const_var(somevar), "somevar");              // copied in and made const
```

### `add_global` / `add_global_const` / `set_global` — Global Shared Variables

Global variables are shared between all threads and are visible from any scope (including inside
functions). Use these when you need a variable accessible everywhere.

```cpp
chai.add_global_const(chaiscript::const_var(somevar), "somevar"); // global const, throws if value is non-const or object already exists
chai.add_global(chaiscript::var(somevar), "somevar");             // global non-const, throws if object already exists
chai.set_global(chaiscript::var(somevar), "somevar");             // global non-const, overwrites existing or creates new
```

### Summary of Differences

| Method | Scope | Thread Safety | If Name Exists |
|--------|-------|---------------|----------------|
| `add` | Thread-local, current scope | Not shared between threads | Overwrites |
| `add_global` | Global, all scopes and threads | Mutex-protected, shared between threads | Throws exception |
| `add_global_const` | Global, all scopes and threads | Mutex-protected, shared between threads | Throws exception |
| `set_global` | Global, all scopes and threads | Mutex-protected, shared between threads | Overwrites |

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
ChaiScript recognizes many types from STL, but you have to add specific instantiation yourself.

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
  // This is the one that will be called in the specific throw() above
}
```

## Sharing Functions


```cpp
auto p = chai.eval<std::function<std::string (double)>>("to_string"); 
p(5); // calls chaiscript's 'to_string' function, returning std::string("5")
```

Note: backtick treats operators as normal functions

```cpp
auto p = chai.eval<std::function<int (int, int)>>("`+`"); 
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
for (i : [1, 2, 3]) { print(i); }
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

## Built-in Types

There are a number of built-in types that are part of ChaiScript.

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
converted into a `std::function` object.

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



## ChaiScript Defined Types (Classes)

ChaiScript supports user-defined types using the `class` keyword. Classes can have attributes,
constructors, methods, guards, and operator overloads. There is no inheritance between
ChaiScript-defined types, but C++ class hierarchies can be exposed (see *Class Hierarchies* above).

### Class Definition (Block Syntax)

Define a type with attributes, a constructor, and methods inside a `class` block.
The keywords `var`, `attr`, and `auto` are interchangeable for declaring attributes.

```
class Rectangle {
  var width
  var height
  def Rectangle(w, h) { this.width = w; this.height = h; }
  def Rectangle() { this.width = 0; this.height = 0; }
  def area() { this.width * this.height; }
}

var r = Rectangle(3, 4)
print(r.area()) // prints 12
```

### Class Definition (Open Syntax)

Equivalently, attributes and methods can be defined outside a block using the `TypeName::` prefix.

```
attr Circle::radius
def Circle::Circle(r) { this.radius = r; }
def Circle::circumference() { 2.0 * 3.14159 * this.radius; }
```

Methods can also be added to an existing class after its initial definition:

```
def Rectangle::perimeter() { 2 * (this.width + this.height); }
```

### Using

```
var m = Rectangle(5, 10)
print(m.area())       // prints 50 — method call syntax
print(area(m))        // prints 50 — function call syntax (equivalent)
```

### Constructor and Method Guards

Constructors and methods can have guard expressions (after `:`) that control which
overload is selected at call time.

```
class Clamped {
  var value
  def Clamped(x) : x >= 0 { this.value = x; }
  def Clamped(x) { this.value = 0; }  // fallback when guard fails
}

Clamped(5).value   // 5
Clamped(-3).value  // 0

class Abs {
  var x
  def Abs(v) { this.x = v; }
  def get() : this.x >= 0 { this.x; }
  def get() { -this.x; }
}
```

### Operator Overloading

Operators can be overloaded on user-defined types using backtick-quoted operator names.

```
class Vec2 {
  var x
  var y
  def Vec2(x, y) { this.x = x; this.y = y; }
  def `+`(other) { Vec2(this.x + other.x, this.y + other.y); }
}

var v = Vec2(1, 2) + Vec2(3, 4)  // v.x == 4, v.y == 6
```

Operators can also be overloaded as free functions with guards:

```
def `-`(a, b) : is_type(a, "Vec2") && is_type(b, "Vec2") {
  Vec2(a.x - b.x, a.y - b.y)
}
```

### Cloning Objects

Use `clone()` to create a deep copy of a ChaiScript-defined object.

```
var original = Rectangle(10, 20)
var copy = clone(original)
copy.width = 99
print(original.width)  // still 10
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

## Strong Typedefs

Strong typedefs create distinct types that are not interchangeable with their underlying type
or with other typedefs of the same underlying type. They use `Dynamic_Object` internally and
automatically expose operators that the underlying type supports.

### Basic Usage

```
using Meters = int
using Seconds = int

var d = Meters(100)
var t = Seconds(10)

// d and t are distinct types — you cannot accidentally mix them
// Meters + Seconds would require an explicit conversion
```

### Arithmetic and Comparison

Operators from the underlying type are forwarded and remain strongly typed:

```
using Meters = int

var a = Meters(10)
var b = Meters(20)

var c = a + b        // Meters(30) — result is still Meters
var bigger = b > a   // true — comparisons return bool

// Compound assignment operators work too
a += b               // a is now Meters(30)
```

### String-Based Strong Typedefs

Strong typedefs work with any type, not just numeric types:

```
using Name = string

var n = Name("Alice")
var greeting = Name("Hello, ") + Name("world")  // Name — string concatenation is forwarded
```

### Accessing the Underlying Value

Use `to_underlying` to extract the wrapped value:

```
using Meters = int

var d = Meters(42)
var raw = to_underlying(d)  // 42, plain int
```

### Extending Strong Typedefs

You can add custom operations to strong typedefs just like any other ChaiScript type:

```
using Meters = int
using Seconds = int
using MetersPerSecond = int

def speed(Meters d, Seconds t) {
  MetersPerSecond(to_underlying(d) / to_underlying(t))
}

var s = speed(Meters(100), Seconds(10))  // MetersPerSecond(10)
```

You can also overload operators between different strong typedefs:

```
using Meters = int
using Feet = int

def to_feet(Meters m) {
  Feet((to_underlying(m) * 328) / 100)
}

var m = Meters(10)
var f = to_feet(m)  // Feet(32)
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


# Built-in Functions

## Evaluation

```
eval("4 + 5") // dynamically eval script string and returns value of last statement
eval_file("filename") // evals file and returns value of last statement
use("filename") // evals file exactly once and returns value of last statement
                // if the file had already been 'used' nothing happens and undefined is returned
```

Both `use` and `eval_file` search the 'usepaths' passed to the ChaiScript constructor

## Reflection and Introspection

ChaiScript provides built-in reflection capabilities for inspecting types, functions, and objects at runtime.

### Type Inspection

```
type_name(x)            // returns the type name of a value as a string
is_type(x, "typename")  // returns true if x is of the named type
type("typename")        // returns a Type_Info object for the named type

// Examples
type_name(1)            // "int"
type_name("hello")      // "string"
is_type(1, "int")       // true
is_type(1, "string")    // false
```

### Object Inspection Methods

Every object in ChaiScript supports these methods:

```
x.get_type_info()     // returns a Type_Info object for the value
x.is_type("string")   // returns true if x is of the named type
x.is_type(string_type) // returns true if x matches the Type_Info
x.is_var_const()      // returns true if x is immutable
x.is_var_null()       // returns true if x is a null pointer
x.is_var_pointer()    // returns true if x is stored as a pointer
x.is_var_reference()  // returns true if x is stored as a reference
x.is_var_undef()      // returns true if x is undefined
```

### Type_Info

`Type_Info` objects describe a type. You can get them via `type("typename")` or `x.get_type_info()`.

```
var ti = type("int")
ti.name()              // ChaiScript registered name, e.g. "int"
ti.cpp_name()          // mangled C++ type name
ti.cpp_bare_name()     // C++ name without const/pointer/reference
ti.bare_equal(other)   // true if types match ignoring const/ptr/ref
ti.is_type_const()     // true if type is const
ti.is_type_reference() // true if type is a reference
ti.is_type_void()      // true if type is void
ti.is_type_undef()     // true if type is undefined
ti.is_type_pointer()   // true if type is a pointer
ti.is_type_arithmetic() // true if type is arithmetic (int, double, etc.)
```

Built-in type constants are available: `int_type`, `double_type`, `string_type`, `bool_type`, `Object_type`, `Function_type`, `vector_type`, `map_type`.

### Function Introspection

Function objects support these introspection methods:

```
f.get_arity()                // number of parameters (-1 for variadic)
f.get_param_types()          // Vector of Type_Info (first element is return type)
f.get_contained_functions()  // Vector of overloaded functions (empty if not a conglomerate)
f.has_guard()                // true if the function has a guard condition
f.get_guard()                // returns the guard function (throws if none)
f.get_annotation()           // returns the annotation description
f.call([param1, param2])     // call the function with a vector of parameters

// Examples
def my_func(a, b) { return a + b; }
my_func.get_arity()          // 2
my_func.has_guard()          // false

def guarded(x) : x > 0 { return x; }
guarded.has_guard()          // true
guarded.get_guard().get_arity() // 1

// Calling functions dynamically
`+`.call([1, 2])             // 3
```

### System Introspection

```
get_functions()        // returns a Map of all registered functions (name -> function)
get_objects()          // returns a Map of all scripting objects (name -> value)
function_exists("f")   // returns true if a function named "f" is registered
call_exists(`f`, args) // returns true if f can be called with the given args
dump_system()          // prints all registered functions to stdout
dump_object(x)         // prints information about a value to stdout

// Examples
var funcs = get_functions()
funcs["print"]                  // the print function object
function_exists("print")        // true
call_exists(`+`, 1, 2)          // true
```

### Dynamic_Object Reflection

ChaiScript-defined classes are Dynamic_Objects internally. They support:

```
obj.get_type_name()    // returns the ChaiScript class name (e.g. "MyClass")
obj.get_attrs()        // returns a Map of all attributes
obj.has_attr("name")   // returns true if the attribute exists
obj.get_attr("name")   // returns the value of the attribute
obj.set_explicit(true) // disables dynamic attribute creation
obj.is_explicit()      // returns true if explicit mode is enabled

// Example
class MyClass {
  var x
  def MyClass() { this.x = 10; }
}
var m = MyClass()
m.get_type_name()      // "MyClass"
m.get_attrs()          // map containing "x" -> 10
type_name(m)           // "Dynamic_Object" (the underlying C++ type)
m.is_type("MyClass")   // true (checks the ChaiScript class name)
```

## JSON

 * `from_json` converts a JSON string into its strongly typed (map, vector, int, double, string) representations
 * `to_json` converts a ChaiScript object (either a `Object` or one of map, vector, int, double, string) tree into its JSON string representation
 
## IO Redirection

By default, ChaiScript's `print()` and `puts()` functions write to stdout. You can redirect
output on a per-instance basis by setting a single print handler. Both `println_string`
(used by `print()`) and `print_string` (used by `puts()`) dispatch through the same handler —
`println_string` simply appends a newline before calling it.

```cpp
chaiscript::ChaiScript chai;

// Redirect all output (print_string and println_string both use this handler)
chai.set_print_handler([](const std::string &s) {
  my_log_window.append(s);
});
```

This is useful for embedding ChaiScript in GUI applications, logging frameworks, or any
context where stdout is not the desired output destination.

```cpp
// Example: capture all output to a string
std::string captured;
chai.set_print_handler([&captured](const std::string &s) {
  captured += s;
});

chai.eval("print(42)");    // captured == "42\n"
chai.eval("puts(\"hi\")"); // captured == "42\nhi"
```

The print handler can also be set from within ChaiScript itself via `set_print_handler`:

```chaiscript
// Redirect output from within a script
set_print_handler(fun(s) { my_custom_log(s) })
```

## Extras
ChaiScript itself does not provide a link to the math functions defined in `<cmath>`. You can either add them yourself, or use the [ChaiScript_Extras](https://github.com/ChaiScript/ChaiScript_Extras) helper library. (Which also provides some additional string functions.)

## Grammar Railroad Diagrams

A formal EBNF grammar for ChaiScript is available in [`grammar/chaiscript.ebnf`](grammar/chaiscript.ebnf). You can visualize it as navigable railroad diagrams by pasting its contents into one of these tools:

  * [rr — Railroad Diagram Generator (IPv6)](https://www.bottlecaps.de/rr/ui)
  * [rr — Railroad Diagram Generator (IPv4)](https://rr.red-dove.com/ui)

Open either link, switch to the **Edit Grammar** tab, paste the file contents, then click **View Diagram**.
