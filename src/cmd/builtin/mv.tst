# regression tests for the mv command

UNIT mv

TEST 01 basics

	EXEC	a b
		ERROR - 'mv: a: not found'
		EXIT 1

	EXEC	a b
		INPUT a aaa
		OUTPUT b aaa
		ERROR -
		EXIT 0
