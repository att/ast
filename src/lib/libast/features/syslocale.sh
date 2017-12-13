#!/bin/bash
ifs=$IFS
str=
sys=
# Original command : `PATH=/bin:/usr/bin:$PATH locale -a | grep -i '^[^C].*\.UTF[-8]*$'`
# but it should be sufficient to just check for first element in the list
for i in `PATH=/bin:/usr/bin:$PATH locale -a | grep -i '^[^C].*\.UTF[-8]*$' | head -n1`
do	IFS=.
	set '' $i
	IFS=$ifs
	case $3 in
	UTF-8)	str=$3
		break
		;;
	*)	if	$SHELL -c "LC_CTYPE=$2.UTF-8 PATH=/bin:/usr/bin:$PATH locale LC_CTYPE | grep -i utf.*8" >/dev/null 2>&1
		then	str=UTF-8
			break
		fi
		;;
	esac
	sys=$3
done
case $str in
'')	str=$sys ;;
esac
case $str in
'')	echo -n "0" ;;
*)	echo -n "\"$str\"" ;;
esac

exit 0
