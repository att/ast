# : : generated from /home/gsf/src/cmd/builtin/head.rt by mktest : : #

UNIT head

TEST 01 'default options'

	EXEC
		INPUT -n -
		OUTPUT -
		ERROR -n -

	EXEC
		INPUT -n - a
		SAME OUTPUT INPUT

	EXEC
		INPUT -n - $'\n'
		SAME OUTPUT INPUT

	EXEC
		INPUT - $'a\n'
		SAME OUTPUT INPUT

	EXEC
		INPUT - $'1\n2\n3\n4\n5\n6\n7\n8'
		SAME OUTPUT INPUT

	EXEC
		INPUT - $'1\n2\n3\n4\n5\n6\n7\n8\n9'
		SAME OUTPUT INPUT

	EXEC
		INPUT - $'1\n2\n3\n4\n5\n6\n7\n8\n9\n0'
		SAME OUTPUT INPUT

	EXEC
		INPUT - $'1\n2\n3\n4\n5\n6\n7\n8\n9\n0\na'
		OUTPUT - $'1\n2\n3\n4\n5\n6\n7\n8\n9\n0'

	EXEC
		INPUT - $'1\n2\n3\n4\n5\n6\n7\n8\n9\n0\na\nb'

TEST 02 '-<number><qualifier>'

	EXEC	-1
		INPUT - $'1\n2'
		OUTPUT - 1
		ERROR -n -

	EXEC	-1c
		INPUT -n -
		OUTPUT -

	EXEC	-1c
		INPUT -n - 12
		OUTPUT -n - 1

	EXEC	-14c
		INPUT -n - 1234567890abcdefg
		OUTPUT -n - 1234567890abcd

TEST 03 -n

	EXEC	-n 4096m
		INPUT - a
		SAME OUTPUT INPUT
		ERROR -n -

	EXEC	-n 2048m
		SAME OUTPUT INPUT

	EXEC
		INPUT $'-fz%Zz\n' -
		SAME OUTPUT INPUT

TEST 04 '-n<octal> and -<octal>'

	EXEC	-08
		INPUT - $'\n\n\n\n\n\n\n\n\n\n\n'
		OUTPUT - $'\n\n\n\n\n\n\n'
		ERROR -n -

	EXEC	-n 08

	EXEC	-c 08

	EXEC	-010
		OUTPUT - $'\n\n\n\n\n\n\n\n\n'

	EXEC	-n 010

	EXEC	-c 010

TEST 05 'incomplete line'

	EXEC	-n 1
		INPUT -n - $'foo\nbar'
		OUTPUT - foo
		ERROR -n -

	EXEC	-1

	EXEC	-n 2
		SAME OUTPUT INPUT

	EXEC	-2
		SAME OUTPUT INPUT

	EXEC	-n 3
		SAME OUTPUT INPUT

	EXEC	-3
		SAME OUTPUT INPUT
