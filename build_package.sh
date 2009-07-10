#!/bin/bash

cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo .
make install

INCLUDED_FILES="include/chaiscript/*.hpp include/chaiscript/dispatchkit/*.hpp include/chaiscript/language/*.hpp bin/chaiscript_eval"

zip -r chaiscript-1.0.zip $INCLUDED_FILES
