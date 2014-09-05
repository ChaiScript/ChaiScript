#!/bin/bash

pushd ..
wget http://sourceforge.net/projects/cppcheck/files/cppcheck/1.66/cppcheck-1.66.tar.bz2
tar -xvf cppcheck-1.66.tar.bz2
cd cppcheck-1.66
CXX=g++-4.8 make -j2
popd
../cppcheck-1.66/cppcheck --enable=all -I include --inline-suppr --suppress=missingIncludeSystem --std=c++11 --platform=unix64 src/main.cpp src/chai*.cpp --template ' - __{severity}__: [{file}:{line}](../blob/TRAVIS_COMMIT/{file}#L{line}) {message} ({id})'  2>output
sed -i "s/TRAVIS_COMMIT/${TRAVIS_COMMIT}/g" output
echo -n '{ "body": " ' > output.json
echo -n `awk '{printf "%s\\\\n", $0;}' output` >> output.json
echo -n '"}' >> output.json
if [ "${TRAVIS_PULL_REQUEST}" = "false" ]; then curl -H "Authorization: token ${TOKEN}" --request POST --data @output.json https://api.github.com/repos/ChaiScript/ChaiScript/commits/${TRAVIS_COMMIT}/comments; else  curl -H "Authorization: token ${TOKEN}" --request POST --data @output.json https://api.github.com/repos/ChaiScript/ChaiScript/issues/${TRAVIS_PULL_REQUEST}/comments; fi


