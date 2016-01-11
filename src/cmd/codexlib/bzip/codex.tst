# regression tests for the codex(1) bzip method

TITLE + bzip

SET pipe-input

KEEP '*.dat'

TEST 01 'basics'

	EXEC	'>bzip'
		INPUT - 'aaaa bbbb cccc aaaa cccc bbbb zzzz aaaa'
		MOVE OUTPUT c.dat
	EXEC	-i
		SAME INPUT c.dat
		OUTPUT - bzip
	EXEC	'<bzip'
		OUTPUT - 'aaaa bbbb cccc aaaa cccc bbbb zzzz aaaa'
	EXEC	-d
