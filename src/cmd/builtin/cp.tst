# regression tests for the cp command

UNIT cp

TEST 01 basics

	EXEC	a
		ERROR - 'cp: a: not found'
		EXIT 1

	EXEC	a b

	EXEC	a b
		INPUT a aaa
		OUTPUT b aaa
		ERROR -
		EXIT 0
