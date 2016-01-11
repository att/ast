# : : generated from wc.rt by mktest : : #

UNIT wc

IGNORESPACE

TEST 01 'empty input'

	EXEC
		INPUT -n -
		OUTPUT - '       0       0       0'

	EXEC	-c
		OUTPUT - '       0'

	EXEC	-l

	EXEC	-w

	EXEC	-L

TEST 02 'default options'

	EXEC
		INPUT - $'a b\nc'
		OUTPUT - '       2       3       6'

TEST 03 'no newline'

	EXEC	-c
		INPUT -n - x
		OUTPUT - '       1'

	EXEC	-l
		OUTPUT - '       0'

	EXEC	-w
		OUTPUT - '       1'

	EXEC	-L

TEST 04 words

	EXEC	-c
		INPUT - 'x y'
		OUTPUT - '       4'

	EXEC	-l
		OUTPUT - '       1'

	EXEC	-w
		OUTPUT - '       2'

	EXEC	-L
		OUTPUT - '       3'

TEST 05 'words with no newline'

	EXEC	-c
		INPUT -n - $'x y\nzzzzz'
		OUTPUT - '       9'

	EXEC	-l
		OUTPUT - '       1'

	EXEC	-w
		OUTPUT - '       3'

	EXEC	-L
		OUTPUT - '       5'

TEST 06 '-l counts newline bytes'

	EXEC	-l
		INPUT -n - 'a b'
		OUTPUT - '       0'

	EXEC	-l
		INPUT - 'x y'
		OUTPUT - '       1'

	EXEC	-l
		INPUT - $'x\ny'
		OUTPUT - '       2'

TEST 07 '-L does not count the newline'

	EXEC	-L
		INPUT - $'1\n12'
		OUTPUT - '       2'

	EXEC	-L
		INPUT - $'1\n123\n1'
		OUTPUT - '       3'

	EXEC	-L
		INPUT -n - $'\n123456'
		OUTPUT - '       6'
