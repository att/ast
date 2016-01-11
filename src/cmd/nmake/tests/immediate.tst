# ast nmake immediate rule tests

INCLUDE test.def

TEST 01 'query'

	EXEC	-n -f - query
		INPUT - $'for i 1 2 3 4 5
	for j 1 2 3 4 5
		print $(i) $(j)
	end
end'
		OUTPUT - $'1 1
1 2
1 3
1 4
1 5
2 1
2 2
2 3
2 4
2 5
3 1
3 2
3 3
3 4
3 5
4 1
4 2
4 3
4 4
4 5
5 1
5 2
5 3
5 4
5 5'

TEST 02 'export variations'

	EXEC	-s
		INPUT Makefile $'_MAKE_TEST_FOO = foo
_MAKE_TEST_BAR = bar
_MAKE_TEST_LCL = lcl
script _MAKE_TEST_FOO _MAKE_TEST_BAR
all :
	export $(=)
	env | grep "^_MAKE_TEST_" | sort'
		OUTPUT - $'_MAKE_TEST_BAR=bar
_MAKE_TEST_FOO=foo'

	EXEC	-s
		INPUT Makefile $'_MAKE_TEST_FOO = foo
_MAKE_TEST_BAR = bar
_MAKE_TEST_LCL = lcl
export _MAKE_TEST_FOO _MAKE_TEST_BAR
all :
	env | grep "^_MAKE_TEST_" | sort'

	EXEC	-s
		INPUT Makefile $'export _MAKE_TEST_FOO=foo _MAKE_TEST_BAR=bar
_MAKE_TEST_LCL=lcl
all :
	env | grep "^_MAKE_TEST_" | sort'
