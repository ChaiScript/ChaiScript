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
def for_each(container, func) { \n\
  var range = range(container); \n\
  while (!range.empty()) { \n\
    func(range.front()) \n\
    range.pop_front() \n\
  } \n\
} \n\
def map(container, func) { \n\
  var retval = Vector() \n\
  var range = range(container) \n\
  while (!range.empty()) { \n\
    retval.push_back(func(range.front())) \n\
    range.pop_front() \n\
  } \n\
  retval \n\
} \n\
def reduce(container, func, initial) { \n\
  var retval = initial \n\
  var range = range(container) \n\
  while (!range.empty()) { \n\
    retval = (func(range.front(), retval)) \n\
    range.pop_front() \n\
  } \n\
  retval \n\
} \n\
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
