IMPLEMENTATION=$($MAIN)

VIEW tst $MAIN-$IMPLEMENTATION.tst

if	[[ -f $tst ]]
then	. $tst
else	print -u2 $MAIN: $IMPLEMENTATION regression tests must be manually generated
	exit 1
fi
