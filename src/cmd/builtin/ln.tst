# regression tests for the ln command

UNIT ln

TEST 01 basics

	EXEC	a b
		ERROR - 'ln: a: not found'
		EXIT 1

	EXEC	a b
		INPUT a aaa
		OUTPUT b aaa
		ERROR -
		EXIT 0

	EXEC	a b
		ERROR - 'ln: b: cannot replace existing file'
		EXIT 1

	EXEC	-f a b
		ERROR -
		EXIT 0

	EXEC	-s c d

	EXEC	-s c d
		ERROR - 'ln: d: cannot replace existing file'
		EXIT 1

	EXEC	-s c d
		INPUT c ccc

	EXEC	-f -s c d
		OUTPUT d ccc
		ERROR -
		EXIT 0
