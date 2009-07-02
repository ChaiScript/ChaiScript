// This file is distributed under the BSD License.
// See LICENSE.TXT for details.

#ifndef CHAISCRIPT_PRELUDE_HPP_
#define CHAISCRIPT_PRELUDE_HPP_

const char *chaiscript_prelude = " \n\
def to_string(x) : call_exists(first, x) && call_exists(second, x) { \n\
    \"<\" + x.first.to_string() + \", \" + x.second.to_string() + \">\"\n\
}\n\
def to_string(x) : call_exists(range, x) { \n\
    \"[\" + x.join(\", \") + \"]\"\n\
}\n\
def to_string(x) { \n\
    x.internal_to_string()\n\
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
}";

#endif /* CHAISCRIPT_PRELUDE_HPP_ */
