#!/bin/bash

echo -n "Running unit tests"
for file in unittests/*.chai
do
  tstname=${file%.*}
	tst="$tstname.txt"
	./chaiscript_eval $file > /tmp/tstout.txt
	diff $tst /tmp/tstout.txt
	if [ "$?" -eq "0" ]
	then
		echo -n "."
	else
		echo "[from failed test $file]"
	fi
done
echo "done"
