#!/usr/bin/env bash
for sig in `/bin/kill -l 2>/dev/null`
do	case $sig in
	*[!A-Za-z0-9_]*|*MIN|*MAX)
		;;
	*)	echo "#if defined(SIG$sig) && !defined(HAD_SIG$sig)"
		echo "0,\"$sig\",SIG$sig,"
		echo "#endif"
		;;
	esac
done
