
#ifndef CHAISCRIPT_NO_THREADS
#define CHAISCRIPT_NO_THREADS
#endif

/// ChaiScript as a static is unsupported with thread support enabled
///

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wexit-time-destructors"
#pragma clang diagnostic ignored "-Wglobal-constructors"
#endif

#include <chaiscript/chaiscript.hpp>

static chaiscript::ChaiScript chai{};

int main() {}
