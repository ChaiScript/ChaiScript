#!/bin/bash

successes=0
failures=0

echo -n "Running unit tests"
for file in unittests/*.chai
do
  tstname=${file%.*}
	tst="$tstname.txt"
	./chaiscript_eval $file > /tmp/tstout.txt
        cat $tst > /tmp/tstmaster.txt
        awk 'sub("$", "\r")' $tst > /tmp/tstmaster.txt
	diff /tmp/tstmaster.txt /tmp/tstout.txt
	if [ "$?" -eq "0" ]
	then
		echo -n "."
		successes=$((successes+1))
	else
		echo "[from failed test $file]"
		failures=$((failures+1))
	fi
done
echo ""

total=$((successes+failures))
echo "$successes out of $total succeeded"
