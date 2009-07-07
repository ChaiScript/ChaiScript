// This file is distributed under the BSD License.
// See LICENSE.TXT for details.

#ifndef CHAISCRIPT_PRELUDE_HPP_
#define CHAISCRIPT_PRELUDE_HPP_


//Note, the expression "[x,y]" in "collate" is parsed as two separate expressions
//by C++, so CODE_STRING, takes two expressions and adds in the missing comma
#define CODE_STRING(x, y) #x ", " #y

#define chaiscript_prelude CODE_STRING(\
def to_string(x) : call_exists(first, x) && call_exists(second, x) { \
  "<" + x.first.to_string() + ", " + x.second.to_string() + ">"; \
} \
def to_string(x) : call_exists(range, x) && !x.is_type("string"){ \
  "[" + x.join(", ") + "]"; \
} \
def to_string(x) { \
  return internal_to_string(x); \
} \
def puts(x) { \
  print_string(x.to_string()); \
} \
def print(x) { \
  println_string(x.to_string()); \
} \
def max(a, b) { if (a>b) { a } else { b } } \
def min(a, b) { if (a<b) { a } else { b } } \
def odd(x)  { if (x % 2 == 1) { true } else { false } } \
def even(x) { if (x % 2 == 0) { true } else { false } } \
def push_back(container, x) { container.push_back_ref(clone(x)) } \
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
def map(container, func) { \
  var retval = Vector();\
  map(container, func, back_inserter(retval));\
  return retval;\
}\
def foldl(container, func, initial) : call_exists(range, container){ \
  var retval = initial; \
  var range = range(container); \
  while (!range.empty()) { \
    retval = (func(range.front(), retval)); \
    range.pop_front(); \
  } \
  retval; \
} \
def sum(container) { foldl(container, `+`, 0.0) } \
def product(container) { foldl(container, `*`, 1.0) } \
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
def drop_while(container, f) {\
  var retval = Vector(); \
  drop_while(container, f, back_inserter(retval)); \
  return retval; \
}\
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
def generate_range(x, y) { \
  var retval = Vector(); \
  generate_range(x,y,back_inserter(retval)); \
  return retval; \
}\
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
def zip_with(f, x, y) { \
  var retval = Vector(); \
  zip_with(f,x,y,back_inserter(retval)); \
  return retval;\
}\
def zip(x, y) { \
  zip_with(collate, x, y); \
}\
)

#endif /* CHAISCRIPT_PRELUDE_HPP_ */
