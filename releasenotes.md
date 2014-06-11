Notes:
=======
Current Version: 5.3.1

### Changes since 5.3.0
* Add automatic conversion of arithmetic return types, following the same
  rules as conversion of arithmetic types when passing parameters
* Add automatic casting up the inheritence hierarchy when possible.
* Enable travis.ci testing
* Allow users to add globals from within script
* Various static analysis fixes
* Code modernization to C++11
* Unofficial support for Haiku added
* Fix #121 - Inability to compile on cygwin
* Formatting fixes and spelling corrections
* Apply "include what you use" https://code.google.com/p/include-what-you-use/
* Apply clang-modernize
* Various threading fixes
* Performance improvements

### Changes since 5.2.0
* Official support for MSVC with C++11. All major platforms and compilers are now support for C++11 release

### Changes since 4.2.0
* Enhanced unit tests
* Add `continue` statement, fix various use cases for `for`  loops
* Fix use of suffixed numbers in vector initialization
* Code cleanups
* Eliminate global data, which makes code more portable and thread safe
* Fix issue #79
* Merge pretty_print fixes from @mgee #82
* Compiler warning fixes for latest compiler releases
* Fix threading problems
* Fix linking error on MacOS Mavericks #88
* Allow non-const globals
* Make sure user cannot name a variable with `::` in it #91
* Fix various string / map / vector `size` and `count` calls for compilers which have weird overloads for them. #90 #93 #95
* Make module search path relative to the currently running executable
* Build and work with wstring windows builds
* fix for some new line cases in the middle of a vector initialization from jespada

### Changes since 5.1.0
* Add support for automatic conversion of arithmetic types when possible
  and when no ambiguous method dispatch exists.

### Changes since 5.0.0
* Fix sizing of numeric constants to match that of the C++ standard
* Add support for u,ll,l,f suffixes for numeric constants
* Siginificant improvement in error reporting

### Changes since 4.0.0
* Dropped boost in favor of C++11
* Separated out stdlib to make more options for compile time improvements

### Changes since 3.1.0
* svenstaro: Unused variables and CMake consistency fixes
* Added support for returning pointers from functions (#13)
* Compile with -pedantic (#9)
* Fix issues with multiple ChaiScript object types having the same attribute name (#15)
* Prevent variable redeclaration in same scope (#22)
* mgee: Boxed_Number improvements (#27)
* Support switch statements (#34)
* Fix uint16 comparions (#26)
* Add ability to add const_var globals in Module objects (#14)
* Add support for ternary operators ?:
* Add headers to CMakeLists so they show up in IDEs
* Add ability to get vector of defined objects and vector of defined functions
* Fix memory leak in cyclical references
* Clean up static analysis issues discovered
* Fix vector construction to be consistent with map construction
* Increased unit tests to 161
* Performance enhancements

### Changes since 3.0.0
* Numeric operations performance increased approximately 10x
* Looping operations performance increased up to 2x
* Engine start up time decreased
* Several parsing bugs related to index operators fixed
* Added full support for all C algebraic types: double, long double, float, int, long, char,
  uint8_t, uint16_t, uint32_t, uint64_t, int8_t, int16_t, int32_t, int64_t
* Enhanced support for capturing of exceptions thrown from ChaiScript in C++

### Changes since 2.3.3
* Code simplifications
* Fully integrate documentation with source code in doxygen style comments
* Unit tests increased from 114 to 137
* Automatic conversion between boost::function objects and ChaiScript functions 
* Many bug fixes
* Minor performance improvements
* Namespace reorganization to make end user code more accessible
* clang support
* VisualStudio 2010 Support
* Support for C++ base classes and automatic upcasting 
* Remove __ reserved identifiers
* Better code organization to reduce #ifdefs 
* clanmills: command line options for chai eval
* clanmills: parser cleanups and code reduction
* Function introspection and reflection 
* Correct function dispatch order to account for base classes and provide a defined order of dispatch 
* Predictable object lifetime that emulates C++ stack lifetime
* emarcotte: pkgconfig support
* standardize on method/member naming and indentation
* 64bit Visual Studio support
* Better support for const objects
* Drastic reduction of runtime exceptions - making debug builds orders of magnitude faster
* Support for platforms with no loadable module support
* Add helper macro for registering class 
