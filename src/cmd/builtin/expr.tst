# regression tests for the expr utility

TEST 01 'string op output'
	EXEC	''
		EXIT 1
	EXEC	aa : 'a\(b\)*a'
	EXEC	aa : 'a\(b*\)a'
	EXEC	aba : 'a\(b\)*a'
		OUTPUT - b
		EXIT 0
	EXEC	aba : 'a\(b*\)a'
	EXEC	fred : '.*'
		OUTPUT - 4
	EXEC	fred/fred : '.*/\(.*\)'
		OUTPUT - fred
	EXEC	fred : '.*/\(.*\)'
		OUTPUT -
		EXIT 1

TEST 02 'numeric output'
	EXEC	10
		OUTPUT - 10
	EXEC	010 + 0
	EXEC	1 + 9
	EXEC	11 - 1
	EXEC	010
		OUTPUT - 010
	EXEC	+010
		OUTPUT - +010
	EXEC	1+2
		OUTPUT - 1+2
	EXEC	1 + 2
		OUTPUT - 3

TEST 03 'gnu arithmetic tests'
	EXEC	5 + 6
		OUTPUT - 11
	EXEC	5 - 6
		OUTPUT - -1
	EXEC	5 \* 6
		OUTPUT - 30
	EXEC	100 / 6
		OUTPUT - 16
	EXEC	100 % 6
		OUTPUT - 4

TEST 04 'gnu paren tests'
	EXEC	\( 100 % 6 \)
		OUTPUT - 4
	EXEC	\( 100 % 6 \) - 8
		OUTPUT - -4
	EXEC	9 / \( 100 % 6 \) - 8
		OUTPUT - -6
	EXEC	9 / \( \( 100 % 6 \) - 8 \)
		OUTPUT - -2
	EXEC	9 + \( 100 % 6 \)
		OUTPUT - 13

TEST 05 'gnu boundary tests'
	EXEC	-- 2 + 2
		OUTPUT - 4
	EXEC	00 \< 0!
		OUTPUT - 0
		EXIT 1
	EXEC	3 + -
		OUTPUT -
		ERROR - 'expr: non-numeric argument'
		EXIT 2

TEST 06 'function-like extensions'
	EXEC	abc : '.*'
		OUTPUT - 3
	EXEC	match abc '.*'
	EXEC	substr abcdef 1 3
		OUTPUT - abc
	EXEC	substr abcdef 2 3
		OUTPUT - bcd
	EXEC	substr abcdef 3 3
		OUTPUT - cde
	EXEC	substr abcdef 4 3
		OUTPUT - def
	EXEC	substr abcdef 5 3
		OUTPUT - ef
	EXEC	substr abcdef 6 3
		OUTPUT - f
	EXEC	index abc b
		OUTPUT - 2
	EXEC	length abc
		OUTPUT - 3
	EXEC	length ''
		OUTPUT - 0
		EXIT 1
	EXEC	index abc z
	EXEC	substr abcdef 0 3
		OUTPUT -
	EXEC	substr abcdef 7 3
