#!/bin/bash


successes=0
failures=0

echo -n "Running unit tests"
for file in unittests/*.chai
do
  tstname=${file%.*}
	OUTPUT=`LD_LIBRARY_PATH=. ./chaiscript_eval unittests/unit_test.inc $file`
	if [ "$?" -eq "0" ]
	then
		echo -n "."
		successes=$((successes+1))
	else
		echo "[from failed test $file] $OUTPUT"
		failures=$((failures+1))
	fi
done
echo ""

total=$((successes+failures))
echo "$successes out of $total succeeded"
