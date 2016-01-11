# regression tests for the codex(1) gzip method

TITLE + gzip

SET pipe-input

KEEP '*.dat'

TEST 01 'basics'

	EXEC	'>gzip'
		INPUT - 'aaaa bbbb cccc aaaa cccc bbbb zzzz aaaa'
		MOVE OUTPUT c.dat
	EXEC	-i
		SAME INPUT c.dat
		OUTPUT - gzip
	EXEC	'<gzip'
		OUTPUT - 'aaaa bbbb cccc aaaa cccc bbbb zzzz aaaa'
	EXEC	-d
