// This file is distributed under the BSD License.
// See LICENSE.TXT for details.
// Copyright 2009, Jonathan Turner (jonathan.d.turner@gmail.com) 
// and Jason Turner (lefticus@gmail.com)
// http://www.chaiscript.com

#ifndef CHAISCRIPT_PRELUDE_HPP_
#define CHAISCRIPT_PRELUDE_HPP_


//Note, the expression "[x,y]" in "collate" is parsed as two separate expressions
//by C++, so CODE_STRING, takes two expressions and adds in the missing comma
#define CODE_STRING(x, y) #x ", " #y

#define chaiscript_prelude CODE_STRING(\
# to_string for Pair()\n\
def to_string(x) : call_exists(first, x) && call_exists(second, x) { \
  "<" + x.first.to_string() + ", " + x.second.to_string() + ">"; \
}\
# to_string for containers\n\
def to_string(x) : call_exists(range, x) && !x.is_type("string"){ \
  "[" + x.join(", ") + "]"; \
}\
# Basic to_string function\n\
def to_string(x) { \
  return internal_to_string(x); \
}\
# Prints to console with no carriage return\n\
def puts(x) { \
  print_string(x.to_string()); \
} \
# Prints to console with carriage return\n\
def print(x) { \
  println_string(x.to_string()); \
} \
# Returns the maximum value of two numbers\n\
def max(a, b) { if (a>b) { a } else { b } } \
# Returns the minimum value of two numbers\n\
def min(a, b) { if (a<b) { a } else { b } } \
# Returns true if the value is odd\n\
def odd(x)  { if (x % 2 == 1) { true } else { false } } \
# Returns true if the value is even\n\
def even(x) { if (x % 2 == 0) { true } else { false } } \
# Pushes the second value onto the container first value while making a clone of the value\n\
def push_back(container, x) { container.push_back_ref(clone(x)) } \n\
# Inserts the third value at the position of the second value into the container of the first\n\
# while making a clone. \n\
def insert_at(container, pos, x) { container.insert_ref_at(pos, clone(x)); } \n\
# Performs the second value function over the container first value\n\
def for_each(container, func) : call_exists(range, container) { \
  var range = range(container); \
  while (!range.empty()) { \
    func(range.front()); \
    range.pop_front(); \
  } \
} \
def back_inserter(container) { \
  return bind(push_back, container, _);     \
}\
\
def map(container, func, inserter) : call_exists(range, container) { \
  var range = range(container); \
  while (!range.empty()) { \
    inserter(func(range.front())); \
    range.pop_front(); \
  } \
} \
# Performs the second value function over the container first value. Creates a new Vector with the results\n\
def map(container, func) { \
  var retval = Vector();\
  map(container, func, back_inserter(retval));\
  return retval;\
}\
# Performs the second value function over the container first value. Starts with initial and continues with each element.\n\
def foldl(container, func, initial) : call_exists(range, container){ \
  var retval = initial; \
  var range = range(container); \
  while (!range.empty()) { \
    retval = (func(range.front(), retval)); \
    range.pop_front(); \
  } \
  retval; \
} \
# Returns the sum of the elements of the given value\n\
def sum(container) { foldl(container, `+`, 0.0) } \
# Returns the product of the elements of the given value\n\
def product(container) { foldl(container, `*`, 1.0) } \
# Returns a new Vector with the elements of the first value concatenated with the elements of the second value\n\
def concat(x, y) : call_exists(clone, x) { \
  var retval = x; \
  var len = y.size(); \
  var i = 0; \
  while (i < len) { \
    retval.push_back(y[i]); \
    ++i; \
  } \
  retval; \
} \
def take(container, num, inserter) : call_exists(range, container) { \
  var r = range(container); \
  var i = num; \
  while ((i > 0) && (!r.empty())) { \
    inserter(r.front()); \
    r.pop_front(); \
    --i; \
  } \
} \
# Returns a new Vector with the given number of elements taken from the container\n\
def take(container, num) {\
  var retval = Vector(); \
  take(container, num, back_inserter(retval)); \
  return retval; \
}\
def take_while(container, f, inserter) : call_exists(range, container) { \
  var r = range(container); \
  while ((!r.empty()) && f(r.front())) { \
    inserter(r.front()); \
    r.pop_front(); \
  } \
} \
# Returns a new Vector with the given elements match the second value function\n\
def take_while(container, f) {\
  var retval = Vector(); \
  take_while(container, f, back_inserter(retval)); \
  return retval;\
}\
def drop(container, num, inserter) : call_exists(range, container) { \
  var r = range(container); \
  var i = num; \
  while ((i > 0) && (!r.empty())) { \
    r.pop_front(); \
    --i; \
  } \
  while (!r.empty()) { \
    inserter(r.front()); \
    r.pop_front(); \
  } \
} \
# Returns a new Vector with the given number of elements dropped from the given container \n\
def drop(container, num) {\
  var retval = Vector(); \
  drop(container, num, back_inserter(retval)); \
  return retval; \
}\
def drop_while(container, f, inserter) : call_exists(range, container) { \
  var r = range(container); \
  while ((!r.empty())&& f(r.front())) { \
    r.pop_front(); \
  } \
  while (!r.empty()) { \
    inserter(r.front()); \
    r.pop_front(); \
  } \
} \
# Returns a new Vector with the given elements dropped that match the second value function\n\
def drop_while(container, f) {\
  var retval = Vector(); \
  drop_while(container, f, back_inserter(retval)); \
  return retval; \
}\
# Applies the second value function to the container. Starts with the first two elements. Expects at least 2 elements.\n\
def reduce(container, func) : container.size() >= 2 && call_exists(range, container) { \
  var r = range(container); \
  var retval = r.front(); \
  r.pop_front(); \
  retval = func(retval, r.front()); \
  r.pop_front(); \
  while (!r.empty()) { \
    retval = func(retval, r.front()); \
    r.pop_front(); \
  } \
  retval; \
} \
# Returns a string of the elements in container delimited by the second value string\n\
def join(container, delim) { \
  var retval = ""; \
  var range = range(container); \
  if (!range.empty()) { \
    retval += to_string(range.front()); \
    range.pop_front(); \
    while (!range.empty()) { \
      retval += delim; \
      retval += to_string(range.front()); \
      range.pop_front(); \
    } \
  } \
  retval; \
} \
def filter(container, f, inserter) : call_exists(range, container) { \
  var r = range(container); \
  while (!r.empty()) { \
    if (f(r.front())) { \
      inserter(r.front()); \
    } \
    r.pop_front(); \
  } \
} \
# Returns a new Vector which match the second value function\n\
def filter(container, f) { \
  var retval = Vector(); \
  filter(container, f, back_inserter(retval));\
  return retval;\
}\
def generate_range(x, y, inserter) { \
  var i = x; \
  while (i <= y) { \
    inserter(i); \
    ++i; \
  } \
} \
# Returns a new Vector which represents the range from the first value to the second value\n\
def generate_range(x, y) { \
  var retval = Vector(); \
  generate_range(x,y,back_inserter(retval)); \
  return retval; \
}\
# Returns a new Vector with the first value to the second value as its elements\n\
def collate(x, y) { \
  [x, y]; \
} \
def zip_with(f, x, y, inserter) : call_exists(range, x) && call_exists(range, y) { \
  var r_x = range(x); \
  var r_y = range(y); \
  while (!r_x.empty() && !r_y.empty()) { \
    inserter(f(r_x.front(), r_y.front())); \
    r_x.pop_front(); \
    r_y.pop_front(); \
  } \
} \
# Returns a new Vector which joins matching elements of the second and third value with the first value function\n\
def zip_with(f, x, y) { \
  var retval = Vector(); \
  zip_with(f,x,y,back_inserter(retval)); \
  return retval;\
}\
# Returns a new Vector which joins matching elements of the first and second\n\
def zip(x, y) { \
  zip_with(collate, x, y); \
}\
# Returns the position of the second value string in the first value string\n\
def find(str, substr) { \
  return int(find(str, substr, size_t(0))); \
} \
# Returns the position of last match of the second value string in the first value string\n\
def rfind(str, substr) { \
  return int(rfind(str, substr, size_t(-1))); \
} \
# Returns the position of the first match of elements in the second value string in the first value string\n\
def find_first_of(str, list) { \
  return int(find_first_of(str, list, size_t(0))); \
} \
# Returns the position of the last match of elements in the second value string in the first value string\n\
def find_last_of(str, list) { \
  return int(find_last_of(str, list, size_t(-1))); \
} \
# Returns the position of the first non-matching element in the second value string in the first value string\n\
def find_first_not_of(str, list) { \
  return int(find_first_not_of(str, list, size_t(0))); \
} \
# Returns the position of the last non-matching element in the second value string in the first value string\n\
def find_last_not_of(str, list) { \
  return int(find_last_not_of(str, list, size_t(-1))); \
} \
)

#endif /* CHAISCRIPT_PRELUDE_HPP_ */
