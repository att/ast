#!/bin/sh
set -e
set -x
for p in /proc/self/exe /proc/self/path/a.out
do	if	test -e $p
	then	echo "\"$p\""
		exit 0
	fi
done
exit 1
