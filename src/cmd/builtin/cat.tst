# : : generated from /home/gsf/src/cmd/builtin/cat.rt by mktest : : #

# regression tests for the cat command

UNIT cat --regress

TEST 01 basics

	EXEC	d00000
		INPUT -n -
		INPUT d00000 $'\n\n\n\n'
		INPUT d00001 $'\n\n\n\na'
		INPUT d00010 $'\n\n\na\n'
		INPUT d00011 $'\n\n\na\nb'
		INPUT d00100 $'\n\na\n\n'
		INPUT d00101 $'\n\na\n\nb'
		INPUT d00110 $'\n\na\nb\n'
		INPUT d00111 $'\n\na\nb\nc'
		INPUT d01000 $'\na\n\n\n'
		INPUT d01001 $'\na\n\n\nb'
		INPUT d01010 $'\na\n\nb\n'
		INPUT d01011 $'\na\n\nb\nc'
		INPUT d01100 $'\na\nb\n\n'
		INPUT d01101 $'\na\nb\n\nc'
		INPUT d01110 $'\na\nb\nc\n'
		INPUT d01111 $'\na\nb\nc\nd'
		INPUT d10000 $'a\n\n\n\n'
		INPUT d10001 $'a\n\n\n\nb'
		INPUT d10010 $'a\n\n\nb\n'
		INPUT d10011 $'a\n\n\nb\nc'
		INPUT d10100 $'a\n\nb\n\n'
		INPUT d10101 $'a\n\nb\n\nc'
		INPUT d10110 $'a\n\nb\nc\n'
		INPUT d10111 $'a\n\nb\nc\nd'
		INPUT d11000 $'a\nb\n\n\n'
		INPUT d11001 $'a\nb\n\n\nc'
		INPUT d11010 $'a\nb\n\nc\n'
		INPUT d11011 $'a\nb\n\nc\nd'
		INPUT d11100 $'a\nb\nc\n\n'
		INPUT d11101 $'a\nb\nc\n\nd'
		INPUT d11110 $'a\nb\nc\nd\n'
		INPUT d11111 $'a\nb\nc\nd\ne'
		OUTPUT - $'\n\n\n\n'
		ERROR -n -

	EXEC	d00001
		OUTPUT - $'\n\n\n\na'

	EXEC	d00010
		OUTPUT - $'\n\n\na\n'

	EXEC	d00011
		OUTPUT - $'\n\n\na\nb'

	EXEC	d00100
		OUTPUT - $'\n\na\n\n'

	EXEC	d00101
		OUTPUT - $'\n\na\n\nb'

	EXEC	d00110
		OUTPUT - $'\n\na\nb\n'

	EXEC	d00111
		OUTPUT - $'\n\na\nb\nc'

	EXEC	d01000
		OUTPUT - $'\na\n\n\n'

	EXEC	d01001
		OUTPUT - $'\na\n\n\nb'

	EXEC	d01010
		OUTPUT - $'\na\n\nb\n'

	EXEC	d01011
		OUTPUT - $'\na\n\nb\nc'

	EXEC	d01100
		OUTPUT - $'\na\nb\n\n'

	EXEC	d01101
		OUTPUT - $'\na\nb\n\nc'

	EXEC	d01110
		OUTPUT - $'\na\nb\nc\n'

	EXEC	d01111
		OUTPUT - $'\na\nb\nc\nd'

	EXEC	d10000
		OUTPUT - $'a\n\n\n\n'

	EXEC	d10001
		OUTPUT - $'a\n\n\n\nb'

	EXEC	d10010
		OUTPUT - $'a\n\n\nb\n'

	EXEC	d10011
		OUTPUT - $'a\n\n\nb\nc'

	EXEC	d10100
		OUTPUT - $'a\n\nb\n\n'

	EXEC	d10101
		OUTPUT - $'a\n\nb\n\nc'

	EXEC	d10110
		OUTPUT - $'a\n\nb\nc\n'

	EXEC	d10111
		OUTPUT - $'a\n\nb\nc\nd'

	EXEC	d11000
		OUTPUT - $'a\nb\n\n\n'

	EXEC	d11001
		OUTPUT - $'a\nb\n\n\nc'

	EXEC	d11010
		OUTPUT - $'a\nb\n\nc\n'

	EXEC	d11011
		OUTPUT - $'a\nb\n\nc\nd'

	EXEC	d11100
		OUTPUT - $'a\nb\nc\n\n'

	EXEC	d11101
		OUTPUT - $'a\nb\nc\n\nd'

	EXEC	d11110
		OUTPUT - $'a\nb\nc\nd\n'

	EXEC	d11111
		OUTPUT - $'a\nb\nc\nd\ne'

	EXEC	--show-ends d00000
		OUTPUT - $'$\n$\n$\n$\n$'

	EXEC	--show-ends d00001
		OUTPUT - $'$\n$\n$\n$\na$'

	EXEC	--show-ends d00010
		OUTPUT - $'$\n$\n$\na$\n$'

	EXEC	--show-ends d00011
		OUTPUT - $'$\n$\n$\na$\nb$'

	EXEC	--show-ends d00100
		OUTPUT - $'$\n$\na$\n$\n$'

	EXEC	--show-ends d00101
		OUTPUT - $'$\n$\na$\n$\nb$'

	EXEC	--show-ends d00110
		OUTPUT - $'$\n$\na$\nb$\n$'

	EXEC	--show-ends d00111
		OUTPUT - $'$\n$\na$\nb$\nc$'

	EXEC	--show-ends d01000
		OUTPUT - $'$\na$\n$\n$\n$'

	EXEC	--show-ends d01001
		OUTPUT - $'$\na$\n$\n$\nb$'

	EXEC	--show-ends d01010
		OUTPUT - $'$\na$\n$\nb$\n$'

	EXEC	--show-ends d01011
		OUTPUT - $'$\na$\n$\nb$\nc$'

	EXEC	--show-ends d01100
		OUTPUT - $'$\na$\nb$\n$\n$'

	EXEC	--show-ends d01101
		OUTPUT - $'$\na$\nb$\n$\nc$'

	EXEC	--show-ends d01110
		OUTPUT - $'$\na$\nb$\nc$\n$'

	EXEC	--show-ends d01111
		OUTPUT - $'$\na$\nb$\nc$\nd$'

	EXEC	--show-ends d10000
		OUTPUT - $'a$\n$\n$\n$\n$'

	EXEC	--show-ends d10001
		OUTPUT - $'a$\n$\n$\n$\nb$'

	EXEC	--show-ends d10010
		OUTPUT - $'a$\n$\n$\nb$\n$'

	EXEC	--show-ends d10011
		OUTPUT - $'a$\n$\n$\nb$\nc$'

	EXEC	--show-ends d10100
		OUTPUT - $'a$\n$\nb$\n$\n$'

	EXEC	--show-ends d10101
		OUTPUT - $'a$\n$\nb$\n$\nc$'

	EXEC	--show-ends d10110
		OUTPUT - $'a$\n$\nb$\nc$\n$'

	EXEC	--show-ends d10111
		OUTPUT - $'a$\n$\nb$\nc$\nd$'

	EXEC	--show-ends d11000
		OUTPUT - $'a$\nb$\n$\n$\n$'

	EXEC	--show-ends d11001
		OUTPUT - $'a$\nb$\n$\n$\nc$'

	EXEC	--show-ends d11010
		OUTPUT - $'a$\nb$\n$\nc$\n$'

	EXEC	--show-ends d11011
		OUTPUT - $'a$\nb$\n$\nc$\nd$'

	EXEC	--show-ends d11100
		OUTPUT - $'a$\nb$\nc$\n$\n$'

	EXEC	--show-ends d11101
		OUTPUT - $'a$\nb$\nc$\n$\nd$'

	EXEC	--show-ends d11110
		OUTPUT - $'a$\nb$\nc$\nd$\n$'

	EXEC	--show-ends d11111
		OUTPUT - $'a$\nb$\nc$\nd$\ne$'

	EXEC	--show-nonprinting d00000
		OUTPUT - $'\n\n\n\n'

	EXEC	--show-nonprinting d00001
		OUTPUT - $'\n\n\n\na'

	EXEC	--show-nonprinting d00010
		OUTPUT - $'\n\n\na\n'

	EXEC	--show-nonprinting d00011
		OUTPUT - $'\n\n\na\nb'

	EXEC	--show-nonprinting d00100
		OUTPUT - $'\n\na\n\n'

	EXEC	--show-nonprinting d00101
		OUTPUT - $'\n\na\n\nb'

	EXEC	--show-nonprinting d00110
		OUTPUT - $'\n\na\nb\n'

	EXEC	--show-nonprinting d00111
		OUTPUT - $'\n\na\nb\nc'

	EXEC	--show-nonprinting d01000
		OUTPUT - $'\na\n\n\n'

	EXEC	--show-nonprinting d01001
		OUTPUT - $'\na\n\n\nb'

	EXEC	--show-nonprinting d01010
		OUTPUT - $'\na\n\nb\n'

	EXEC	--show-nonprinting d01011
		OUTPUT - $'\na\n\nb\nc'

	EXEC	--show-nonprinting d01100
		OUTPUT - $'\na\nb\n\n'

	EXEC	--show-nonprinting d01101
		OUTPUT - $'\na\nb\n\nc'

	EXEC	--show-nonprinting d01110
		OUTPUT - $'\na\nb\nc\n'

	EXEC	--show-nonprinting d01111
		OUTPUT - $'\na\nb\nc\nd'

	EXEC	--show-nonprinting d10000
		OUTPUT - $'a\n\n\n\n'

	EXEC	--show-nonprinting d10001
		OUTPUT - $'a\n\n\n\nb'

	EXEC	--show-nonprinting d10010
		OUTPUT - $'a\n\n\nb\n'

	EXEC	--show-nonprinting d10011
		OUTPUT - $'a\n\n\nb\nc'

	EXEC	--show-nonprinting d10100
		OUTPUT - $'a\n\nb\n\n'

	EXEC	--show-nonprinting d10101
		OUTPUT - $'a\n\nb\n\nc'

	EXEC	--show-nonprinting d10110
		OUTPUT - $'a\n\nb\nc\n'

	EXEC	--show-nonprinting d10111
		OUTPUT - $'a\n\nb\nc\nd'

	EXEC	--show-nonprinting d11000
		OUTPUT - $'a\nb\n\n\n'

	EXEC	--show-nonprinting d11001
		OUTPUT - $'a\nb\n\n\nc'

	EXEC	--show-nonprinting d11010
		OUTPUT - $'a\nb\n\nc\n'

	EXEC	--show-nonprinting d11011
		OUTPUT - $'a\nb\n\nc\nd'

	EXEC	--show-nonprinting d11100
		OUTPUT - $'a\nb\nc\n\n'

	EXEC	--show-nonprinting d11101
		OUTPUT - $'a\nb\nc\n\nd'

	EXEC	--show-nonprinting d11110
		OUTPUT - $'a\nb\nc\nd\n'

	EXEC	--show-nonprinting d11111
		OUTPUT - $'a\nb\nc\nd\ne'

	EXEC	--show-ends --show-nonprinting d00000
		OUTPUT - $'$\n$\n$\n$\n$'

	EXEC	--show-ends --show-nonprinting d00001
		OUTPUT - $'$\n$\n$\n$\na$'

	EXEC	--show-ends --show-nonprinting d00010
		OUTPUT - $'$\n$\n$\na$\n$'

	EXEC	--show-ends --show-nonprinting d00011
		OUTPUT - $'$\n$\n$\na$\nb$'

	EXEC	--show-ends --show-nonprinting d00100
		OUTPUT - $'$\n$\na$\n$\n$'

	EXEC	--show-ends --show-nonprinting d00101
		OUTPUT - $'$\n$\na$\n$\nb$'

	EXEC	--show-ends --show-nonprinting d00110
		OUTPUT - $'$\n$\na$\nb$\n$'

	EXEC	--show-ends --show-nonprinting d00111
		OUTPUT - $'$\n$\na$\nb$\nc$'

	EXEC	--show-ends --show-nonprinting d01000
		OUTPUT - $'$\na$\n$\n$\n$'

	EXEC	--show-ends --show-nonprinting d01001
		OUTPUT - $'$\na$\n$\n$\nb$'

	EXEC	--show-ends --show-nonprinting d01010
		OUTPUT - $'$\na$\n$\nb$\n$'

	EXEC	--show-ends --show-nonprinting d01011
		OUTPUT - $'$\na$\n$\nb$\nc$'

	EXEC	--show-ends --show-nonprinting d01100
		OUTPUT - $'$\na$\nb$\n$\n$'

	EXEC	--show-ends --show-nonprinting d01101
		OUTPUT - $'$\na$\nb$\n$\nc$'

	EXEC	--show-ends --show-nonprinting d01110
		OUTPUT - $'$\na$\nb$\nc$\n$'

	EXEC	--show-ends --show-nonprinting d01111
		OUTPUT - $'$\na$\nb$\nc$\nd$'

	EXEC	--show-ends --show-nonprinting d10000
		OUTPUT - $'a$\n$\n$\n$\n$'

	EXEC	--show-ends --show-nonprinting d10001
		OUTPUT - $'a$\n$\n$\n$\nb$'

	EXEC	--show-ends --show-nonprinting d10010
		OUTPUT - $'a$\n$\n$\nb$\n$'

	EXEC	--show-ends --show-nonprinting d10011
		OUTPUT - $'a$\n$\n$\nb$\nc$'

	EXEC	--show-ends --show-nonprinting d10100
		OUTPUT - $'a$\n$\nb$\n$\n$'

	EXEC	--show-ends --show-nonprinting d10101
		OUTPUT - $'a$\n$\nb$\n$\nc$'

	EXEC	--show-ends --show-nonprinting d10110
		OUTPUT - $'a$\n$\nb$\nc$\n$'

	EXEC	--show-ends --show-nonprinting d10111
		OUTPUT - $'a$\n$\nb$\nc$\nd$'

	EXEC	--show-ends --show-nonprinting d11000
		OUTPUT - $'a$\nb$\n$\n$\n$'

	EXEC	--show-ends --show-nonprinting d11001
		OUTPUT - $'a$\nb$\n$\n$\nc$'

	EXEC	--show-ends --show-nonprinting d11010
		OUTPUT - $'a$\nb$\n$\nc$\n$'

	EXEC	--show-ends --show-nonprinting d11011
		OUTPUT - $'a$\nb$\n$\nc$\nd$'

	EXEC	--show-ends --show-nonprinting d11100
		OUTPUT - $'a$\nb$\nc$\n$\n$'

	EXEC	--show-ends --show-nonprinting d11101
		OUTPUT - $'a$\nb$\nc$\n$\nd$'

	EXEC	--show-ends --show-nonprinting d11110
		OUTPUT - $'a$\nb$\nc$\nd$\n$'

	EXEC	--show-ends --show-nonprinting d11111
		OUTPUT - $'a$\nb$\nc$\nd$\ne$'

	EXEC	--number-nonblank d00000
		OUTPUT - $'\n\n\n\n'

	EXEC	--number-nonblank d00001
		OUTPUT - $'\n\n\n\n     1\ta'

	EXEC	--number-nonblank d00010
		OUTPUT - $'\n\n\n     1\ta\n'

	EXEC	--number-nonblank d00011
		OUTPUT - $'\n\n\n     1\ta\n     2\tb'

	EXEC	--number-nonblank d00100
		OUTPUT - $'\n\n     1\ta\n\n'

	EXEC	--number-nonblank d00101
		OUTPUT - $'\n\n     1\ta\n\n     2\tb'

	EXEC	--number-nonblank d00110
		OUTPUT - $'\n\n     1\ta\n     2\tb\n'

	EXEC	--number-nonblank d00111
		OUTPUT - $'\n\n     1\ta\n     2\tb\n     3\tc'

	EXEC	--number-nonblank d01000
		OUTPUT - $'\n     1\ta\n\n\n'

	EXEC	--number-nonblank d01001
		OUTPUT - $'\n     1\ta\n\n\n     2\tb'

	EXEC	--number-nonblank d01010
		OUTPUT - $'\n     1\ta\n\n     2\tb\n'

	EXEC	--number-nonblank d01011
		OUTPUT - $'\n     1\ta\n\n     2\tb\n     3\tc'

	EXEC	--number-nonblank d01100
		OUTPUT - $'\n     1\ta\n     2\tb\n\n'

	EXEC	--number-nonblank d01101
		OUTPUT - $'\n     1\ta\n     2\tb\n\n     3\tc'

	EXEC	--number-nonblank d01110
		OUTPUT - $'\n     1\ta\n     2\tb\n     3\tc\n'

	EXEC	--number-nonblank d01111
		OUTPUT - $'\n     1\ta\n     2\tb\n     3\tc\n     4\td'

	EXEC	--number-nonblank d10000
		OUTPUT - $'     1\ta\n\n\n\n'

	EXEC	--number-nonblank d10001
		OUTPUT - $'     1\ta\n\n\n\n     2\tb'

	EXEC	--number-nonblank d10010
		OUTPUT - $'     1\ta\n\n\n     2\tb\n'

	EXEC	--number-nonblank d10011
		OUTPUT - $'     1\ta\n\n\n     2\tb\n     3\tc'

	EXEC	--number-nonblank d10100
		OUTPUT - $'     1\ta\n\n     2\tb\n\n'

	EXEC	--number-nonblank d10101
		OUTPUT - $'     1\ta\n\n     2\tb\n\n     3\tc'

	EXEC	--number-nonblank d10110
		OUTPUT - $'     1\ta\n\n     2\tb\n     3\tc\n'

	EXEC	--number-nonblank d10111
		OUTPUT - $'     1\ta\n\n     2\tb\n     3\tc\n     4\td'

	EXEC	--number-nonblank d11000
		OUTPUT - $'     1\ta\n     2\tb\n\n\n'

	EXEC	--number-nonblank d11001
		OUTPUT - $'     1\ta\n     2\tb\n\n\n     3\tc'

	EXEC	--number-nonblank d11010
		OUTPUT - $'     1\ta\n     2\tb\n\n     3\tc\n'

	EXEC	--number-nonblank d11011
		OUTPUT - $'     1\ta\n     2\tb\n\n     3\tc\n     4\td'

	EXEC	--number-nonblank d11100
		OUTPUT - $'     1\ta\n     2\tb\n     3\tc\n\n'

	EXEC	--number-nonblank d11101
		OUTPUT - $'     1\ta\n     2\tb\n     3\tc\n\n     4\td'

	EXEC	--number-nonblank d11110
		OUTPUT - $'     1\ta\n     2\tb\n     3\tc\n     4\td\n'

	EXEC	--number-nonblank d11111
		OUTPUT - $'     1\ta\n     2\tb\n     3\tc\n     4\td\n     5\te'

	EXEC	--show-ends --number-nonblank d00000
		OUTPUT - $'$\n$\n$\n$\n$'

	EXEC	--show-ends --number-nonblank d00001
		OUTPUT - $'$\n$\n$\n$\n     1\ta$'

	EXEC	--show-ends --number-nonblank d00010
		OUTPUT - $'$\n$\n$\n     1\ta$\n$'

	EXEC	--show-ends --number-nonblank d00011
		OUTPUT - $'$\n$\n$\n     1\ta$\n     2\tb$'

	EXEC	--show-ends --number-nonblank d00100
		OUTPUT - $'$\n$\n     1\ta$\n$\n$'

	EXEC	--show-ends --number-nonblank d00101
		OUTPUT - $'$\n$\n     1\ta$\n$\n     2\tb$'

	EXEC	--show-ends --number-nonblank d00110
		OUTPUT - $'$\n$\n     1\ta$\n     2\tb$\n$'

	EXEC	--show-ends --number-nonblank d00111
		OUTPUT - $'$\n$\n     1\ta$\n     2\tb$\n     3\tc$'

	EXEC	--show-ends --number-nonblank d01000
		OUTPUT - $'$\n     1\ta$\n$\n$\n$'

	EXEC	--show-ends --number-nonblank d01001
		OUTPUT - $'$\n     1\ta$\n$\n$\n     2\tb$'

	EXEC	--show-ends --number-nonblank d01010
		OUTPUT - $'$\n     1\ta$\n$\n     2\tb$\n$'

	EXEC	--show-ends --number-nonblank d01011
		OUTPUT - $'$\n     1\ta$\n$\n     2\tb$\n     3\tc$'

	EXEC	--show-ends --number-nonblank d01100
		OUTPUT - $'$\n     1\ta$\n     2\tb$\n$\n$'

	EXEC	--show-ends --number-nonblank d01101
		OUTPUT - $'$\n     1\ta$\n     2\tb$\n$\n     3\tc$'

	EXEC	--show-ends --number-nonblank d01110
		OUTPUT - $'$\n     1\ta$\n     2\tb$\n     3\tc$\n$'

	EXEC	--show-ends --number-nonblank d01111
		OUTPUT - $'$\n     1\ta$\n     2\tb$\n     3\tc$\n     4\td$'

	EXEC	--show-ends --number-nonblank d10000
		OUTPUT - $'     1\ta$\n$\n$\n$\n$'

	EXEC	--show-ends --number-nonblank d10001
		OUTPUT - $'     1\ta$\n$\n$\n$\n     2\tb$'

	EXEC	--show-ends --number-nonblank d10010
		OUTPUT - $'     1\ta$\n$\n$\n     2\tb$\n$'

	EXEC	--show-ends --number-nonblank d10011
		OUTPUT - $'     1\ta$\n$\n$\n     2\tb$\n     3\tc$'

	EXEC	--show-ends --number-nonblank d10100
		OUTPUT - $'     1\ta$\n$\n     2\tb$\n$\n$'

	EXEC	--show-ends --number-nonblank d10101
		OUTPUT - $'     1\ta$\n$\n     2\tb$\n$\n     3\tc$'

	EXEC	--show-ends --number-nonblank d10110
		OUTPUT - $'     1\ta$\n$\n     2\tb$\n     3\tc$\n$'

	EXEC	--show-ends --number-nonblank d10111
		OUTPUT - $'     1\ta$\n$\n     2\tb$\n     3\tc$\n     4\td$'

	EXEC	--show-ends --number-nonblank d11000
		OUTPUT - $'     1\ta$\n     2\tb$\n$\n$\n$'

	EXEC	--show-ends --number-nonblank d11001
		OUTPUT - $'     1\ta$\n     2\tb$\n$\n$\n     3\tc$'

	EXEC	--show-ends --number-nonblank d11010
		OUTPUT - $'     1\ta$\n     2\tb$\n$\n     3\tc$\n$'

	EXEC	--show-ends --number-nonblank d11011
		OUTPUT - $'     1\ta$\n     2\tb$\n$\n     3\tc$\n     4\td$'

	EXEC	--show-ends --number-nonblank d11100
		OUTPUT - $'     1\ta$\n     2\tb$\n     3\tc$\n$\n$'

	EXEC	--show-ends --number-nonblank d11101
		OUTPUT - $'     1\ta$\n     2\tb$\n     3\tc$\n$\n     4\td$'

	EXEC	--show-ends --number-nonblank d11110
		OUTPUT - $'     1\ta$\n     2\tb$\n     3\tc$\n     4\td$\n$'

	EXEC	--show-ends --number-nonblank d11111
		OUTPUT - $'     1\ta$\n     2\tb$\n     3\tc$\n     4\td$\n     5\te$'

	EXEC	--show-nonprinting --number-nonblank d00000
		OUTPUT - $'\n\n\n\n'

	EXEC	--show-nonprinting --number-nonblank d00001
		OUTPUT - $'\n\n\n\n     1\ta'

	EXEC	--show-nonprinting --number-nonblank d00010
		OUTPUT - $'\n\n\n     1\ta\n'

	EXEC	--show-nonprinting --number-nonblank d00011
		OUTPUT - $'\n\n\n     1\ta\n     2\tb'

	EXEC	--show-nonprinting --number-nonblank d00100
		OUTPUT - $'\n\n     1\ta\n\n'

	EXEC	--show-nonprinting --number-nonblank d00101
		OUTPUT - $'\n\n     1\ta\n\n     2\tb'

	EXEC	--show-nonprinting --number-nonblank d00110
		OUTPUT - $'\n\n     1\ta\n     2\tb\n'

	EXEC	--show-nonprinting --number-nonblank d00111
		OUTPUT - $'\n\n     1\ta\n     2\tb\n     3\tc'

	EXEC	--show-nonprinting --number-nonblank d01000
		OUTPUT - $'\n     1\ta\n\n\n'

	EXEC	--show-nonprinting --number-nonblank d01001
		OUTPUT - $'\n     1\ta\n\n\n     2\tb'

	EXEC	--show-nonprinting --number-nonblank d01010
		OUTPUT - $'\n     1\ta\n\n     2\tb\n'

	EXEC	--show-nonprinting --number-nonblank d01011
		OUTPUT - $'\n     1\ta\n\n     2\tb\n     3\tc'

	EXEC	--show-nonprinting --number-nonblank d01100
		OUTPUT - $'\n     1\ta\n     2\tb\n\n'

	EXEC	--show-nonprinting --number-nonblank d01101
		OUTPUT - $'\n     1\ta\n     2\tb\n\n     3\tc'

	EXEC	--show-nonprinting --number-nonblank d01110
		OUTPUT - $'\n     1\ta\n     2\tb\n     3\tc\n'

	EXEC	--show-nonprinting --number-nonblank d01111
		OUTPUT - $'\n     1\ta\n     2\tb\n     3\tc\n     4\td'

	EXEC	--show-nonprinting --number-nonblank d10000
		OUTPUT - $'     1\ta\n\n\n\n'

	EXEC	--show-nonprinting --number-nonblank d10001
		OUTPUT - $'     1\ta\n\n\n\n     2\tb'

	EXEC	--show-nonprinting --number-nonblank d10010
		OUTPUT - $'     1\ta\n\n\n     2\tb\n'

	EXEC	--show-nonprinting --number-nonblank d10011
		OUTPUT - $'     1\ta\n\n\n     2\tb\n     3\tc'

	EXEC	--show-nonprinting --number-nonblank d10100
		OUTPUT - $'     1\ta\n\n     2\tb\n\n'

	EXEC	--show-nonprinting --number-nonblank d10101
		OUTPUT - $'     1\ta\n\n     2\tb\n\n     3\tc'

	EXEC	--show-nonprinting --number-nonblank d10110
		OUTPUT - $'     1\ta\n\n     2\tb\n     3\tc\n'

	EXEC	--show-nonprinting --number-nonblank d10111
		OUTPUT - $'     1\ta\n\n     2\tb\n     3\tc\n     4\td'

	EXEC	--show-nonprinting --number-nonblank d11000
		OUTPUT - $'     1\ta\n     2\tb\n\n\n'

	EXEC	--show-nonprinting --number-nonblank d11001
		OUTPUT - $'     1\ta\n     2\tb\n\n\n     3\tc'

	EXEC	--show-nonprinting --number-nonblank d11010
		OUTPUT - $'     1\ta\n     2\tb\n\n     3\tc\n'

	EXEC	--show-nonprinting --number-nonblank d11011
		OUTPUT - $'     1\ta\n     2\tb\n\n     3\tc\n     4\td'

	EXEC	--show-nonprinting --number-nonblank d11100
		OUTPUT - $'     1\ta\n     2\tb\n     3\tc\n\n'

	EXEC	--show-nonprinting --number-nonblank d11101
		OUTPUT - $'     1\ta\n     2\tb\n     3\tc\n\n     4\td'

	EXEC	--show-nonprinting --number-nonblank d11110
		OUTPUT - $'     1\ta\n     2\tb\n     3\tc\n     4\td\n'

	EXEC	--show-nonprinting --number-nonblank d11111
		OUTPUT - $'     1\ta\n     2\tb\n     3\tc\n     4\td\n     5\te'

	EXEC	--show-ends --show-nonprinting --number-nonblank d00000
		OUTPUT - $'$\n$\n$\n$\n$'

	EXEC	--show-ends --show-nonprinting --number-nonblank d00001
		OUTPUT - $'$\n$\n$\n$\n     1\ta$'

	EXEC	--show-ends --show-nonprinting --number-nonblank d00010
		OUTPUT - $'$\n$\n$\n     1\ta$\n$'

	EXEC	--show-ends --show-nonprinting --number-nonblank d00011
		OUTPUT - $'$\n$\n$\n     1\ta$\n     2\tb$'

	EXEC	--show-ends --show-nonprinting --number-nonblank d00100
		OUTPUT - $'$\n$\n     1\ta$\n$\n$'

	EXEC	--show-ends --show-nonprinting --number-nonblank d00101
		OUTPUT - $'$\n$\n     1\ta$\n$\n     2\tb$'

	EXEC	--show-ends --show-nonprinting --number-nonblank d00110
		OUTPUT - $'$\n$\n     1\ta$\n     2\tb$\n$'

	EXEC	--show-ends --show-nonprinting --number-nonblank d00111
		OUTPUT - $'$\n$\n     1\ta$\n     2\tb$\n     3\tc$'

	EXEC	--show-ends --show-nonprinting --number-nonblank d01000
		OUTPUT - $'$\n     1\ta$\n$\n$\n$'

	EXEC	--show-ends --show-nonprinting --number-nonblank d01001
		OUTPUT - $'$\n     1\ta$\n$\n$\n     2\tb$'

	EXEC	--show-ends --show-nonprinting --number-nonblank d01010
		OUTPUT - $'$\n     1\ta$\n$\n     2\tb$\n$'

	EXEC	--show-ends --show-nonprinting --number-nonblank d01011
		OUTPUT - $'$\n     1\ta$\n$\n     2\tb$\n     3\tc$'

	EXEC	--show-ends --show-nonprinting --number-nonblank d01100
		OUTPUT - $'$\n     1\ta$\n     2\tb$\n$\n$'

	EXEC	--show-ends --show-nonprinting --number-nonblank d01101
		OUTPUT - $'$\n     1\ta$\n     2\tb$\n$\n     3\tc$'

	EXEC	--show-ends --show-nonprinting --number-nonblank d01110
		OUTPUT - $'$\n     1\ta$\n     2\tb$\n     3\tc$\n$'

	EXEC	--show-ends --show-nonprinting --number-nonblank d01111
		OUTPUT - $'$\n     1\ta$\n     2\tb$\n     3\tc$\n     4\td$'

	EXEC	--show-ends --show-nonprinting --number-nonblank d10000
		OUTPUT - $'     1\ta$\n$\n$\n$\n$'

	EXEC	--show-ends --show-nonprinting --number-nonblank d10001
		OUTPUT - $'     1\ta$\n$\n$\n$\n     2\tb$'

	EXEC	--show-ends --show-nonprinting --number-nonblank d10010
		OUTPUT - $'     1\ta$\n$\n$\n     2\tb$\n$'

	EXEC	--show-ends --show-nonprinting --number-nonblank d10011
		OUTPUT - $'     1\ta$\n$\n$\n     2\tb$\n     3\tc$'

	EXEC	--show-ends --show-nonprinting --number-nonblank d10100
		OUTPUT - $'     1\ta$\n$\n     2\tb$\n$\n$'

	EXEC	--show-ends --show-nonprinting --number-nonblank d10101
		OUTPUT - $'     1\ta$\n$\n     2\tb$\n$\n     3\tc$'

	EXEC	--show-ends --show-nonprinting --number-nonblank d10110
		OUTPUT - $'     1\ta$\n$\n     2\tb$\n     3\tc$\n$'

	EXEC	--show-ends --show-nonprinting --number-nonblank d10111
		OUTPUT - $'     1\ta$\n$\n     2\tb$\n     3\tc$\n     4\td$'

	EXEC	--show-ends --show-nonprinting --number-nonblank d11000
		OUTPUT - $'     1\ta$\n     2\tb$\n$\n$\n$'

	EXEC	--show-ends --show-nonprinting --number-nonblank d11001
		OUTPUT - $'     1\ta$\n     2\tb$\n$\n$\n     3\tc$'

	EXEC	--show-ends --show-nonprinting --number-nonblank d11010
		OUTPUT - $'     1\ta$\n     2\tb$\n$\n     3\tc$\n$'

	EXEC	--show-ends --show-nonprinting --number-nonblank d11011
		OUTPUT - $'     1\ta$\n     2\tb$\n$\n     3\tc$\n     4\td$'

	EXEC	--show-ends --show-nonprinting --number-nonblank d11100
		OUTPUT - $'     1\ta$\n     2\tb$\n     3\tc$\n$\n$'

	EXEC	--show-ends --show-nonprinting --number-nonblank d11101
		OUTPUT - $'     1\ta$\n     2\tb$\n     3\tc$\n$\n     4\td$'

	EXEC	--show-ends --show-nonprinting --number-nonblank d11110
		OUTPUT - $'     1\ta$\n     2\tb$\n     3\tc$\n     4\td$\n$'

	EXEC	--show-ends --show-nonprinting --number-nonblank d11111
		OUTPUT - $'     1\ta$\n     2\tb$\n     3\tc$\n     4\td$\n     5\te$'

	EXEC	--squeeze-blank d00000
		OUTPUT -n - $'\n'

	EXEC	--squeeze-blank d00001
		OUTPUT - $'\na'

	EXEC	--squeeze-blank d00010
		OUTPUT - $'\na\n'

	EXEC	--squeeze-blank d00011
		OUTPUT - $'\na\nb'

	EXEC	--squeeze-blank d00100
		OUTPUT - $'\na\n'

	EXEC	--squeeze-blank d00101
		OUTPUT - $'\na\n\nb'

	EXEC	--squeeze-blank d00110
		OUTPUT - $'\na\nb\n'

	EXEC	--squeeze-blank d00111
		OUTPUT - $'\na\nb\nc'

	EXEC	--squeeze-blank d01000
		OUTPUT - $'\na\n'

	EXEC	--squeeze-blank d01001
		OUTPUT - $'\na\n\nb'

	EXEC	--squeeze-blank d01010
		OUTPUT - $'\na\n\nb\n'

	EXEC	--squeeze-blank d01011
		OUTPUT - $'\na\n\nb\nc'

	EXEC	--squeeze-blank d01100
		OUTPUT - $'\na\nb\n'

	EXEC	--squeeze-blank d01101
		OUTPUT - $'\na\nb\n\nc'

	EXEC	--squeeze-blank d01110
		OUTPUT - $'\na\nb\nc\n'

	EXEC	--squeeze-blank d01111
		OUTPUT - $'\na\nb\nc\nd'

	EXEC	--squeeze-blank d10000
		OUTPUT - $'a\n'

	EXEC	--squeeze-blank d10001
		OUTPUT - $'a\n\nb'

	EXEC	--squeeze-blank d10010
		OUTPUT - $'a\n\nb\n'

	EXEC	--squeeze-blank d10011
		OUTPUT - $'a\n\nb\nc'

	EXEC	--squeeze-blank d10100
		OUTPUT - $'a\n\nb\n'

	EXEC	--squeeze-blank d10101
		OUTPUT - $'a\n\nb\n\nc'

	EXEC	--squeeze-blank d10110
		OUTPUT - $'a\n\nb\nc\n'

	EXEC	--squeeze-blank d10111
		OUTPUT - $'a\n\nb\nc\nd'

	EXEC	--squeeze-blank d11000
		OUTPUT - $'a\nb\n'

	EXEC	--squeeze-blank d11001
		OUTPUT - $'a\nb\n\nc'

	EXEC	--squeeze-blank d11010
		OUTPUT - $'a\nb\n\nc\n'

	EXEC	--squeeze-blank d11011
		OUTPUT - $'a\nb\n\nc\nd'

	EXEC	--squeeze-blank d11100
		OUTPUT - $'a\nb\nc\n'

	EXEC	--squeeze-blank d11101
		OUTPUT - $'a\nb\nc\n\nd'

	EXEC	--squeeze-blank d11110
		OUTPUT - $'a\nb\nc\nd\n'

	EXEC	--squeeze-blank d11111
		OUTPUT - $'a\nb\nc\nd\ne'

	EXEC	--show-ends --squeeze-blank d00000
		OUTPUT - '$'

	EXEC	--show-ends --squeeze-blank d00001
		OUTPUT - $'$\na$'

	EXEC	--show-ends --squeeze-blank d00010
		OUTPUT - $'$\na$\n$'

	EXEC	--show-ends --squeeze-blank d00011
		OUTPUT - $'$\na$\nb$'

	EXEC	--show-ends --squeeze-blank d00100
		OUTPUT - $'$\na$\n$'

	EXEC	--show-ends --squeeze-blank d00101
		OUTPUT - $'$\na$\n$\nb$'

	EXEC	--show-ends --squeeze-blank d00110
		OUTPUT - $'$\na$\nb$\n$'

	EXEC	--show-ends --squeeze-blank d00111
		OUTPUT - $'$\na$\nb$\nc$'

	EXEC	--show-ends --squeeze-blank d01000
		OUTPUT - $'$\na$\n$'

	EXEC	--show-ends --squeeze-blank d01001
		OUTPUT - $'$\na$\n$\nb$'

	EXEC	--show-ends --squeeze-blank d01010
		OUTPUT - $'$\na$\n$\nb$\n$'

	EXEC	--show-ends --squeeze-blank d01011
		OUTPUT - $'$\na$\n$\nb$\nc$'

	EXEC	--show-ends --squeeze-blank d01100
		OUTPUT - $'$\na$\nb$\n$'

	EXEC	--show-ends --squeeze-blank d01101
		OUTPUT - $'$\na$\nb$\n$\nc$'

	EXEC	--show-ends --squeeze-blank d01110
		OUTPUT - $'$\na$\nb$\nc$\n$'

	EXEC	--show-ends --squeeze-blank d01111
		OUTPUT - $'$\na$\nb$\nc$\nd$'

	EXEC	--show-ends --squeeze-blank d10000
		OUTPUT - $'a$\n$'

	EXEC	--show-ends --squeeze-blank d10001
		OUTPUT - $'a$\n$\nb$'

	EXEC	--show-ends --squeeze-blank d10010
		OUTPUT - $'a$\n$\nb$\n$'

	EXEC	--show-ends --squeeze-blank d10011
		OUTPUT - $'a$\n$\nb$\nc$'

	EXEC	--show-ends --squeeze-blank d10100
		OUTPUT - $'a$\n$\nb$\n$'

	EXEC	--show-ends --squeeze-blank d10101
		OUTPUT - $'a$\n$\nb$\n$\nc$'

	EXEC	--show-ends --squeeze-blank d10110
		OUTPUT - $'a$\n$\nb$\nc$\n$'

	EXEC	--show-ends --squeeze-blank d10111
		OUTPUT - $'a$\n$\nb$\nc$\nd$'

	EXEC	--show-ends --squeeze-blank d11000
		OUTPUT - $'a$\nb$\n$'

	EXEC	--show-ends --squeeze-blank d11001
		OUTPUT - $'a$\nb$\n$\nc$'

	EXEC	--show-ends --squeeze-blank d11010
		OUTPUT - $'a$\nb$\n$\nc$\n$'

	EXEC	--show-ends --squeeze-blank d11011
		OUTPUT - $'a$\nb$\n$\nc$\nd$'

	EXEC	--show-ends --squeeze-blank d11100
		OUTPUT - $'a$\nb$\nc$\n$'

	EXEC	--show-ends --squeeze-blank d11101
		OUTPUT - $'a$\nb$\nc$\n$\nd$'

	EXEC	--show-ends --squeeze-blank d11110
		OUTPUT - $'a$\nb$\nc$\nd$\n$'

	EXEC	--show-ends --squeeze-blank d11111
		OUTPUT - $'a$\nb$\nc$\nd$\ne$'

	EXEC	--show-nonprinting --squeeze-blank d00000
		OUTPUT -n - $'\n'

	EXEC	--show-nonprinting --squeeze-blank d00001
		OUTPUT - $'\na'

	EXEC	--show-nonprinting --squeeze-blank d00010
		OUTPUT - $'\na\n'

	EXEC	--show-nonprinting --squeeze-blank d00011
		OUTPUT - $'\na\nb'

	EXEC	--show-nonprinting --squeeze-blank d00100
		OUTPUT - $'\na\n'

	EXEC	--show-nonprinting --squeeze-blank d00101
		OUTPUT - $'\na\n\nb'

	EXEC	--show-nonprinting --squeeze-blank d00110
		OUTPUT - $'\na\nb\n'

	EXEC	--show-nonprinting --squeeze-blank d00111
		OUTPUT - $'\na\nb\nc'

	EXEC	--show-nonprinting --squeeze-blank d01000
		OUTPUT - $'\na\n'

	EXEC	--show-nonprinting --squeeze-blank d01001
		OUTPUT - $'\na\n\nb'

	EXEC	--show-nonprinting --squeeze-blank d01010
		OUTPUT - $'\na\n\nb\n'

	EXEC	--show-nonprinting --squeeze-blank d01011
		OUTPUT - $'\na\n\nb\nc'

	EXEC	--show-nonprinting --squeeze-blank d01100
		OUTPUT - $'\na\nb\n'

	EXEC	--show-nonprinting --squeeze-blank d01101
		OUTPUT - $'\na\nb\n\nc'

	EXEC	--show-nonprinting --squeeze-blank d01110
		OUTPUT - $'\na\nb\nc\n'

	EXEC	--show-nonprinting --squeeze-blank d01111
		OUTPUT - $'\na\nb\nc\nd'

	EXEC	--show-nonprinting --squeeze-blank d10000
		OUTPUT - $'a\n'

	EXEC	--show-nonprinting --squeeze-blank d10001
		OUTPUT - $'a\n\nb'

	EXEC	--show-nonprinting --squeeze-blank d10010
		OUTPUT - $'a\n\nb\n'

	EXEC	--show-nonprinting --squeeze-blank d10011
		OUTPUT - $'a\n\nb\nc'

	EXEC	--show-nonprinting --squeeze-blank d10100
		OUTPUT - $'a\n\nb\n'

	EXEC	--show-nonprinting --squeeze-blank d10101
		OUTPUT - $'a\n\nb\n\nc'

	EXEC	--show-nonprinting --squeeze-blank d10110
		OUTPUT - $'a\n\nb\nc\n'

	EXEC	--show-nonprinting --squeeze-blank d10111
		OUTPUT - $'a\n\nb\nc\nd'

	EXEC	--show-nonprinting --squeeze-blank d11000
		OUTPUT - $'a\nb\n'

	EXEC	--show-nonprinting --squeeze-blank d11001
		OUTPUT - $'a\nb\n\nc'

	EXEC	--show-nonprinting --squeeze-blank d11010
		OUTPUT - $'a\nb\n\nc\n'

	EXEC	--show-nonprinting --squeeze-blank d11011
		OUTPUT - $'a\nb\n\nc\nd'

	EXEC	--show-nonprinting --squeeze-blank d11100
		OUTPUT - $'a\nb\nc\n'

	EXEC	--show-nonprinting --squeeze-blank d11101
		OUTPUT - $'a\nb\nc\n\nd'

	EXEC	--show-nonprinting --squeeze-blank d11110
		OUTPUT - $'a\nb\nc\nd\n'

	EXEC	--show-nonprinting --squeeze-blank d11111
		OUTPUT - $'a\nb\nc\nd\ne'

	EXEC	--show-ends --show-nonprinting --squeeze-blank d00000
		OUTPUT - '$'

	EXEC	--show-ends --show-nonprinting --squeeze-blank d00001
		OUTPUT - $'$\na$'

	EXEC	--show-ends --show-nonprinting --squeeze-blank d00010
		OUTPUT - $'$\na$\n$'

	EXEC	--show-ends --show-nonprinting --squeeze-blank d00011
		OUTPUT - $'$\na$\nb$'

	EXEC	--show-ends --show-nonprinting --squeeze-blank d00100
		OUTPUT - $'$\na$\n$'

	EXEC	--show-ends --show-nonprinting --squeeze-blank d00101
		OUTPUT - $'$\na$\n$\nb$'

	EXEC	--show-ends --show-nonprinting --squeeze-blank d00110
		OUTPUT - $'$\na$\nb$\n$'

	EXEC	--show-ends --show-nonprinting --squeeze-blank d00111
		OUTPUT - $'$\na$\nb$\nc$'

	EXEC	--show-ends --show-nonprinting --squeeze-blank d01000
		OUTPUT - $'$\na$\n$'

	EXEC	--show-ends --show-nonprinting --squeeze-blank d01001
		OUTPUT - $'$\na$\n$\nb$'

	EXEC	--show-ends --show-nonprinting --squeeze-blank d01010
		OUTPUT - $'$\na$\n$\nb$\n$'

	EXEC	--show-ends --show-nonprinting --squeeze-blank d01011
		OUTPUT - $'$\na$\n$\nb$\nc$'

	EXEC	--show-ends --show-nonprinting --squeeze-blank d01100
		OUTPUT - $'$\na$\nb$\n$'

	EXEC	--show-ends --show-nonprinting --squeeze-blank d01101
		OUTPUT - $'$\na$\nb$\n$\nc$'

	EXEC	--show-ends --show-nonprinting --squeeze-blank d01110
		OUTPUT - $'$\na$\nb$\nc$\n$'

	EXEC	--show-ends --show-nonprinting --squeeze-blank d01111
		OUTPUT - $'$\na$\nb$\nc$\nd$'

	EXEC	--show-ends --show-nonprinting --squeeze-blank d10000
		OUTPUT - $'a$\n$'

	EXEC	--show-ends --show-nonprinting --squeeze-blank d10001
		OUTPUT - $'a$\n$\nb$'

	EXEC	--show-ends --show-nonprinting --squeeze-blank d10010
		OUTPUT - $'a$\n$\nb$\n$'

	EXEC	--show-ends --show-nonprinting --squeeze-blank d10011
		OUTPUT - $'a$\n$\nb$\nc$'

	EXEC	--show-ends --show-nonprinting --squeeze-blank d10100
		OUTPUT - $'a$\n$\nb$\n$'

	EXEC	--show-ends --show-nonprinting --squeeze-blank d10101
		OUTPUT - $'a$\n$\nb$\n$\nc$'

	EXEC	--show-ends --show-nonprinting --squeeze-blank d10110
		OUTPUT - $'a$\n$\nb$\nc$\n$'

	EXEC	--show-ends --show-nonprinting --squeeze-blank d10111
		OUTPUT - $'a$\n$\nb$\nc$\nd$'

	EXEC	--show-ends --show-nonprinting --squeeze-blank d11000
		OUTPUT - $'a$\nb$\n$'

	EXEC	--show-ends --show-nonprinting --squeeze-blank d11001
		OUTPUT - $'a$\nb$\n$\nc$'

	EXEC	--show-ends --show-nonprinting --squeeze-blank d11010
		OUTPUT - $'a$\nb$\n$\nc$\n$'

	EXEC	--show-ends --show-nonprinting --squeeze-blank d11011
		OUTPUT - $'a$\nb$\n$\nc$\nd$'

	EXEC	--show-ends --show-nonprinting --squeeze-blank d11100
		OUTPUT - $'a$\nb$\nc$\n$'

	EXEC	--show-ends --show-nonprinting --squeeze-blank d11101
		OUTPUT - $'a$\nb$\nc$\n$\nd$'

	EXEC	--show-ends --show-nonprinting --squeeze-blank d11110
		OUTPUT - $'a$\nb$\nc$\nd$\n$'

	EXEC	--show-ends --show-nonprinting --squeeze-blank d11111
		OUTPUT - $'a$\nb$\nc$\nd$\ne$'

	EXEC	--number-nonblank --squeeze-blank d00000
		OUTPUT -n - $'\n'

	EXEC	--number-nonblank --squeeze-blank d00001
		OUTPUT - $'\n     1\ta'

	EXEC	--number-nonblank --squeeze-blank d00010
		OUTPUT - $'\n     1\ta\n'

	EXEC	--number-nonblank --squeeze-blank d00011
		OUTPUT - $'\n     1\ta\n     2\tb'

	EXEC	--number-nonblank --squeeze-blank d00100
		OUTPUT - $'\n     1\ta\n'

	EXEC	--number-nonblank --squeeze-blank d00101
		OUTPUT - $'\n     1\ta\n\n     2\tb'

	EXEC	--number-nonblank --squeeze-blank d00110
		OUTPUT - $'\n     1\ta\n     2\tb\n'

	EXEC	--number-nonblank --squeeze-blank d00111
		OUTPUT - $'\n     1\ta\n     2\tb\n     3\tc'

	EXEC	--number-nonblank --squeeze-blank d01000
		OUTPUT - $'\n     1\ta\n'

	EXEC	--number-nonblank --squeeze-blank d01001
		OUTPUT - $'\n     1\ta\n\n     2\tb'

	EXEC	--number-nonblank --squeeze-blank d01010
		OUTPUT - $'\n     1\ta\n\n     2\tb\n'

	EXEC	--number-nonblank --squeeze-blank d01011
		OUTPUT - $'\n     1\ta\n\n     2\tb\n     3\tc'

	EXEC	--number-nonblank --squeeze-blank d01100
		OUTPUT - $'\n     1\ta\n     2\tb\n'

	EXEC	--number-nonblank --squeeze-blank d01101
		OUTPUT - $'\n     1\ta\n     2\tb\n\n     3\tc'

	EXEC	--number-nonblank --squeeze-blank d01110
		OUTPUT - $'\n     1\ta\n     2\tb\n     3\tc\n'

	EXEC	--number-nonblank --squeeze-blank d01111
		OUTPUT - $'\n     1\ta\n     2\tb\n     3\tc\n     4\td'

	EXEC	--number-nonblank --squeeze-blank d10000
		OUTPUT - $'     1\ta\n'

	EXEC	--number-nonblank --squeeze-blank d10001
		OUTPUT - $'     1\ta\n\n     2\tb'

	EXEC	--number-nonblank --squeeze-blank d10010
		OUTPUT - $'     1\ta\n\n     2\tb\n'

	EXEC	--number-nonblank --squeeze-blank d10011
		OUTPUT - $'     1\ta\n\n     2\tb\n     3\tc'

	EXEC	--number-nonblank --squeeze-blank d10100
		OUTPUT - $'     1\ta\n\n     2\tb\n'

	EXEC	--number-nonblank --squeeze-blank d10101
		OUTPUT - $'     1\ta\n\n     2\tb\n\n     3\tc'

	EXEC	--number-nonblank --squeeze-blank d10110
		OUTPUT - $'     1\ta\n\n     2\tb\n     3\tc\n'

	EXEC	--number-nonblank --squeeze-blank d10111
		OUTPUT - $'     1\ta\n\n     2\tb\n     3\tc\n     4\td'

	EXEC	--number-nonblank --squeeze-blank d11000
		OUTPUT - $'     1\ta\n     2\tb\n'

	EXEC	--number-nonblank --squeeze-blank d11001
		OUTPUT - $'     1\ta\n     2\tb\n\n     3\tc'

	EXEC	--number-nonblank --squeeze-blank d11010
		OUTPUT - $'     1\ta\n     2\tb\n\n     3\tc\n'

	EXEC	--number-nonblank --squeeze-blank d11011
		OUTPUT - $'     1\ta\n     2\tb\n\n     3\tc\n     4\td'

	EXEC	--number-nonblank --squeeze-blank d11100
		OUTPUT - $'     1\ta\n     2\tb\n     3\tc\n'

	EXEC	--number-nonblank --squeeze-blank d11101
		OUTPUT - $'     1\ta\n     2\tb\n     3\tc\n\n     4\td'

	EXEC	--number-nonblank --squeeze-blank d11110
		OUTPUT - $'     1\ta\n     2\tb\n     3\tc\n     4\td\n'

	EXEC	--number-nonblank --squeeze-blank d11111
		OUTPUT - $'     1\ta\n     2\tb\n     3\tc\n     4\td\n     5\te'

	EXEC	--show-ends --number-nonblank --squeeze-blank d00000
		OUTPUT - '$'

	EXEC	--show-ends --number-nonblank --squeeze-blank d00001
		OUTPUT - $'$\n     1\ta$'

	EXEC	--show-ends --number-nonblank --squeeze-blank d00010
		OUTPUT - $'$\n     1\ta$\n$'

	EXEC	--show-ends --number-nonblank --squeeze-blank d00011
		OUTPUT - $'$\n     1\ta$\n     2\tb$'

	EXEC	--show-ends --number-nonblank --squeeze-blank d00100
		OUTPUT - $'$\n     1\ta$\n$'

	EXEC	--show-ends --number-nonblank --squeeze-blank d00101
		OUTPUT - $'$\n     1\ta$\n$\n     2\tb$'

	EXEC	--show-ends --number-nonblank --squeeze-blank d00110
		OUTPUT - $'$\n     1\ta$\n     2\tb$\n$'

	EXEC	--show-ends --number-nonblank --squeeze-blank d00111
		OUTPUT - $'$\n     1\ta$\n     2\tb$\n     3\tc$'

	EXEC	--show-ends --number-nonblank --squeeze-blank d01000
		OUTPUT - $'$\n     1\ta$\n$'

	EXEC	--show-ends --number-nonblank --squeeze-blank d01001
		OUTPUT - $'$\n     1\ta$\n$\n     2\tb$'

	EXEC	--show-ends --number-nonblank --squeeze-blank d01010
		OUTPUT - $'$\n     1\ta$\n$\n     2\tb$\n$'

	EXEC	--show-ends --number-nonblank --squeeze-blank d01011
		OUTPUT - $'$\n     1\ta$\n$\n     2\tb$\n     3\tc$'

	EXEC	--show-ends --number-nonblank --squeeze-blank d01100
		OUTPUT - $'$\n     1\ta$\n     2\tb$\n$'

	EXEC	--show-ends --number-nonblank --squeeze-blank d01101
		OUTPUT - $'$\n     1\ta$\n     2\tb$\n$\n     3\tc$'

	EXEC	--show-ends --number-nonblank --squeeze-blank d01110
		OUTPUT - $'$\n     1\ta$\n     2\tb$\n     3\tc$\n$'

	EXEC	--show-ends --number-nonblank --squeeze-blank d01111
		OUTPUT - $'$\n     1\ta$\n     2\tb$\n     3\tc$\n     4\td$'

	EXEC	--show-ends --number-nonblank --squeeze-blank d10000
		OUTPUT - $'     1\ta$\n$'

	EXEC	--show-ends --number-nonblank --squeeze-blank d10001
		OUTPUT - $'     1\ta$\n$\n     2\tb$'

	EXEC	--show-ends --number-nonblank --squeeze-blank d10010
		OUTPUT - $'     1\ta$\n$\n     2\tb$\n$'

	EXEC	--show-ends --number-nonblank --squeeze-blank d10011
		OUTPUT - $'     1\ta$\n$\n     2\tb$\n     3\tc$'

	EXEC	--show-ends --number-nonblank --squeeze-blank d10100
		OUTPUT - $'     1\ta$\n$\n     2\tb$\n$'

	EXEC	--show-ends --number-nonblank --squeeze-blank d10101
		OUTPUT - $'     1\ta$\n$\n     2\tb$\n$\n     3\tc$'

	EXEC	--show-ends --number-nonblank --squeeze-blank d10110
		OUTPUT - $'     1\ta$\n$\n     2\tb$\n     3\tc$\n$'

	EXEC	--show-ends --number-nonblank --squeeze-blank d10111
		OUTPUT - $'     1\ta$\n$\n     2\tb$\n     3\tc$\n     4\td$'

	EXEC	--show-ends --number-nonblank --squeeze-blank d11000
		OUTPUT - $'     1\ta$\n     2\tb$\n$'

	EXEC	--show-ends --number-nonblank --squeeze-blank d11001
		OUTPUT - $'     1\ta$\n     2\tb$\n$\n     3\tc$'

	EXEC	--show-ends --number-nonblank --squeeze-blank d11010
		OUTPUT - $'     1\ta$\n     2\tb$\n$\n     3\tc$\n$'

	EXEC	--show-ends --number-nonblank --squeeze-blank d11011
		OUTPUT - $'     1\ta$\n     2\tb$\n$\n     3\tc$\n     4\td$'

	EXEC	--show-ends --number-nonblank --squeeze-blank d11100
		OUTPUT - $'     1\ta$\n     2\tb$\n     3\tc$\n$'

	EXEC	--show-ends --number-nonblank --squeeze-blank d11101
		OUTPUT - $'     1\ta$\n     2\tb$\n     3\tc$\n$\n     4\td$'

	EXEC	--show-ends --number-nonblank --squeeze-blank d11110
		OUTPUT - $'     1\ta$\n     2\tb$\n     3\tc$\n     4\td$\n$'

	EXEC	--show-ends --number-nonblank --squeeze-blank d11111
		OUTPUT - $'     1\ta$\n     2\tb$\n     3\tc$\n     4\td$\n     5\te$'

	EXEC	--show-nonprinting --number-nonblank --squeeze-blank d00000
		OUTPUT -n - $'\n'

	EXEC	--show-nonprinting --number-nonblank --squeeze-blank d00001
		OUTPUT - $'\n     1\ta'

	EXEC	--show-nonprinting --number-nonblank --squeeze-blank d00010
		OUTPUT - $'\n     1\ta\n'

	EXEC	--show-nonprinting --number-nonblank --squeeze-blank d00011
		OUTPUT - $'\n     1\ta\n     2\tb'

	EXEC	--show-nonprinting --number-nonblank --squeeze-blank d00100
		OUTPUT - $'\n     1\ta\n'

	EXEC	--show-nonprinting --number-nonblank --squeeze-blank d00101
		OUTPUT - $'\n     1\ta\n\n     2\tb'

	EXEC	--show-nonprinting --number-nonblank --squeeze-blank d00110
		OUTPUT - $'\n     1\ta\n     2\tb\n'

	EXEC	--show-nonprinting --number-nonblank --squeeze-blank d00111
		OUTPUT - $'\n     1\ta\n     2\tb\n     3\tc'

	EXEC	--show-nonprinting --number-nonblank --squeeze-blank d01000
		OUTPUT - $'\n     1\ta\n'

	EXEC	--show-nonprinting --number-nonblank --squeeze-blank d01001
		OUTPUT - $'\n     1\ta\n\n     2\tb'

	EXEC	--show-nonprinting --number-nonblank --squeeze-blank d01010
		OUTPUT - $'\n     1\ta\n\n     2\tb\n'

	EXEC	--show-nonprinting --number-nonblank --squeeze-blank d01011
		OUTPUT - $'\n     1\ta\n\n     2\tb\n     3\tc'

	EXEC	--show-nonprinting --number-nonblank --squeeze-blank d01100
		OUTPUT - $'\n     1\ta\n     2\tb\n'

	EXEC	--show-nonprinting --number-nonblank --squeeze-blank d01101
		OUTPUT - $'\n     1\ta\n     2\tb\n\n     3\tc'

	EXEC	--show-nonprinting --number-nonblank --squeeze-blank d01110
		OUTPUT - $'\n     1\ta\n     2\tb\n     3\tc\n'

	EXEC	--show-nonprinting --number-nonblank --squeeze-blank d01111
		OUTPUT - $'\n     1\ta\n     2\tb\n     3\tc\n     4\td'

	EXEC	--show-nonprinting --number-nonblank --squeeze-blank d10000
		OUTPUT - $'     1\ta\n'

	EXEC	--show-nonprinting --number-nonblank --squeeze-blank d10001
		OUTPUT - $'     1\ta\n\n     2\tb'

	EXEC	--show-nonprinting --number-nonblank --squeeze-blank d10010
		OUTPUT - $'     1\ta\n\n     2\tb\n'

	EXEC	--show-nonprinting --number-nonblank --squeeze-blank d10011
		OUTPUT - $'     1\ta\n\n     2\tb\n     3\tc'

	EXEC	--show-nonprinting --number-nonblank --squeeze-blank d10100
		OUTPUT - $'     1\ta\n\n     2\tb\n'

	EXEC	--show-nonprinting --number-nonblank --squeeze-blank d10101
		OUTPUT - $'     1\ta\n\n     2\tb\n\n     3\tc'

	EXEC	--show-nonprinting --number-nonblank --squeeze-blank d10110
		OUTPUT - $'     1\ta\n\n     2\tb\n     3\tc\n'

	EXEC	--show-nonprinting --number-nonblank --squeeze-blank d10111
		OUTPUT - $'     1\ta\n\n     2\tb\n     3\tc\n     4\td'

	EXEC	--show-nonprinting --number-nonblank --squeeze-blank d11000
		OUTPUT - $'     1\ta\n     2\tb\n'

	EXEC	--show-nonprinting --number-nonblank --squeeze-blank d11001
		OUTPUT - $'     1\ta\n     2\tb\n\n     3\tc'

	EXEC	--show-nonprinting --number-nonblank --squeeze-blank d11010
		OUTPUT - $'     1\ta\n     2\tb\n\n     3\tc\n'

	EXEC	--show-nonprinting --number-nonblank --squeeze-blank d11011
		OUTPUT - $'     1\ta\n     2\tb\n\n     3\tc\n     4\td'

	EXEC	--show-nonprinting --number-nonblank --squeeze-blank d11100
		OUTPUT - $'     1\ta\n     2\tb\n     3\tc\n'

	EXEC	--show-nonprinting --number-nonblank --squeeze-blank d11101
		OUTPUT - $'     1\ta\n     2\tb\n     3\tc\n\n     4\td'

	EXEC	--show-nonprinting --number-nonblank --squeeze-blank d11110
		OUTPUT - $'     1\ta\n     2\tb\n     3\tc\n     4\td\n'

	EXEC	--show-nonprinting --number-nonblank --squeeze-blank d11111
		OUTPUT - $'     1\ta\n     2\tb\n     3\tc\n     4\td\n     5\te'

	EXEC	--show-ends --show-nonprinting --number-nonblank --squeeze-blank d00000
		OUTPUT - '$'

	EXEC	--show-ends --show-nonprinting --number-nonblank --squeeze-blank d00001
		OUTPUT - $'$\n     1\ta$'

	EXEC	--show-ends --show-nonprinting --number-nonblank --squeeze-blank d00010
		OUTPUT - $'$\n     1\ta$\n$'

	EXEC	--show-ends --show-nonprinting --number-nonblank --squeeze-blank d00011
		OUTPUT - $'$\n     1\ta$\n     2\tb$'

	EXEC	--show-ends --show-nonprinting --number-nonblank --squeeze-blank d00100
		OUTPUT - $'$\n     1\ta$\n$'

	EXEC	--show-ends --show-nonprinting --number-nonblank --squeeze-blank d00101
		OUTPUT - $'$\n     1\ta$\n$\n     2\tb$'

	EXEC	--show-ends --show-nonprinting --number-nonblank --squeeze-blank d00110
		OUTPUT - $'$\n     1\ta$\n     2\tb$\n$'

	EXEC	--show-ends --show-nonprinting --number-nonblank --squeeze-blank d00111
		OUTPUT - $'$\n     1\ta$\n     2\tb$\n     3\tc$'

	EXEC	--show-ends --show-nonprinting --number-nonblank --squeeze-blank d01000
		OUTPUT - $'$\n     1\ta$\n$'

	EXEC	--show-ends --show-nonprinting --number-nonblank --squeeze-blank d01001
		OUTPUT - $'$\n     1\ta$\n$\n     2\tb$'

	EXEC	--show-ends --show-nonprinting --number-nonblank --squeeze-blank d01010
		OUTPUT - $'$\n     1\ta$\n$\n     2\tb$\n$'

	EXEC	--show-ends --show-nonprinting --number-nonblank --squeeze-blank d01011
		OUTPUT - $'$\n     1\ta$\n$\n     2\tb$\n     3\tc$'

	EXEC	--show-ends --show-nonprinting --number-nonblank --squeeze-blank d01100
		OUTPUT - $'$\n     1\ta$\n     2\tb$\n$'

	EXEC	--show-ends --show-nonprinting --number-nonblank --squeeze-blank d01101
		OUTPUT - $'$\n     1\ta$\n     2\tb$\n$\n     3\tc$'

	EXEC	--show-ends --show-nonprinting --number-nonblank --squeeze-blank d01110
		OUTPUT - $'$\n     1\ta$\n     2\tb$\n     3\tc$\n$'

	EXEC	--show-ends --show-nonprinting --number-nonblank --squeeze-blank d01111
		OUTPUT - $'$\n     1\ta$\n     2\tb$\n     3\tc$\n     4\td$'

	EXEC	--show-ends --show-nonprinting --number-nonblank --squeeze-blank d10000
		OUTPUT - $'     1\ta$\n$'

	EXEC	--show-ends --show-nonprinting --number-nonblank --squeeze-blank d10001
		OUTPUT - $'     1\ta$\n$\n     2\tb$'

	EXEC	--show-ends --show-nonprinting --number-nonblank --squeeze-blank d10010
		OUTPUT - $'     1\ta$\n$\n     2\tb$\n$'

	EXEC	--show-ends --show-nonprinting --number-nonblank --squeeze-blank d10011
		OUTPUT - $'     1\ta$\n$\n     2\tb$\n     3\tc$'

	EXEC	--show-ends --show-nonprinting --number-nonblank --squeeze-blank d10100
		OUTPUT - $'     1\ta$\n$\n     2\tb$\n$'

	EXEC	--show-ends --show-nonprinting --number-nonblank --squeeze-blank d10101
		OUTPUT - $'     1\ta$\n$\n     2\tb$\n$\n     3\tc$'

	EXEC	--show-ends --show-nonprinting --number-nonblank --squeeze-blank d10110
		OUTPUT - $'     1\ta$\n$\n     2\tb$\n     3\tc$\n$'

	EXEC	--show-ends --show-nonprinting --number-nonblank --squeeze-blank d10111
		OUTPUT - $'     1\ta$\n$\n     2\tb$\n     3\tc$\n     4\td$'

	EXEC	--show-ends --show-nonprinting --number-nonblank --squeeze-blank d11000
		OUTPUT - $'     1\ta$\n     2\tb$\n$'

	EXEC	--show-ends --show-nonprinting --number-nonblank --squeeze-blank d11001
		OUTPUT - $'     1\ta$\n     2\tb$\n$\n     3\tc$'

	EXEC	--show-ends --show-nonprinting --number-nonblank --squeeze-blank d11010
		OUTPUT - $'     1\ta$\n     2\tb$\n$\n     3\tc$\n$'

	EXEC	--show-ends --show-nonprinting --number-nonblank --squeeze-blank d11011
		OUTPUT - $'     1\ta$\n     2\tb$\n$\n     3\tc$\n     4\td$'

	EXEC	--show-ends --show-nonprinting --number-nonblank --squeeze-blank d11100
		OUTPUT - $'     1\ta$\n     2\tb$\n     3\tc$\n$'

	EXEC	--show-ends --show-nonprinting --number-nonblank --squeeze-blank d11101
		OUTPUT - $'     1\ta$\n     2\tb$\n     3\tc$\n$\n     4\td$'

	EXEC	--show-ends --show-nonprinting --number-nonblank --squeeze-blank d11110
		OUTPUT - $'     1\ta$\n     2\tb$\n     3\tc$\n     4\td$\n$'

	EXEC	--show-ends --show-nonprinting --number-nonblank --squeeze-blank d11111
		OUTPUT - $'     1\ta$\n     2\tb$\n     3\tc$\n     4\td$\n     5\te$'

TEST 02 -v

	DO getconf UNIVERSE = att

	EXEC	-v i
		INPUT -n -
		INPUT i $'a\x89b\xc9c\tdIe'
		INPUT -n j X
		OUTPUT - $'aM-\tbM-Ic\tdIe'
		ERROR -n -

	EXEC	-v j
		OUTPUT -n - X
