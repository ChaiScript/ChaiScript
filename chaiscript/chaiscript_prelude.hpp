// This file is distributed under the BSD License.
// See LICENSE.TXT for details.

#ifndef CHAISCRIPT_PRELUDE_HPP_
#define CHAISCRIPT_PRELUDE_HPP_

const char *chaiscript_prelude = " \
def to_string(x) : call_exists(first, x) && call_exists(second, x) { \n\
  \"<\" + x.first.to_string() + \", \" + x.second.to_string() + \">\"\n\
}\n\
def to_string(x) : call_exists(range, x) { \n\
  \"[\" + x.join(\", \") + \"]\"\n\
}\n\
def to_string(x) { \n\
  return internal_to_string(x)\n\
}\n\
def puts(x) { \n\
  print_string(x.to_string()) \n\
}; \n\
def print(x) { \n\
  println_string(x.to_string()) \n\
}; \n\
def for_each(container, func) : call_exists(range, container) { \n\
  var range = range(container); \n\
  while (!range.empty()) { \n\
    func(range.front()) \n\
    range.pop_front() \n\
  } \n\
} \n\
def map(container, func) : call_exists(range, container) { \n\
  var retval = Vector() \n\
  var range = range(container) \n\
  while (!range.empty()) { \n\
    retval.push_back(func(range.front())) \n\
    range.pop_front() \n\
  } \n\
  retval \n\
} \n\
def foldl(container, func, initial) : call_exists(range, container){ \n\
  var retval = initial \n\
  var range = range(container) \n\
  while (!range.empty()) { \n\
    retval = (func(range.front(), retval)) \n\
    range.pop_front() \n\
  } \n\
  retval \n\
} \n\
def sum(x) { foldl(x, `+`, 0) } \n\
def product(x) { foldl(x, `*`, 1) } \n\
def take(container, num) : call_exists(range, container) { \n\
  var r = range(container); \n\
  var i = num; \n\
  var retval = Vector(); \n\
  while ((i > 0) && (!r.empty())) { \n\
    retval.push_back(r.front()); \n\
    r.pop_front(); \n\
    --i; \n\
  } \n\
  retval \n\
} \n\
def take_while(container, f) : call_exists(range, container) { \n\
  var r = range(container); \n\
  var retval = Vector(); \n\
  while ((!r.empty()) && f(r.front())) { \n\
    retval.push_back(r.front()); \n\
    r.pop_front(); \n\
  } \n\
  retval \n\
} \n\
def drop(container, num) : call_exists(range, container) { \n\
  var r = range(container); \n\
  var i = num; \n\
  var retval = Vector(); \n\
  while ((i > 0) && (!r.empty())) { \n\
    r.pop_front(); \n\
    --i; \n\
  } \n\
  while (!r.empty()) { \n\
    retval.push_back(r.front()); \n\
    r.pop_front(); \n\
  } \n\
  retval \n\
} \n\
def drop_while(container, f) : call_exists(range, container) { \n\
  var r = range(container); \n\
  var retval = Vector(); \n\
  while ((!r.empty())&& f(r.front())) { \n\
    r.pop_front(); \n\
  } \n\
  while (!r.empty()) { \n\
    retval.push_back(r.front()); \n\
    r.pop_front(); \n\
  } \n\
  retval \n\
} \n\
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
  retval \n\
} \n\
def max(a, b) { if (a>b) { a } else { b } } \n\
def min(a, b) { if (a<b) { a } else { b } } \n\
def join(container, delim) { \n\
  var retval = \"\" \n\
  var range = range(container) \n\
  if (!range.empty()) { \n\
    retval += to_string(range.front()) \n\
    range.pop_front() \n\
    while (!range.empty()) { \n\
      retval += delim \n\
      retval += to_string(range.front()) \n\
      range.pop_front() \n\
    } \n\
  } \n\
  retval \n\
} \n\
def filter(a, f) : call_exists(range, a) { \n\
  var retval = Vector(); \n\
  var r = range(a); \n\
  while (!r.empty()) { \n\
    if (f(r.front())) { \n\
      retval.push_back(r.front()); \n\
    } \n\
    r.pop_front(); \n\
  } \n\
  retval \n\
} \n\
def collate(x, y) { \n\
  [x, y] \n\
} \n\
def zip_with(f, x, y) : call_exists(range, x) && call_exists(range, y) { \n\
  var r_x = range(x); \n\
  var r_y = range(y); \n\
  var retval = Vector(); \n\
  while (!r_x.empty() && !r_y.empty()) { \n\
    retval.push_back(f(r_x.front(), r_y.front())); \n\
    r_x.pop_front(); \n\
    r_y.pop_front(); \n\
  } \n\
  retval \n\
} \n\
def zip(x, y) { \n\
  zip_with(collate, x, y) \n\
}\n";

#endif /* CHAISCRIPT_PRELUDE_HPP_ */
