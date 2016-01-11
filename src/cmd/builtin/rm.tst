# : : generated from rm.rt by mktest : : #

# regression tests for the rm command

UNIT rm

TEST 01 basics

	EXEC	-f x
		INPUT - x

	EXEC	-f x

	EXEC	-f x x
		INPUT - x

	EXEC	-f x x
