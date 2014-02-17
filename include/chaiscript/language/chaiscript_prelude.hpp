// This file is distributed under the BSD License.
// See "license.txt" for details.
// Copyright 2009-2012, Jonathan Turner (jonathan@emptycrate.com)
// Copyright 2009-2014, Jason Turner (jason@emptycrate.com)
// http://www.chaiscript.com

#ifndef CHAISCRIPT_PRELUDE_HPP_
#define CHAISCRIPT_PRELUDE_HPP_

//Note, the expression "[x,y]" in "collate" is parsed as two separate expressions
//by C++, so CODE_STRING, takes two expressions and adds in the missing comma
#define CODE_STRING(x, y) #x ", " #y 

#define chaiscript_prelude CODE_STRING(\
def lt(l, r) { if (call_exists(`<`, l, r)) { l < r } else { type_name(l) < type_name(r) } } \n\
def gt(l, r) { if (call_exists(`>`, l, r)) { l > r } else { type_name(l) > type_name(r) } } \n\
def eq(l, r) { if (call_exists(`==`, l, r)) { l == r } else { false } } \n\
def new(x) { eval(type_name(x))(); } \n\
def clone(x) : function_exists(type_name(x)) && call_exists(eval(type_name(x)), x)  { eval(type_name(x))(x); } \n\
# to_string for Pair()\n\
def to_string(x) : call_exists(first, x) && call_exists(second, x) { \n\
  "<" + x.first.to_string() + ", " + x.second.to_string() + ">"; \n\
}\n\
# to_string for containers\n\
def to_string(x) : call_exists(range, x) && !x.is_type("string"){ \n\
  "[" + x.join(", ") + "]"; \n\
}\n\
# Basic to_string function\n\
def to_string(x) { \n\
  internal_to_string(x); \n\
}\n\
# Prints to console with no carriage return\n\
def puts(x) { \n\
  print_string(x.to_string()); \n\
} \n\
# Prints to console with carriage return\n\
def print(x) { \n\
  println_string(x.to_string()); \n\
} \n\
# Returns the maximum value of two numbers\n\
def max(a, b) { if (a>b) { a } else { b } } \n\
# Returns the minimum value of two numbers\n\
def min(a, b) { if (a<b) { a } else { b } } \n\
# Returns true if the value is odd\n\
def odd(x)  { if (x % 2 == 1) { true } else { false } } \n\
# Returns true if the value is even\n\
def even(x) { if (x % 2 == 0) { true } else { false } } \n\
# Pushes the second value onto the container first value while making a clone of the value\n\
def push_back(container, x) : call_exists(push_back_ref, container, x) { container.push_back_ref(clone(x)) } \n\
# Pushes the second value onto the front of the container first value while making a clone of the value\n\
def push_front(container, x) : call_exists(push_front_ref, container, x) { container.push_front_ref(clone(x)) } \n\
# Inserts the third value at the position of the second value into the container of the first\n\
# while making a clone. \n\
def insert_at(container, pos, x) { container.insert_ref_at(pos, clone(x)); } \n\
# Returns the reverse of the given container\n\
def reverse(container) {\n\
  var retval = new(container); \n\
  var r = range(container); \n\
  while (!r.empty()) { \n\
    retval.push_back(r.back()); \n\
    r.pop_back(); \n\
  } \n\
  retval; \n\
} \n\
# Return a range from a range \n\
def range(r) : call_exists(empty, r) && call_exists(pop_front, r) && call_exists(pop_back, r) && call_exists(back, r) && call_exists(front, r) { return clone(r); }\n\
# The retro attribute that contains the underlying range \n\
attr retro::m_range; \n\
# Creates a retro from a retro by returning the original range\n\
def retro(r) : call_exists(get_type_name, r) && get_type_name(r) == "retro" { clone(r.m_range) }\n\
# Creates a retro range from a range\n\
def retro::retro(r) : call_exists(empty, r) && call_exists(pop_front, r) && call_exists(pop_back, r) && call_exists(back, r) && call_exists(front, r) { this.m_range = r; }\n\
# Returns the first value of a retro\n\
def retro::front()  { back(this.m_range) }\n\
# Returns the last value of a retro\n\
def retro::back()  { front(this.m_range) }\n\
# Moves the back iterator of a retro towards the front by one \n\
def retro::pop_back()  { pop_front(this.m_range) }\n\
# Moves the front iterator of a retro towards the back by one \n\
def retro::pop_front()  { pop_back(this.m_range) } \n\
# returns true if the retro is out of elements \n\
def retro::empty() { empty(this.m_range); } \n\
# Performs the second value function over the container first value\n\
def for_each(container, func) : call_exists(range, container) { \n\
  var t_range = range(container); \n\
  while (!t_range.empty()) { \n\
    func(t_range.front()); \n\
    t_range.pop_front(); \n\
  } \n\
} \n\
def back_inserter(container) { \n\
  bind(push_back, container, _);     \n\
}\n\
\n\
def contains(container, item, compare_func) : call_exists(range, container) { \n\
  var t_range = range(container); \n\
  while (!t_range.empty()) { \n\
    if ( compare_func(t_range.front(), item) ) { return true; } \n\
    t_range.pop_front(); \n\
  } \n\
  return false; \n\
} \n\
def contains(container, item) { return contains(container, item, eq) } \n\
def map(container, func, inserter) : call_exists(range, container) { \n\
  var range = range(container); \n\
  while (!range.empty()) { \n\
    inserter(func(range.front())); \n\
    range.pop_front(); \n\
  } \n\
} \n\
# Performs the second value function over the container first value. Creates a new container with the results\n\
def map(container, func) { \n\
  var retval = new(container); \n\
  map(container, func, back_inserter(retval));\n\
  retval;\n\
}\n\
# Performs the second value function over the container first value. Starts with initial and continues with each element.\n\
def foldl(container, func, initial) : call_exists(range, container){ \n\
  var retval = initial; \n\
  var range = range(container); \n\
  while (!range.empty()) { \n\
    retval = (func(range.front(), retval)); \n\
    range.pop_front(); \n\
  } \n\
  retval; \n\
} \n\
# Returns the sum of the elements of the given value\n\
def sum(container) { foldl(container, `+`, 0.0) } \n\
# Returns the product of the elements of the given value\n\
def product(container) { foldl(container, `*`, 1.0) } \n\
# Returns a new container with the elements of the first value concatenated with the elements of the second value\n\
def concat(x, y) : call_exists(clone, x) { \n\
  var retval = x; \n\
  var inserter = back_inserter(retval); \n\
  var range = range(y); \n\
  while (!range.empty()) { \n\
    inserter(range.front()); \n\
    range.pop_front(); \n\
  } \n\
  retval; \n\
} \n\
def take(container, num, inserter) : call_exists(range, container) { \n\
  var r = range(container); \n\
  var i = num; \n\
  while ((i > 0) && (!r.empty())) { \n\
    inserter(r.front()); \n\
    r.pop_front(); \n\
    --i; \n\
  } \n\
} \n\
# Returns a new container with the given number of elements taken from the container\n\
def take(container, num) {\n\
  var retval = new(container); \n\
  take(container, num, back_inserter(retval)); \n\
  retval; \n\
}\n\
def take_while(container, f, inserter) : call_exists(range, container) { \n\
  var r = range(container); \n\
  while ((!r.empty()) && f(r.front())) { \n\
    inserter(r.front()); \n\
    r.pop_front(); \n\
  } \n\
} \n\
# Returns a new container with the given elements match the second value function\n\
def take_while(container, f) {\n\
  var retval = new(container); \n\
  take_while(container, f, back_inserter(retval)); \n\
  retval;\n\
}\n\
def drop(container, num, inserter) : call_exists(range, container) { \n\
  var r = range(container); \n\
  var i = num; \n\
  while ((i > 0) && (!r.empty())) { \n\
    r.pop_front(); \n\
    --i; \n\
  } \n\
  while (!r.empty()) { \n\
    inserter(r.front()); \n\
    r.pop_front(); \n\
  } \n\
} \n\
# Returns a new container with the given number of elements dropped from the given container \n\
def drop(container, num) {\n\
  var retval = new(container); \n\
  drop(container, num, back_inserter(retval)); \n\
  retval; \n\
}\n\
def drop_while(container, f, inserter) : call_exists(range, container) { \n\
  var r = range(container); \n\
  while ((!r.empty())&& f(r.front())) { \n\
    r.pop_front(); \n\
  } \n\
  while (!r.empty()) { \n\
    inserter(r.front()); \n\
    r.pop_front(); \n\
  } \n\
} \n\
# Returns a new container with the given elements dropped that match the second value function\n\
def drop_while(container, f) {\n\
  var retval = new(container); \n\
  drop_while(container, f, back_inserter(retval)); \n\
  retval; \n\
}\n\
# Applies the second value function to the container. Starts with the first two elements. Expects at least 2 elements.\n\
def reduce(container, func) : container.size() >= 2 && call_exists(range, container) { \n\
  var r = range(container); \n\
  var retval = r.front(); \n\
  r.pop_front(); \n\
  retval = func(retval, r.front()); \n\
  r.pop_front(); \n\
  while (!r.empty()) { \n\
    retval = func(retval, r.front()); \n\
    r.pop_front(); \n\
  } \n\
  retval; \n\
} \n\
# Returns a string of the elements in container delimited by the second value string\n\
def join(container, delim) { \n\
  var retval = ""; \n\
  var range = range(container); \n\
  if (!range.empty()) { \n\
    retval += to_string(range.front()); \n\
    range.pop_front(); \n\
    while (!range.empty()) { \n\
      retval += delim; \n\
      retval += to_string(range.front()); \n\
      range.pop_front(); \n\
    } \n\
  } \n\
  retval; \n\
} \n\
def filter(container, f, inserter) : call_exists(range, container) { \n\
  var r = range(container); \n\
  while (!r.empty()) { \n\
    if (f(r.front())) { \n\
      inserter(r.front()); \n\
    } \n\
    r.pop_front(); \n\
  } \n\
} \n\
# Returns a new Vector which match the second value function\n\
def filter(container, f) { \n\
  var retval = new(container); \n\
  filter(container, f, back_inserter(retval));\n\
  retval;\n\
}\n\
def generate_range(x, y, inserter) { \n\
  var i = x; \n\
  while (i <= y) { \n\
    inserter(i); \n\
    ++i; \n\
  } \n\
} \n\
# Returns a new Vector which represents the range from the first value to the second value\n\
def generate_range(x, y) { \n\
  var retval = Vector(); \n\
  generate_range(x,y,back_inserter(retval)); \n\
  retval; \n\
}\n\
# Returns a new Vector with the first value to the second value as its elements\n\
def collate(x, y) { \n\
  [x, y]; \n\
} \n\
def zip_with(f, x, y, inserter) : call_exists(range, x) && call_exists(range, y) { \n\
  var r_x = range(x); \n\
  var r_y = range(y); \n\
  while (!r_x.empty() && !r_y.empty()) { \n\
    inserter(f(r_x.front(), r_y.front())); \n\
    r_x.pop_front(); \n\
    r_y.pop_front(); \n\
  } \n\
} \n\
# Returns a new Vector which joins matching elements of the second and third value with the first value function\n\
def zip_with(f, x, y) { \n\
  var retval = Vector(); \n\
  zip_with(f,x,y,back_inserter(retval)); \n\
  retval;\n\
}\n\
# Returns a new Vector which joins matching elements of the first and second\n\
def zip(x, y) { \n\
  zip_with(collate, x, y); \n\
}\n\
# Returns the position of the second value string in the first value string\n\
def string::find(substr) : is_type(substr, "string") { \n\
  int(find(this, substr, 0)); \n\
} \n\
# Returns the position of last match of the second value string in the first value string\n\
def string::rfind(substr) : is_type(substr, "string") { \n\
  int(rfind(this, substr, -1)); \n\
} \n\
# Returns the position of the first match of elements in the second value string in the first value string\n\
def string::find_first_of(list) : is_type(list, "string") { \n\
  int(find_first_of(this, list, 0)); \n\
} \n\
# Returns the position of the last match of elements in the second value string in the first value string\n\
def string::find_last_of(list) : is_type(list, "string") { \n\
  int(find_last_of(this, list, -1)); \n\
} \n\
# Returns the position of the first non-matching element in the second value string in the first value string\n\
def string::find_first_not_of(list) : is_type(list, "string") { \n\
  int(find_first_not_of(this, list, 0)); \n\
} \n\
# Returns the position of the last non-matching element in the second value string in the first value string\n\
def string::find_last_not_of(list) : is_type(list, "string") { \n\
  int(find_last_not_of(this, list, -1)); \n\
} \n\
def string::ltrim() { \n\
  drop_while(this, fun(x) { x == ' ' || x == '\t' || x == '\r' || x == '\n'}); \n\
} \n\
def string::rtrim() { \n\
  reverse(drop_while(reverse(this), fun(x) { x == ' ' || x == '\t' || x == '\r' || x == '\n'})); \n\
} \n\
def string::trim() { \n\
  ltrim(rtrim(this)); \n\
} \n\
def find(container, value, compare_func) : call_exists(range, container) && is_type(compare_func, "Function") { \n\
  var range = range(container); \n\
  while (!range.empty()) { \n\
    if (compare_func(range.front(), value)) { \n\
      return range; \n\
    } else { \n\
      range.pop_front(); \n\
    } \n\
  } \n\
  return range; \n\
} \n\
def find(container, value) { return find(container, value, eq) } \
)
#endif /* CHAISCRIPT_PRELUDE_HPP_ */
