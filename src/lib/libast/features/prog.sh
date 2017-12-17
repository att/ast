#!/usr/bin/env bash
for p in /proc/self/exe /proc/self/path/a.out
do	if	test -e $p
	then	echo -n "\"$p\""
		exit 0
	fi
done
exit 1
