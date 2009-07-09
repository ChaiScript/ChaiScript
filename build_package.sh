#!/bin/bash

cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo .
make install

INCLUDED_FILES="dispatchkit/bootstrap_stl.hpp dispatchkit/bootstrap.hpp dispatchkit/boxed_value.hpp dispatchkit/dispatchkit.hpp dispatchkit/function_call.hpp dispatchkit/proxy_constructors.hpp dispatchkit/proxy_functions.hpp dispatchkit/type_info.hpp chaiscript/chaiscript_engine.hpp chaiscript/chaiscript_eval.hpp chaiscript/chaiscript_parser.hpp chaiscript/chaiscript_prelude.hpp chaiscript/chaiscript.hpp bin/chaiscript_eval"

zip chaiscript-1.0.zip $INCLUDED_FILES
