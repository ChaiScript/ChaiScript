
#ifndef CHAISCRIPT_NO_THREADS
#define CHAISCRIPT_NO_THREADS
#endif

/// ChaiScript as a static is unsupported with thread support enabled
///

#include <chaiscript/chaiscript.hpp>

static chaiscript::ChaiScript chai;

int main() {}
