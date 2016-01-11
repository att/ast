# : : generated from /home/gsf/src/cmd/builtin/rm.rt by mktest : : #

# regression tests for the rm command

UNIT rm

TEST 01 basics

	EXEC	-f x
		INPUT - x
		OUTPUT -
		ERROR -n -

	EXEC	-f x

	EXEC	-f x x
		INPUT - x

	EXEC	-f x x
