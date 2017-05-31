
#define CHAISCRIPT_NO_THREADS

/// ChaiScript as a static is unsupported with thread support enabled
///

#include <chaiscript/chaiscript.hpp>

static chaiscript::ChaiScript chai;

int main() {}
