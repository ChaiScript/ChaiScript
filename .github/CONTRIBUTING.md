# Contributing to ChaiScript

Thank you for contributing!

# Pull Requests

Please follow the existing style in the code you are patching.

 - two space indent
 - no tabs EVER
 - match the existing indentation level
 
All ChaiScript commits are run through a large set of builds and analysis on all supported platforms. Those results are posted on the 
[build dashboard](http://chaiscript.com/ChaiScript-BuildResults/index.html). No PR will be accepted until all tests pass.

The build system has full integration with GitHub and you will be notified automatically if all tests have passed.

# Issues

Please do not post a "chaiscript is too slow", "chaiscript compiles too slowly", or "chaiscript needs more documentation" issue
without first reading the following notes.

## ChaiScript is Too Slow

We are actively working on constently improving the runtime performance of ChaiScript. With the performance being
[monitored with each commit](http://chaiscript.com/ChaiScript-BuildResults/performance.html).

If you feel you *must* post an issue about performance, please post a complete example that illustrates the exact case you
feel should be better optimized.

Any issue request regarding performance without a complete example of the issue experienced will be closed.

## ChaiScript Compiles Too Slowly

This is also something we are actively working on. If you need highly optimized build times, please see [this discussion
on the discourse site](http://discourse.chaiscript.com/t/slow-build-times/94).

## ChaiScript Needs More Documentation

If you have a question that is not addressed in the [cheatsheet](https://github.com/ChaiScript/ChaiScript/blob/develop/cheatsheet.md) 
please open an issue so we can get the Cheatsheet updated.
