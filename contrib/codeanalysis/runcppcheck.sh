#!/bin/sh

pushd ..
wget http://sourceforge.net/projects/cppcheck/files/cppcheck/1.64/cppcheck-1.64.tar.bz2
tar -xvf cppcheck-1.64.tar.bz2
cd cppcheck-1.64
make -j2
popd
../cppcheck-1.64/cppcheck --enable=all --inconclusive -I include --inline-suppr --std=c++11 --platform=unix64 src/main.cpp src/chai*.cpp --template ' - __{severity}__: [{file}:{line}](../blob/TRAVIS_COMMIT/{file}#L{line}) {message} ({id})'  2>output
sed -i "s/TRAVIS_COMMIT/${TRAVIS_COMMIT}/g" output
echo -n '{ "body": " ' > output.json
echo -n `awk '{printf "%s\\\\n", $0;}' output` >> output.json
echo -n '"}' >> output.json
if [ "${TRAVIS_PULL_REQUEST}" = "false" ]; then curl -H "Authorization: token ${TOKEN}" --request POST --data @output.json https://api.github.com/repos/ChaiScript/ChaiScript/commits/${TRAVIS_COMMIT}/comments; else  curl -H "Authorization: token ${TOKEN}" --request POST --data @output.json https://api.github.com/repos/ChaiScript/ChaiScript/issues/${TRAVIS_PULL_REQUEST}/comments; fi


