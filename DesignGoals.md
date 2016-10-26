# Introduction

This document outlines the principles that drive the development of ChaiScript. ChaiScript does not intent to be the perfect tool for *every* situation, but it does intend to be a good general purpose tool for *most* situations.

# Goals

1. Trivially easy to integrate with C++ projects
2. 0 external depenencies
3. "Perfect" integration with C++
   * Direct mapping between ChaiScript objects and C++ objects
   * Direct mapping between ChaiScript functions and C++ functions
   * Direct mapping between ChaiScript exceptions and C++ exceptions
3. Never surprise the C++ developer
   * Object lifetimes managed by the stack
   * Familiar syntax to C++ developers
4. Perform "well enough" to not get in the way


# Alternatives

## Sol2

If you are looking for the fastest performing scripting language and don't mind Lua, you might want to consider [sol2](https://github.com/ThePhD/sol2).

## SWIG

If you are looking for the most flexible solution to be able to support multiple target languages, consider [SWIG](http://swig.org)

