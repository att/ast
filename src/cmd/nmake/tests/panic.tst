# ast nmake panic tests
#
# these tests may throw nmake into a panic
#
# panic messages still to be fixed are listed
# as FIXME: instead of panic: so they show up
# as regression test failures

INCLUDE test.def

TEST 01 'local generated stdio.h'

	EXEC	--silent --nojobs
		INPUT Makefile $'all : t.c stdio.h
stdio.h : stdio
	cp $(*) $(<)
'
		INPUT t.c $'#include "/usr/include/stdio.h"'
		INPUT stdio

	EXEC	--silent --nojobs
