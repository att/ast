# : : generated from /home/gsf/src/cmd/builtin/cut.rt by mktest : : #

UNIT cut

TEST 01 '-d -f'

	EXEC	-d: -f1,3- f1
		INPUT -n -
		INPUT f1 a:b:c
		OUTPUT - a:c
		ERROR -n -

	EXEC	-d: -f1,3- f1

	EXEC	-d: -f2- f1
		OUTPUT - b:c

	EXEC	-d: -f4 f1
		OUTPUT -n - $'\n'

	EXEC	-d: -f4 f1

TEST 02 -c

	EXEC	-c4 f1
		INPUT -n -
		INPUT -n f1
		OUTPUT -
		ERROR -n -

	EXEC	-c4 f1
		INPUT f1 123
		OUTPUT -n - $'\n'

	EXEC	-c4 f1

	EXEC	-c4 f1
		INPUT f1 $'123\n1'
		OUTPUT - $'\n'

	EXEC	-c-5 f2
		INPUT f2 1234567890
		OUTPUT - 12345

	EXEC	-c1-5 f2

	EXEC	-c5- f2
		OUTPUT - 567890

	EXEC	-c5-10 f2

TEST 03 '-d -f -s'

	EXEC	-s -d: -f2,3 f1
		INPUT -n -
		INPUT f1 abc
		OUTPUT -
		ERROR -n -

	EXEC	-s -d: -f3- f1
		INPUT f1 a:b:c
		OUTPUT - c

	EXEC	-s -d: -f3-4 f1

	EXEC	-s -d: -f3,4 f1

	EXEC	-s -d: -f2,3 f1
		OUTPUT - b:c

	EXEC	-s -d: -f1,3 f1
		OUTPUT - a:c

TEST 04 '-d -f'

	EXEC	-d: -f1-3 f1
		INPUT -n -
		INPUT f1 :::
		OUTPUT - ::
		ERROR -n -

	EXEC	-d: -f1-4 f1
		INPUT f1 :::
		OUTPUT - :::

	EXEC	-d: -f2-3 f1
		OUTPUT - :

	EXEC	-d: -f2-4 f1
		OUTPUT - ::

	EXEC	'-d ' -f1-5 f5
		INPUT f5 '1 2 3 4 5'
		OUTPUT - '1 2 3 4 5'

	EXEC	'-d ' -f1 f5
		OUTPUT - 1

	EXEC	'-d ' -f2 f5
		OUTPUT - 2

	EXEC	'-d ' -f3 f5
		OUTPUT - 3

	EXEC	'-d ' -f4 f5
		OUTPUT - 4

	EXEC	'-d ' -f5 f5
		OUTPUT - 5

	EXEC	'-d ' -f1,2 f5
		OUTPUT - '1 2'

	EXEC	'-d ' -f1,3 f5
		OUTPUT - '1 3'

	EXEC	'-d ' -f1,4 f5
		OUTPUT - '1 4'

	EXEC	'-d ' -f1,5 f5
		OUTPUT - '1 5'

	EXEC	'-d ' -f2,3 f5
		OUTPUT - '2 3'

	EXEC	'-d ' -f2,4 f5
		OUTPUT - '2 4'

	EXEC	'-d ' -f2,5 f5
		OUTPUT - '2 5'

	EXEC	'-d ' -f2,4 f5
		OUTPUT - '2 4'

	EXEC	'-d ' -f3,5 f5
		OUTPUT - '3 5'

	EXEC	'-d ' -f4,5 f5
		OUTPUT - '4 5'

TEST 05 '-d -f -s'

	EXEC	-s -d: -f1-3 f1
		INPUT -n -
		INPUT f1 :::
		OUTPUT - ::
		ERROR -n -

	EXEC	-s -d: -f1-4 f1
		INPUT f1 :::
		OUTPUT - :::

	EXEC	-s -d: -f2-3 f1
		INPUT f1 :::
		OUTPUT - :

	EXEC	-s -d: -f2-4 f1
		INPUT f1 :::
		OUTPUT - ::

	EXEC	-s -d: -f2-4 f1
		INPUT f1 $':::\n:'
		OUTPUT - $'::\n'

	EXEC	-s -d: -f2-4 f1
		INPUT f1 $':::\n:1'
		OUTPUT - $'::\n1'

	EXEC	-s -d: -f1-4 f1
		INPUT f1 $':::\n:a'
		OUTPUT - $':::\n:a'

	EXEC	-s -d: -f3- f1
		INPUT f1 $':::\n:1'
		OUTPUT - $':\n'

TEST 07 'no fields'

	EXEC	-f3- f1
		INPUT -n -
		INPUT -n f1
		OUTPUT -
		ERROR -n -

	EXEC	-s -f3- f1

	EXEC	-b 1 f1

TEST 08 'two empty fields'

	EXEC	-s -d: -f2-4 f1
		INPUT -n -
		INPUT f1 :
		OUTPUT -n - $'\n'
		ERROR -n -

TEST 09 'fixed length records'

	EXEC	-r5 -b2-4 f1
		INPUT -n -
		INPUT -n 'f1 abcdefghij'
		OUTPUT -
		ERROR - 'cut: f1: cannot open [No such file or directory]'
		EXIT 1

	EXEC	-N -r5 -b2-4 f1

TEST 10 'misc errors'

DIAGNOSTICS

	EXEC
		INPUT -n -
		OUTPUT -
		ERROR - $'cut: b, c or f option must be specified
Usage: cut [-nsN] [-b list] [-c list] [-d delim] [-f list] [-R|r reclen]
           [-D ldelim] [file ...]'
		EXIT 2

	EXEC	-b
		ERROR - $'cut: -b: list argument expected
Usage: cut [-nsN] [-b list] [-c list] [-d delim] [-f list] [-R|r reclen]
           [-D ldelim] [file ...]'

	EXEC	-f
		ERROR - $'cut: -f: list argument expected
Usage: cut [-nsN] [-b list] [-c list] [-d delim] [-f list] [-R|r reclen]
           [-D ldelim] [file ...]'

	EXEC	f1
		INPUT f1 :
		ERROR - $'cut: b, c or f option must be specified
Usage: cut [-nsN] [-b list] [-c list] [-d delim] [-f list] [-R|r reclen]
           [-D ldelim] [file ...]'

	EXEC	-b '' f1
		ERROR - 'cut: non-empty b, c or f option must be specified'
		EXIT 1

	EXEC	-f '' f1

	EXEC	-s -b4 f1
		ERROR - 'cut: s option requires f option'

TEST 11 'multibyte UTF-8 -- off-by-oners doomed here'

EXPORT LC_CTYPE=C.UTF-8

	EXEC	-d $'\xe2\x82\xac' -f1 a.dat
		INPUT -n -
		INPUT a.dat $'\xc3\x9f\xe2\x82\xac\xe2\x82\xacf2\xe2\x82\xac\xc3\xbc'
		OUTPUT - $'\xc3\x9f'
		ERROR -n -

	EXEC	-b 1-2 b.dat
		INPUT b.dat $'\xc3\x9f\xe2\x82\xac\xe2\x82\xacf2\xe2\x82\xac\xc3\xbc'
		INPUT c.dat $'\xc3\x9f\xe2\x82\xac\xe2\x82\xacf2\xe2\x82\xac\xc3\xbc\n'

	EXEC	-b 1-2 c.dat
		OUTPUT - $'\xc3\x9f\n'

	EXEC	-n -b 1-2 b.dat
		OUTPUT - $'\xc3\x9f'

	EXEC	-n -b 1-2 c.dat
		OUTPUT - $'\xc3\x9f\n'

	EXEC	-b 1-3 b.dat
		OUTPUT - $'\xc3\x9f\xe2'

	EXEC	-b 1-3 c.dat
		OUTPUT - $'\xc3\x9f\xe2\n'

	EXEC	-n -b 1-3 b.dat
		OUTPUT - $'\xc3\x9f'

	EXEC	-n -b 1-3 c.dat
		OUTPUT - $'\xc3\x9f\n'

	EXEC	-b 1-4 b.dat
		OUTPUT - $'\xc3\x9f\xe2\x82'

	EXEC	-b 1-4 c.dat
		OUTPUT - $'\xc3\x9f\xe2\x82\n'

	EXEC	-n -b 1-4 b.dat
		OUTPUT - $'\xc3\x9f'

	EXEC	-n -b 1-4 c.dat
		OUTPUT - $'\xc3\x9f\n'

	EXEC	-b 1-5 b.dat
		OUTPUT - $'\xc3\x9f\xe2\x82\xac'

	EXEC	-b 1-5 c.dat
		OUTPUT - $'\xc3\x9f\xe2\x82\xac\n'

	EXEC	-n -b 1-5 b.dat
		OUTPUT - $'\xc3\x9f\xe2\x82\xac'

	EXEC	-n -b 1-5 c.dat
		OUTPUT - $'\xc3\x9f\xe2\x82\xac\n'

	EXEC	-b 1-6 b.dat
		OUTPUT - $'\xc3\x9f\xe2\x82\xac\xe2'

	EXEC	-b 1-6 c.dat
		OUTPUT - $'\xc3\x9f\xe2\x82\xac\xe2\n'

	EXEC	-n -b 1-6 b.dat
		OUTPUT - $'\xc3\x9f\xe2\x82\xac'

	EXEC	-n -b 1-6 c.dat
		OUTPUT - $'\xc3\x9f\xe2\x82\xac\n'

	EXEC	-b 1-7 b.dat
		OUTPUT - $'\xc3\x9f\xe2\x82\xac\xe2\x82'

	EXEC	-b 1-7 c.dat
		OUTPUT - $'\xc3\x9f\xe2\x82\xac\xe2\x82\n'

	EXEC	-n -b 1-7 b.dat
		OUTPUT - $'\xc3\x9f\xe2\x82\xac'

	EXEC	-n -b 1-7 c.dat
		OUTPUT - $'\xc3\x9f\xe2\x82\xac\n'

	EXEC	-b 1-8 b.dat
		OUTPUT - $'\xc3\x9f\xe2\x82\xac\xe2\x82\xac'

	EXEC	-b 1-8 c.dat
		OUTPUT - $'\xc3\x9f\xe2\x82\xac\xe2\x82\xac\n'

	EXEC	-n -b 1-8 b.dat
		OUTPUT - $'\xc3\x9f\xe2\x82\xac\xe2\x82\xac'

	EXEC	-n -b 1-8 c.dat
		OUTPUT - $'\xc3\x9f\xe2\x82\xac\xe2\x82\xac\n'

	EXEC	-b 1-9 b.dat
		OUTPUT - $'\xc3\x9f\xe2\x82\xac\xe2\x82\xacf'

	EXEC	-b 1-9 c.dat
		OUTPUT - $'\xc3\x9f\xe2\x82\xac\xe2\x82\xacf\n'

	EXEC	-n -b 1-9 b.dat
		OUTPUT - $'\xc3\x9f\xe2\x82\xac\xe2\x82\xacf'

	EXEC	-n -b 1-9 c.dat
		OUTPUT - $'\xc3\x9f\xe2\x82\xac\xe2\x82\xacf\n'

	EXEC	-b 1-10 b.dat
		OUTPUT - $'\xc3\x9f\xe2\x82\xac\xe2\x82\xacf2'

	EXEC	-b 1-10 c.dat
		OUTPUT - $'\xc3\x9f\xe2\x82\xac\xe2\x82\xacf2\n'

	EXEC	-n -b 1-10 b.dat
		OUTPUT - $'\xc3\x9f\xe2\x82\xac\xe2\x82\xacf2'

	EXEC	-n -b 1-10 c.dat
		OUTPUT - $'\xc3\x9f\xe2\x82\xac\xe2\x82\xacf2\n'

	EXEC	-b 1-11 b.dat
		OUTPUT - $'\xc3\x9f\xe2\x82\xac\xe2\x82\xacf2\xe2'

	EXEC	-b 1-11 c.dat
		OUTPUT - $'\xc3\x9f\xe2\x82\xac\xe2\x82\xacf2\xe2\n'

	EXEC	-n -b 1-11 b.dat
		OUTPUT - $'\xc3\x9f\xe2\x82\xac\xe2\x82\xacf2'

	EXEC	-n -b 1-11 c.dat
		OUTPUT - $'\xc3\x9f\xe2\x82\xac\xe2\x82\xacf2\n'

	EXEC	-b 1-12 b.dat
		OUTPUT - $'\xc3\x9f\xe2\x82\xac\xe2\x82\xacf2\xe2\x82'

	EXEC	-b 1-12 c.dat
		OUTPUT - $'\xc3\x9f\xe2\x82\xac\xe2\x82\xacf2\xe2\x82\n'

	EXEC	-n -b 1-12 b.dat
		OUTPUT - $'\xc3\x9f\xe2\x82\xac\xe2\x82\xacf2'

	EXEC	-n -b 1-12 c.dat
		OUTPUT - $'\xc3\x9f\xe2\x82\xac\xe2\x82\xacf2\n'

	EXEC	-b 1-13 b.dat
		OUTPUT - $'\xc3\x9f\xe2\x82\xac\xe2\x82\xacf2\xe2\x82\xac'

	EXEC	-b 1-13 c.dat
		OUTPUT - $'\xc3\x9f\xe2\x82\xac\xe2\x82\xacf2\xe2\x82\xac\n'

	EXEC	-n -b 1-13 b.dat
		OUTPUT - $'\xc3\x9f\xe2\x82\xac\xe2\x82\xacf2\xe2\x82\xac'

	EXEC	-n -b 1-13 c.dat
		OUTPUT - $'\xc3\x9f\xe2\x82\xac\xe2\x82\xacf2\xe2\x82\xac\n'

	EXEC	-b 1-14 b.dat
		OUTPUT - $'\xc3\x9f\xe2\x82\xac\xe2\x82\xacf2\xe2\x82\xac\xc3'

	EXEC	-b 1-14 c.dat
		OUTPUT - $'\xc3\x9f\xe2\x82\xac\xe2\x82\xacf2\xe2\x82\xac\xc3\n'

	EXEC	-n -b 1-14 b.dat
		OUTPUT - $'\xc3\x9f\xe2\x82\xac\xe2\x82\xacf2\xe2\x82\xac'

	EXEC	-n -b 1-14 c.dat
		OUTPUT - $'\xc3\x9f\xe2\x82\xac\xe2\x82\xacf2\xe2\x82\xac\n'

	EXEC	-b 1-15 b.dat
		OUTPUT - $'\xc3\x9f\xe2\x82\xac\xe2\x82\xacf2\xe2\x82\xac\xc3\xbc'

	EXEC	-b 1-15 c.dat
		OUTPUT - $'\xc3\x9f\xe2\x82\xac\xe2\x82\xacf2\xe2\x82\xac\xc3\xbc\n'

	EXEC	-n -b 1-15 b.dat
		OUTPUT - $'\xc3\x9f\xe2\x82\xac\xe2\x82\xacf2\xe2\x82\xac\xc3\xbc'

	EXEC	-n -b 1-15 c.dat
		OUTPUT - $'\xc3\x9f\xe2\x82\xac\xe2\x82\xacf2\xe2\x82\xac\xc3\xbc\n'

	EXEC	-b 1-16 b.dat
		OUTPUT - $'\xc3\x9f\xe2\x82\xac\xe2\x82\xacf2\xe2\x82\xac\xc3\xbc'

	EXEC	-b 1-16 c.dat
		OUTPUT - $'\xc3\x9f\xe2\x82\xac\xe2\x82\xacf2\xe2\x82\xac\xc3\xbc\n'

	EXEC	-n -b 1-16 b.dat
		OUTPUT - $'\xc3\x9f\xe2\x82\xac\xe2\x82\xacf2\xe2\x82\xac\xc3\xbc'

	EXEC	-n -b 1-16 c.dat
		OUTPUT - $'\xc3\x9f\xe2\x82\xac\xe2\x82\xacf2\xe2\x82\xac\xc3\xbc\n'

	EXEC	-b 2-3 b.dat
		OUTPUT - $'\x9f\xe2'

	EXEC	-b 2-3 c.dat
		OUTPUT - $'\x9f\xe2\n'

	EXEC	-n -b 2-3 b.dat
		OUTPUT - $'\xc3\x9f'

	EXEC	-n -b 2-3 c.dat
		OUTPUT - $'\xc3\x9f\n'

	EXEC	-b 2-4 b.dat
		OUTPUT - $'\x9f\xe2\x82'

	EXEC	-b 2-4 c.dat
		OUTPUT - $'\x9f\xe2\x82\n'

	EXEC	-n -b 2-4 b.dat
		OUTPUT - $'\xc3\x9f'

	EXEC	-n -b 2-4 c.dat
		OUTPUT - $'\xc3\x9f\n'

	EXEC	-b 2-5 b.dat
		OUTPUT - $'\x9f\xe2\x82\xac'

	EXEC	-b 2-5 c.dat
		OUTPUT - $'\x9f\xe2\x82\xac\n'

	EXEC	-n -b 2-5 b.dat
		OUTPUT - $'\xc3\x9f\xe2\x82\xac'

	EXEC	-n -b 2-5 c.dat
		OUTPUT - $'\xc3\x9f\xe2\x82\xac\n'

	EXEC	-b 2-6 b.dat
		OUTPUT - $'\x9f\xe2\x82\xac\xe2'

	EXEC	-b 2-6 c.dat
		OUTPUT - $'\x9f\xe2\x82\xac\xe2\n'

	EXEC	-n -b 2-6 b.dat
		OUTPUT - $'\xc3\x9f\xe2\x82\xac'

	EXEC	-n -b 2-6 c.dat
		OUTPUT - $'\xc3\x9f\xe2\x82\xac\n'

	EXEC	-b 2-7 b.dat
		OUTPUT - $'\x9f\xe2\x82\xac\xe2\x82'

	EXEC	-b 2-7 c.dat
		OUTPUT - $'\x9f\xe2\x82\xac\xe2\x82\n'

	EXEC	-n -b 2-7 b.dat
		OUTPUT - $'\xc3\x9f\xe2\x82\xac'

	EXEC	-n -b 2-7 c.dat
		OUTPUT - $'\xc3\x9f\xe2\x82\xac\n'

	EXEC	-b 2-8 b.dat
		OUTPUT - $'\x9f\xe2\x82\xac\xe2\x82\xac'

	EXEC	-b 2-8 c.dat
		OUTPUT - $'\x9f\xe2\x82\xac\xe2\x82\xac\n'

	EXEC	-n -b 2-8 b.dat
		OUTPUT - $'\xc3\x9f\xe2\x82\xac\xe2\x82\xac'

	EXEC	-n -b 2-8 c.dat
		OUTPUT - $'\xc3\x9f\xe2\x82\xac\xe2\x82\xac\n'

	EXEC	-b 2-9 b.dat
		OUTPUT - $'\x9f\xe2\x82\xac\xe2\x82\xacf'

	EXEC	-b 2-9 c.dat
		OUTPUT - $'\x9f\xe2\x82\xac\xe2\x82\xacf\n'

	EXEC	-n -b 2-9 b.dat
		OUTPUT - $'\xc3\x9f\xe2\x82\xac\xe2\x82\xacf'

	EXEC	-n -b 2-9 c.dat
		OUTPUT - $'\xc3\x9f\xe2\x82\xac\xe2\x82\xacf\n'

	EXEC	-b 2-10 b.dat
		OUTPUT - $'\x9f\xe2\x82\xac\xe2\x82\xacf2'

	EXEC	-b 2-10 c.dat
		OUTPUT - $'\x9f\xe2\x82\xac\xe2\x82\xacf2\n'

	EXEC	-n -b 2-10 b.dat
		OUTPUT - $'\xc3\x9f\xe2\x82\xac\xe2\x82\xacf2'

	EXEC	-n -b 2-10 c.dat
		OUTPUT - $'\xc3\x9f\xe2\x82\xac\xe2\x82\xacf2\n'

	EXEC	-b 2-11 b.dat
		OUTPUT - $'\x9f\xe2\x82\xac\xe2\x82\xacf2\xe2'

	EXEC	-b 2-11 c.dat
		OUTPUT - $'\x9f\xe2\x82\xac\xe2\x82\xacf2\xe2\n'

	EXEC	-n -b 2-11 b.dat
		OUTPUT - $'\xc3\x9f\xe2\x82\xac\xe2\x82\xacf2'

	EXEC	-n -b 2-11 c.dat
		OUTPUT - $'\xc3\x9f\xe2\x82\xac\xe2\x82\xacf2\n'

	EXEC	-b 2-12 b.dat
		OUTPUT - $'\x9f\xe2\x82\xac\xe2\x82\xacf2\xe2\x82'

	EXEC	-b 2-12 c.dat
		OUTPUT - $'\x9f\xe2\x82\xac\xe2\x82\xacf2\xe2\x82\n'

	EXEC	-n -b 2-12 b.dat
		OUTPUT - $'\xc3\x9f\xe2\x82\xac\xe2\x82\xacf2'

	EXEC	-n -b 2-12 c.dat
		OUTPUT - $'\xc3\x9f\xe2\x82\xac\xe2\x82\xacf2\n'

	EXEC	-b 2-13 b.dat
		OUTPUT - $'\x9f\xe2\x82\xac\xe2\x82\xacf2\xe2\x82\xac'

	EXEC	-b 2-13 c.dat
		OUTPUT - $'\x9f\xe2\x82\xac\xe2\x82\xacf2\xe2\x82\xac\n'

	EXEC	-n -b 2-13 b.dat
		OUTPUT - $'\xc3\x9f\xe2\x82\xac\xe2\x82\xacf2\xe2\x82\xac'

	EXEC	-n -b 2-13 c.dat
		OUTPUT - $'\xc3\x9f\xe2\x82\xac\xe2\x82\xacf2\xe2\x82\xac\n'

	EXEC	-b 2-14 b.dat
		OUTPUT - $'\x9f\xe2\x82\xac\xe2\x82\xacf2\xe2\x82\xac\xc3'

	EXEC	-b 2-14 c.dat
		OUTPUT - $'\x9f\xe2\x82\xac\xe2\x82\xacf2\xe2\x82\xac\xc3\n'

	EXEC	-n -b 2-14 b.dat
		OUTPUT - $'\xc3\x9f\xe2\x82\xac\xe2\x82\xacf2\xe2\x82\xac'

	EXEC	-n -b 2-14 c.dat
		OUTPUT - $'\xc3\x9f\xe2\x82\xac\xe2\x82\xacf2\xe2\x82\xac\n'

	EXEC	-b 2-15 b.dat
		OUTPUT - $'\x9f\xe2\x82\xac\xe2\x82\xacf2\xe2\x82\xac\xc3\xbc'

	EXEC	-b 2-15 c.dat
		OUTPUT - $'\x9f\xe2\x82\xac\xe2\x82\xacf2\xe2\x82\xac\xc3\xbc\n'

	EXEC	-n -b 2-15 b.dat
		OUTPUT - $'\xc3\x9f\xe2\x82\xac\xe2\x82\xacf2\xe2\x82\xac\xc3\xbc'

	EXEC	-n -b 2-15 c.dat
		OUTPUT - $'\xc3\x9f\xe2\x82\xac\xe2\x82\xacf2\xe2\x82\xac\xc3\xbc\n'

	EXEC	-b 2-16 b.dat
		OUTPUT - $'\x9f\xe2\x82\xac\xe2\x82\xacf2\xe2\x82\xac\xc3\xbc'

	EXEC	-b 2-16 c.dat
		OUTPUT - $'\x9f\xe2\x82\xac\xe2\x82\xacf2\xe2\x82\xac\xc3\xbc\n'

	EXEC	-n -b 2-16 b.dat
		OUTPUT - $'\xc3\x9f\xe2\x82\xac\xe2\x82\xacf2\xe2\x82\xac\xc3\xbc'

	EXEC	-n -b 2-16 c.dat
		OUTPUT - $'\xc3\x9f\xe2\x82\xac\xe2\x82\xacf2\xe2\x82\xac\xc3\xbc\n'

	EXEC	-b 3-4 b.dat
		OUTPUT - $'\xe2\x82'

	EXEC	-b 3-4 c.dat
		OUTPUT - $'\xe2\x82\n'

	EXEC	-n -b 3-4 b.dat
		OUTPUT -n - $'\n'

	EXEC	-n -b 3-4 c.dat
		OUTPUT - $'\n'

	EXEC	-b 3-5 b.dat
		OUTPUT - $'\xe2\x82\xac'

	EXEC	-b 3-5 c.dat
		OUTPUT - $'\xe2\x82\xac\n'

	EXEC	-n -b 3-5 b.dat
		OUTPUT - $'\xe2\x82\xac'

	EXEC	-n -b 3-5 c.dat
		OUTPUT - $'\xe2\x82\xac\n'

	EXEC	-b 3-6 b.dat
		OUTPUT - $'\xe2\x82\xac\xe2'

	EXEC	-b 3-6 c.dat
		OUTPUT - $'\xe2\x82\xac\xe2\n'

	EXEC	-n -b 3-6 b.dat
		OUTPUT - $'\xe2\x82\xac'

	EXEC	-n -b 3-6 c.dat
		OUTPUT - $'\xe2\x82\xac\n'

	EXEC	-b 3-7 b.dat
		OUTPUT - $'\xe2\x82\xac\xe2\x82'

	EXEC	-b 3-7 c.dat
		OUTPUT - $'\xe2\x82\xac\xe2\x82\n'

	EXEC	-n -b 3-7 b.dat
		OUTPUT - $'\xe2\x82\xac'

	EXEC	-n -b 3-7 c.dat
		OUTPUT - $'\xe2\x82\xac\n'

	EXEC	-b 3-8 b.dat
		OUTPUT - $'\xe2\x82\xac\xe2\x82\xac'

	EXEC	-b 3-8 c.dat
		OUTPUT - $'\xe2\x82\xac\xe2\x82\xac\n'

	EXEC	-n -b 3-8 b.dat
		OUTPUT - $'\xe2\x82\xac\xe2\x82\xac'

	EXEC	-n -b 3-8 c.dat
		OUTPUT - $'\xe2\x82\xac\xe2\x82\xac\n'

	EXEC	-b 3-9 b.dat
		OUTPUT - $'\xe2\x82\xac\xe2\x82\xacf'

	EXEC	-b 3-9 c.dat
		OUTPUT - $'\xe2\x82\xac\xe2\x82\xacf\n'

	EXEC	-n -b 3-9 b.dat
		OUTPUT - $'\xe2\x82\xac\xe2\x82\xacf'

	EXEC	-n -b 3-9 c.dat
		OUTPUT - $'\xe2\x82\xac\xe2\x82\xacf\n'

	EXEC	-b 3-10 b.dat
		OUTPUT - $'\xe2\x82\xac\xe2\x82\xacf2'

	EXEC	-b 3-10 c.dat
		OUTPUT - $'\xe2\x82\xac\xe2\x82\xacf2\n'

	EXEC	-n -b 3-10 b.dat
		OUTPUT - $'\xe2\x82\xac\xe2\x82\xacf2'

	EXEC	-n -b 3-10 c.dat
		OUTPUT - $'\xe2\x82\xac\xe2\x82\xacf2\n'

	EXEC	-b 3-11 b.dat
		OUTPUT - $'\xe2\x82\xac\xe2\x82\xacf2\xe2'

	EXEC	-b 3-11 c.dat
		OUTPUT - $'\xe2\x82\xac\xe2\x82\xacf2\xe2\n'

	EXEC	-n -b 3-11 b.dat
		OUTPUT - $'\xe2\x82\xac\xe2\x82\xacf2'

	EXEC	-n -b 3-11 c.dat
		OUTPUT - $'\xe2\x82\xac\xe2\x82\xacf2\n'

	EXEC	-b 3-12 b.dat
		OUTPUT - $'\xe2\x82\xac\xe2\x82\xacf2\xe2\x82'

	EXEC	-b 3-12 c.dat
		OUTPUT - $'\xe2\x82\xac\xe2\x82\xacf2\xe2\x82\n'

	EXEC	-n -b 3-12 b.dat
		OUTPUT - $'\xe2\x82\xac\xe2\x82\xacf2'

	EXEC	-n -b 3-12 c.dat
		OUTPUT - $'\xe2\x82\xac\xe2\x82\xacf2\n'

	EXEC	-b 3-13 b.dat
		OUTPUT - $'\xe2\x82\xac\xe2\x82\xacf2\xe2\x82\xac'

	EXEC	-b 3-13 c.dat
		OUTPUT - $'\xe2\x82\xac\xe2\x82\xacf2\xe2\x82\xac\n'

	EXEC	-n -b 3-13 b.dat
		OUTPUT - $'\xe2\x82\xac\xe2\x82\xacf2\xe2\x82\xac'

	EXEC	-n -b 3-13 c.dat
		OUTPUT - $'\xe2\x82\xac\xe2\x82\xacf2\xe2\x82\xac\n'

	EXEC	-b 3-14 b.dat
		OUTPUT - $'\xe2\x82\xac\xe2\x82\xacf2\xe2\x82\xac\xc3'

	EXEC	-b 3-14 c.dat
		OUTPUT - $'\xe2\x82\xac\xe2\x82\xacf2\xe2\x82\xac\xc3\n'

	EXEC	-n -b 3-14 b.dat
		OUTPUT - $'\xe2\x82\xac\xe2\x82\xacf2\xe2\x82\xac'

	EXEC	-n -b 3-14 c.dat
		OUTPUT - $'\xe2\x82\xac\xe2\x82\xacf2\xe2\x82\xac\n'

	EXEC	-b 3-15 b.dat
		OUTPUT - $'\xe2\x82\xac\xe2\x82\xacf2\xe2\x82\xac\xc3\xbc'

	EXEC	-b 3-15 c.dat
		OUTPUT - $'\xe2\x82\xac\xe2\x82\xacf2\xe2\x82\xac\xc3\xbc\n'

	EXEC	-n -b 3-15 b.dat
		OUTPUT - $'\xe2\x82\xac\xe2\x82\xacf2\xe2\x82\xac\xc3\xbc'

	EXEC	-n -b 3-15 c.dat
		OUTPUT - $'\xe2\x82\xac\xe2\x82\xacf2\xe2\x82\xac\xc3\xbc\n'

	EXEC	-b 3-16 b.dat
		OUTPUT - $'\xe2\x82\xac\xe2\x82\xacf2\xe2\x82\xac\xc3\xbc'

	EXEC	-b 3-16 c.dat
		OUTPUT - $'\xe2\x82\xac\xe2\x82\xacf2\xe2\x82\xac\xc3\xbc\n'

	EXEC	-n -b 3-16 b.dat
		OUTPUT - $'\xe2\x82\xac\xe2\x82\xacf2\xe2\x82\xac\xc3\xbc'

	EXEC	-n -b 3-16 c.dat
		OUTPUT - $'\xe2\x82\xac\xe2\x82\xacf2\xe2\x82\xac\xc3\xbc\n'

	EXEC	-b 4-5 b.dat
		OUTPUT - $'\x82\xac'

	EXEC	-b 4-5 c.dat
		OUTPUT - $'\x82\xac\n'

	EXEC	-n -b 4-5 b.dat
		OUTPUT - $'\xe2\x82\xac'

	EXEC	-n -b 4-5 c.dat
		OUTPUT - $'\xe2\x82\xac\n'

	EXEC	-b 4-6 b.dat
		OUTPUT - $'\x82\xac\xe2'

	EXEC	-b 4-6 c.dat
		OUTPUT - $'\x82\xac\xe2\n'

	EXEC	-n -b 4-6 b.dat
		OUTPUT - $'\xe2\x82\xac'

	EXEC	-n -b 4-6 c.dat
		OUTPUT - $'\xe2\x82\xac\n'

	EXEC	-b 4-7 b.dat
		OUTPUT - $'\x82\xac\xe2\x82'

	EXEC	-b 4-7 c.dat
		OUTPUT - $'\x82\xac\xe2\x82\n'

	EXEC	-n -b 4-7 b.dat
		OUTPUT - $'\xe2\x82\xac'

	EXEC	-n -b 4-7 c.dat
		OUTPUT - $'\xe2\x82\xac\n'

	EXEC	-b 4-8 b.dat
		OUTPUT - $'\x82\xac\xe2\x82\xac'

	EXEC	-b 4-8 c.dat
		OUTPUT - $'\x82\xac\xe2\x82\xac\n'

	EXEC	-n -b 4-8 b.dat
		OUTPUT - $'\xe2\x82\xac\xe2\x82\xac'

	EXEC	-n -b 4-8 c.dat
		OUTPUT - $'\xe2\x82\xac\xe2\x82\xac\n'

	EXEC	-b 4-9 b.dat
		OUTPUT - $'\x82\xac\xe2\x82\xacf'

	EXEC	-b 4-9 c.dat
		OUTPUT - $'\x82\xac\xe2\x82\xacf\n'

	EXEC	-n -b 4-9 b.dat
		OUTPUT - $'\xe2\x82\xac\xe2\x82\xacf'

	EXEC	-n -b 4-9 c.dat
		OUTPUT - $'\xe2\x82\xac\xe2\x82\xacf\n'

	EXEC	-b 4-10 b.dat
		OUTPUT - $'\x82\xac\xe2\x82\xacf2'

	EXEC	-b 4-10 c.dat
		OUTPUT - $'\x82\xac\xe2\x82\xacf2\n'

	EXEC	-n -b 4-10 b.dat
		OUTPUT - $'\xe2\x82\xac\xe2\x82\xacf2'

	EXEC	-n -b 4-10 c.dat
		OUTPUT - $'\xe2\x82\xac\xe2\x82\xacf2\n'

	EXEC	-b 4-11 b.dat
		OUTPUT - $'\x82\xac\xe2\x82\xacf2\xe2'

	EXEC	-b 4-11 c.dat
		OUTPUT - $'\x82\xac\xe2\x82\xacf2\xe2\n'

	EXEC	-n -b 4-11 b.dat
		OUTPUT - $'\xe2\x82\xac\xe2\x82\xacf2'

	EXEC	-n -b 4-11 c.dat
		OUTPUT - $'\xe2\x82\xac\xe2\x82\xacf2\n'

	EXEC	-b 4-12 b.dat
		OUTPUT - $'\x82\xac\xe2\x82\xacf2\xe2\x82'

	EXEC	-b 4-12 c.dat
		OUTPUT - $'\x82\xac\xe2\x82\xacf2\xe2\x82\n'

	EXEC	-n -b 4-12 b.dat
		OUTPUT - $'\xe2\x82\xac\xe2\x82\xacf2'

	EXEC	-n -b 4-12 c.dat
		OUTPUT - $'\xe2\x82\xac\xe2\x82\xacf2\n'

	EXEC	-b 4-13 b.dat
		OUTPUT - $'\x82\xac\xe2\x82\xacf2\xe2\x82\xac'

	EXEC	-b 4-13 c.dat
		OUTPUT - $'\x82\xac\xe2\x82\xacf2\xe2\x82\xac\n'

	EXEC	-n -b 4-13 b.dat
		OUTPUT - $'\xe2\x82\xac\xe2\x82\xacf2\xe2\x82\xac'

	EXEC	-n -b 4-13 c.dat
		OUTPUT - $'\xe2\x82\xac\xe2\x82\xacf2\xe2\x82\xac\n'

	EXEC	-b 4-14 b.dat
		OUTPUT - $'\x82\xac\xe2\x82\xacf2\xe2\x82\xac\xc3'

	EXEC	-b 4-14 c.dat
		OUTPUT - $'\x82\xac\xe2\x82\xacf2\xe2\x82\xac\xc3\n'

	EXEC	-n -b 4-14 b.dat
		OUTPUT - $'\xe2\x82\xac\xe2\x82\xacf2\xe2\x82\xac'

	EXEC	-n -b 4-14 c.dat
		OUTPUT - $'\xe2\x82\xac\xe2\x82\xacf2\xe2\x82\xac\n'

	EXEC	-b 4-15 b.dat
		OUTPUT - $'\x82\xac\xe2\x82\xacf2\xe2\x82\xac\xc3\xbc'

	EXEC	-b 4-15 c.dat
		OUTPUT - $'\x82\xac\xe2\x82\xacf2\xe2\x82\xac\xc3\xbc\n'

	EXEC	-n -b 4-15 b.dat
		OUTPUT - $'\xe2\x82\xac\xe2\x82\xacf2\xe2\x82\xac\xc3\xbc'

	EXEC	-n -b 4-15 c.dat
		OUTPUT - $'\xe2\x82\xac\xe2\x82\xacf2\xe2\x82\xac\xc3\xbc\n'

	EXEC	-b 4-16 b.dat
		OUTPUT - $'\x82\xac\xe2\x82\xacf2\xe2\x82\xac\xc3\xbc'

	EXEC	-b 4-16 c.dat
		OUTPUT - $'\x82\xac\xe2\x82\xacf2\xe2\x82\xac\xc3\xbc\n'

	EXEC	-n -b 4-16 b.dat
		OUTPUT - $'\xe2\x82\xac\xe2\x82\xacf2\xe2\x82\xac\xc3\xbc'

	EXEC	-n -b 4-16 c.dat
		OUTPUT - $'\xe2\x82\xac\xe2\x82\xacf2\xe2\x82\xac\xc3\xbc\n'

	EXEC	-b 5-6 b.dat
		OUTPUT - $'\xac\xe2'

	EXEC	-b 5-6 c.dat
		OUTPUT - $'\xac\xe2\n'

	EXEC	-n -b 5-6 b.dat
		OUTPUT - $'\xe2\x82\xac'

	EXEC	-n -b 5-6 c.dat
		OUTPUT - $'\xe2\x82\xac\n'

	EXEC	-b 5-7 b.dat
		OUTPUT - $'\xac\xe2\x82'

	EXEC	-b 5-7 c.dat
		OUTPUT - $'\xac\xe2\x82\n'

	EXEC	-n -b 5-7 b.dat
		OUTPUT - $'\xe2\x82\xac'

	EXEC	-n -b 5-7 c.dat
		OUTPUT - $'\xe2\x82\xac\n'

	EXEC	-b 5-8 b.dat
		OUTPUT - $'\xac\xe2\x82\xac'

	EXEC	-b 5-8 c.dat
		OUTPUT - $'\xac\xe2\x82\xac\n'

	EXEC	-n -b 5-8 b.dat
		OUTPUT - $'\xe2\x82\xac\xe2\x82\xac'

	EXEC	-n -b 5-8 c.dat
		OUTPUT - $'\xe2\x82\xac\xe2\x82\xac\n'

	EXEC	-b 5-9 b.dat
		OUTPUT - $'\xac\xe2\x82\xacf'

	EXEC	-b 5-9 c.dat
		OUTPUT - $'\xac\xe2\x82\xacf\n'

	EXEC	-n -b 5-9 b.dat
		OUTPUT - $'\xe2\x82\xac\xe2\x82\xacf'

	EXEC	-n -b 5-9 c.dat
		OUTPUT - $'\xe2\x82\xac\xe2\x82\xacf\n'

	EXEC	-b 5-10 b.dat
		OUTPUT - $'\xac\xe2\x82\xacf2'

	EXEC	-b 5-10 c.dat
		OUTPUT - $'\xac\xe2\x82\xacf2\n'

	EXEC	-n -b 5-10 b.dat
		OUTPUT - $'\xe2\x82\xac\xe2\x82\xacf2'

	EXEC	-n -b 5-10 c.dat
		OUTPUT - $'\xe2\x82\xac\xe2\x82\xacf2\n'

	EXEC	-b 5-11 b.dat
		OUTPUT - $'\xac\xe2\x82\xacf2\xe2'

	EXEC	-b 5-11 c.dat
		OUTPUT - $'\xac\xe2\x82\xacf2\xe2\n'

	EXEC	-n -b 5-11 b.dat
		OUTPUT - $'\xe2\x82\xac\xe2\x82\xacf2'

	EXEC	-n -b 5-11 c.dat
		OUTPUT - $'\xe2\x82\xac\xe2\x82\xacf2\n'

	EXEC	-b 5-12 b.dat
		OUTPUT - $'\xac\xe2\x82\xacf2\xe2\x82'

	EXEC	-b 5-12 c.dat
		OUTPUT - $'\xac\xe2\x82\xacf2\xe2\x82\n'

	EXEC	-n -b 5-12 b.dat
		OUTPUT - $'\xe2\x82\xac\xe2\x82\xacf2'

	EXEC	-n -b 5-12 c.dat
		OUTPUT - $'\xe2\x82\xac\xe2\x82\xacf2\n'

	EXEC	-b 5-13 b.dat
		OUTPUT - $'\xac\xe2\x82\xacf2\xe2\x82\xac'

	EXEC	-b 5-13 c.dat
		OUTPUT - $'\xac\xe2\x82\xacf2\xe2\x82\xac\n'

	EXEC	-n -b 5-13 b.dat
		OUTPUT - $'\xe2\x82\xac\xe2\x82\xacf2\xe2\x82\xac'

	EXEC	-n -b 5-13 c.dat
		OUTPUT - $'\xe2\x82\xac\xe2\x82\xacf2\xe2\x82\xac\n'

	EXEC	-b 5-14 b.dat
		OUTPUT - $'\xac\xe2\x82\xacf2\xe2\x82\xac\xc3'

	EXEC	-b 5-14 c.dat
		OUTPUT - $'\xac\xe2\x82\xacf2\xe2\x82\xac\xc3\n'

	EXEC	-n -b 5-14 b.dat
		OUTPUT - $'\xe2\x82\xac\xe2\x82\xacf2\xe2\x82\xac'

	EXEC	-n -b 5-14 c.dat
		OUTPUT - $'\xe2\x82\xac\xe2\x82\xacf2\xe2\x82\xac\n'

	EXEC	-b 5-15 b.dat
		OUTPUT - $'\xac\xe2\x82\xacf2\xe2\x82\xac\xc3\xbc'

	EXEC	-b 5-15 c.dat
		OUTPUT - $'\xac\xe2\x82\xacf2\xe2\x82\xac\xc3\xbc\n'

	EXEC	-n -b 5-15 b.dat
		OUTPUT - $'\xe2\x82\xac\xe2\x82\xacf2\xe2\x82\xac\xc3\xbc'

	EXEC	-n -b 5-15 c.dat
		OUTPUT - $'\xe2\x82\xac\xe2\x82\xacf2\xe2\x82\xac\xc3\xbc\n'

	EXEC	-b 5-16 b.dat
		OUTPUT - $'\xac\xe2\x82\xacf2\xe2\x82\xac\xc3\xbc'

	EXEC	-b 5-16 c.dat
		OUTPUT - $'\xac\xe2\x82\xacf2\xe2\x82\xac\xc3\xbc\n'

	EXEC	-n -b 5-16 b.dat
		OUTPUT - $'\xe2\x82\xac\xe2\x82\xacf2\xe2\x82\xac\xc3\xbc'

	EXEC	-n -b 5-16 c.dat
		OUTPUT - $'\xe2\x82\xac\xe2\x82\xacf2\xe2\x82\xac\xc3\xbc\n'

	EXEC	-b 6-7 b.dat
		OUTPUT - $'\xe2\x82'

	EXEC	-b 6-7 c.dat
		OUTPUT - $'\xe2\x82\n'

	EXEC	-n -b 6-7 b.dat
		OUTPUT -n - $'\n'

	EXEC	-n -b 6-7 c.dat
		OUTPUT - $'\n'

	EXEC	-b 6-8 b.dat
		OUTPUT - $'\xe2\x82\xac'

	EXEC	-b 6-8 c.dat
		OUTPUT - $'\xe2\x82\xac\n'

	EXEC	-n -b 6-8 b.dat
		OUTPUT - $'\xe2\x82\xac'

	EXEC	-n -b 6-8 c.dat
		OUTPUT - $'\xe2\x82\xac\n'

	EXEC	-b 6-9 b.dat
		OUTPUT - $'\xe2\x82\xacf'

	EXEC	-b 6-9 c.dat
		OUTPUT - $'\xe2\x82\xacf\n'

	EXEC	-n -b 6-9 b.dat
		OUTPUT - $'\xe2\x82\xacf'

	EXEC	-n -b 6-9 c.dat
		OUTPUT - $'\xe2\x82\xacf\n'

	EXEC	-b 6-10 b.dat
		OUTPUT - $'\xe2\x82\xacf2'

	EXEC	-b 6-10 c.dat
		OUTPUT - $'\xe2\x82\xacf2\n'

	EXEC	-n -b 6-10 b.dat
		OUTPUT - $'\xe2\x82\xacf2'

	EXEC	-n -b 6-10 c.dat
		OUTPUT - $'\xe2\x82\xacf2\n'

	EXEC	-b 6-11 b.dat
		OUTPUT - $'\xe2\x82\xacf2\xe2'

	EXEC	-b 6-11 c.dat
		OUTPUT - $'\xe2\x82\xacf2\xe2\n'

	EXEC	-n -b 6-11 b.dat
		OUTPUT - $'\xe2\x82\xacf2'

	EXEC	-n -b 6-11 c.dat
		OUTPUT - $'\xe2\x82\xacf2\n'

	EXEC	-b 6-12 b.dat
		OUTPUT - $'\xe2\x82\xacf2\xe2\x82'

	EXEC	-b 6-12 c.dat
		OUTPUT - $'\xe2\x82\xacf2\xe2\x82\n'

	EXEC	-n -b 6-12 b.dat
		OUTPUT - $'\xe2\x82\xacf2'

	EXEC	-n -b 6-12 c.dat
		OUTPUT - $'\xe2\x82\xacf2\n'

	EXEC	-b 6-13 b.dat
		OUTPUT - $'\xe2\x82\xacf2\xe2\x82\xac'

	EXEC	-b 6-13 c.dat
		OUTPUT - $'\xe2\x82\xacf2\xe2\x82\xac\n'

	EXEC	-n -b 6-13 b.dat
		OUTPUT - $'\xe2\x82\xacf2\xe2\x82\xac'

	EXEC	-n -b 6-13 c.dat
		OUTPUT - $'\xe2\x82\xacf2\xe2\x82\xac\n'

	EXEC	-b 6-14 b.dat
		OUTPUT - $'\xe2\x82\xacf2\xe2\x82\xac\xc3'

	EXEC	-b 6-14 c.dat
		OUTPUT - $'\xe2\x82\xacf2\xe2\x82\xac\xc3\n'

	EXEC	-n -b 6-14 b.dat
		OUTPUT - $'\xe2\x82\xacf2\xe2\x82\xac'

	EXEC	-n -b 6-14 c.dat
		OUTPUT - $'\xe2\x82\xacf2\xe2\x82\xac\n'

	EXEC	-b 6-15 b.dat
		OUTPUT - $'\xe2\x82\xacf2\xe2\x82\xac\xc3\xbc'

	EXEC	-b 6-15 c.dat
		OUTPUT - $'\xe2\x82\xacf2\xe2\x82\xac\xc3\xbc\n'

	EXEC	-n -b 6-15 b.dat
		OUTPUT - $'\xe2\x82\xacf2\xe2\x82\xac\xc3\xbc'

	EXEC	-n -b 6-15 c.dat
		OUTPUT - $'\xe2\x82\xacf2\xe2\x82\xac\xc3\xbc\n'

	EXEC	-b 6-16 b.dat
		OUTPUT - $'\xe2\x82\xacf2\xe2\x82\xac\xc3\xbc'

	EXEC	-b 6-16 c.dat
		OUTPUT - $'\xe2\x82\xacf2\xe2\x82\xac\xc3\xbc\n'

	EXEC	-n -b 6-16 b.dat
		OUTPUT - $'\xe2\x82\xacf2\xe2\x82\xac\xc3\xbc'

	EXEC	-n -b 6-16 c.dat
		OUTPUT - $'\xe2\x82\xacf2\xe2\x82\xac\xc3\xbc\n'

	EXEC	-b 7-8 b.dat
		OUTPUT - $'\x82\xac'

	EXEC	-b 7-8 c.dat
		OUTPUT - $'\x82\xac\n'

	EXEC	-n -b 7-8 b.dat
		OUTPUT - $'\xe2\x82\xac'

	EXEC	-n -b 7-8 c.dat
		OUTPUT - $'\xe2\x82\xac\n'

	EXEC	-b 7-9 b.dat
		OUTPUT - $'\x82\xacf'

	EXEC	-b 7-9 c.dat
		OUTPUT - $'\x82\xacf\n'

	EXEC	-n -b 7-9 b.dat
		OUTPUT - $'\xe2\x82\xacf'

	EXEC	-n -b 7-9 c.dat
		OUTPUT - $'\xe2\x82\xacf\n'

	EXEC	-b 7-10 b.dat
		OUTPUT - $'\x82\xacf2'

	EXEC	-b 7-10 c.dat
		OUTPUT - $'\x82\xacf2\n'

	EXEC	-n -b 7-10 b.dat
		OUTPUT - $'\xe2\x82\xacf2'

	EXEC	-n -b 7-10 c.dat
		OUTPUT - $'\xe2\x82\xacf2\n'

	EXEC	-b 7-11 b.dat
		OUTPUT - $'\x82\xacf2\xe2'

	EXEC	-b 7-11 c.dat
		OUTPUT - $'\x82\xacf2\xe2\n'

	EXEC	-n -b 7-11 b.dat
		OUTPUT - $'\xe2\x82\xacf2'

	EXEC	-n -b 7-11 c.dat
		OUTPUT - $'\xe2\x82\xacf2\n'

	EXEC	-b 7-12 b.dat
		OUTPUT - $'\x82\xacf2\xe2\x82'

	EXEC	-b 7-12 c.dat
		OUTPUT - $'\x82\xacf2\xe2\x82\n'

	EXEC	-n -b 7-12 b.dat
		OUTPUT - $'\xe2\x82\xacf2'

	EXEC	-n -b 7-12 c.dat
		OUTPUT - $'\xe2\x82\xacf2\n'

	EXEC	-b 7-13 b.dat
		OUTPUT - $'\x82\xacf2\xe2\x82\xac'

	EXEC	-b 7-13 c.dat
		OUTPUT - $'\x82\xacf2\xe2\x82\xac\n'

	EXEC	-n -b 7-13 b.dat
		OUTPUT - $'\xe2\x82\xacf2\xe2\x82\xac'

	EXEC	-n -b 7-13 c.dat
		OUTPUT - $'\xe2\x82\xacf2\xe2\x82\xac\n'

	EXEC	-b 7-14 b.dat
		OUTPUT - $'\x82\xacf2\xe2\x82\xac\xc3'

	EXEC	-b 7-14 c.dat
		OUTPUT - $'\x82\xacf2\xe2\x82\xac\xc3\n'

	EXEC	-n -b 7-14 b.dat
		OUTPUT - $'\xe2\x82\xacf2\xe2\x82\xac'

	EXEC	-n -b 7-14 c.dat
		OUTPUT - $'\xe2\x82\xacf2\xe2\x82\xac\n'

	EXEC	-b 7-15 b.dat
		OUTPUT - $'\x82\xacf2\xe2\x82\xac\xc3\xbc'

	EXEC	-b 7-15 c.dat
		OUTPUT - $'\x82\xacf2\xe2\x82\xac\xc3\xbc\n'

	EXEC	-n -b 7-15 b.dat
		OUTPUT - $'\xe2\x82\xacf2\xe2\x82\xac\xc3\xbc'

	EXEC	-n -b 7-15 c.dat
		OUTPUT - $'\xe2\x82\xacf2\xe2\x82\xac\xc3\xbc\n'

	EXEC	-b 7-16 b.dat
		OUTPUT - $'\x82\xacf2\xe2\x82\xac\xc3\xbc'

	EXEC	-b 7-16 c.dat
		OUTPUT - $'\x82\xacf2\xe2\x82\xac\xc3\xbc\n'

	EXEC	-n -b 7-16 b.dat
		OUTPUT - $'\xe2\x82\xacf2\xe2\x82\xac\xc3\xbc'

	EXEC	-n -b 7-16 c.dat
		OUTPUT - $'\xe2\x82\xacf2\xe2\x82\xac\xc3\xbc\n'

	EXEC	-b 8-9 b.dat
		OUTPUT - $'\xacf'

	EXEC	-b 8-9 c.dat
		OUTPUT - $'\xacf\n'

	EXEC	-n -b 8-9 b.dat
		OUTPUT - $'\xe2\x82\xacf'

	EXEC	-n -b 8-9 c.dat
		OUTPUT - $'\xe2\x82\xacf\n'

	EXEC	-b 8-10 b.dat
		OUTPUT - $'\xacf2'

	EXEC	-b 8-10 c.dat
		OUTPUT - $'\xacf2\n'

	EXEC	-n -b 8-10 b.dat
		OUTPUT - $'\xe2\x82\xacf2'

	EXEC	-n -b 8-10 c.dat
		OUTPUT - $'\xe2\x82\xacf2\n'

	EXEC	-b 8-11 b.dat
		OUTPUT - $'\xacf2\xe2'

	EXEC	-b 8-11 c.dat
		OUTPUT - $'\xacf2\xe2\n'

	EXEC	-n -b 8-11 b.dat
		OUTPUT - $'\xe2\x82\xacf2'

	EXEC	-n -b 8-11 c.dat
		OUTPUT - $'\xe2\x82\xacf2\n'

	EXEC	-b 8-12 b.dat
		OUTPUT - $'\xacf2\xe2\x82'

	EXEC	-b 8-12 c.dat
		OUTPUT - $'\xacf2\xe2\x82\n'

	EXEC	-n -b 8-12 b.dat
		OUTPUT - $'\xe2\x82\xacf2'

	EXEC	-n -b 8-12 c.dat
		OUTPUT - $'\xe2\x82\xacf2\n'

	EXEC	-b 8-13 b.dat
		OUTPUT - $'\xacf2\xe2\x82\xac'

	EXEC	-b 8-13 c.dat
		OUTPUT - $'\xacf2\xe2\x82\xac\n'

	EXEC	-n -b 8-13 b.dat
		OUTPUT - $'\xe2\x82\xacf2\xe2\x82\xac'

	EXEC	-n -b 8-13 c.dat
		OUTPUT - $'\xe2\x82\xacf2\xe2\x82\xac\n'

	EXEC	-b 8-14 b.dat
		OUTPUT - $'\xacf2\xe2\x82\xac\xc3'

	EXEC	-b 8-14 c.dat
		OUTPUT - $'\xacf2\xe2\x82\xac\xc3\n'

	EXEC	-n -b 8-14 b.dat
		OUTPUT - $'\xe2\x82\xacf2\xe2\x82\xac'

	EXEC	-n -b 8-14 c.dat
		OUTPUT - $'\xe2\x82\xacf2\xe2\x82\xac\n'

	EXEC	-b 8-15 b.dat
		OUTPUT - $'\xacf2\xe2\x82\xac\xc3\xbc'

	EXEC	-b 8-15 c.dat
		OUTPUT - $'\xacf2\xe2\x82\xac\xc3\xbc\n'

	EXEC	-n -b 8-15 b.dat
		OUTPUT - $'\xe2\x82\xacf2\xe2\x82\xac\xc3\xbc'

	EXEC	-n -b 8-15 c.dat
		OUTPUT - $'\xe2\x82\xacf2\xe2\x82\xac\xc3\xbc\n'

	EXEC	-b 8-16 b.dat
		OUTPUT - $'\xacf2\xe2\x82\xac\xc3\xbc'

	EXEC	-b 8-16 c.dat
		OUTPUT - $'\xacf2\xe2\x82\xac\xc3\xbc\n'

	EXEC	-n -b 8-16 b.dat
		OUTPUT - $'\xe2\x82\xacf2\xe2\x82\xac\xc3\xbc'

	EXEC	-n -b 8-16 c.dat
		OUTPUT - $'\xe2\x82\xacf2\xe2\x82\xac\xc3\xbc\n'

	EXEC	-b 9-10 b.dat
		OUTPUT - f2

	EXEC	-b 9-10 c.dat
		OUTPUT - $'f2\n'

	EXEC	-n -b 9-10 b.dat
		OUTPUT - f2

	EXEC	-n -b 9-10 c.dat
		OUTPUT - $'f2\n'

	EXEC	-b 9-11 b.dat
		OUTPUT - $'f2\xe2'

	EXEC	-b 9-11 c.dat
		OUTPUT - $'f2\xe2\n'

	EXEC	-n -b 9-11 b.dat
		OUTPUT - f2

	EXEC	-n -b 9-11 c.dat
		OUTPUT - $'f2\n'

	EXEC	-b 9-12 b.dat
		OUTPUT - $'f2\xe2\x82'

	EXEC	-b 9-12 c.dat
		OUTPUT - $'f2\xe2\x82\n'

	EXEC	-n -b 9-12 b.dat
		OUTPUT - f2

	EXEC	-n -b 9-12 c.dat
		OUTPUT - $'f2\n'

	EXEC	-b 9-13 b.dat
		OUTPUT - $'f2\xe2\x82\xac'

	EXEC	-b 9-13 c.dat
		OUTPUT - $'f2\xe2\x82\xac\n'

	EXEC	-n -b 9-13 b.dat
		OUTPUT - $'f2\xe2\x82\xac'

	EXEC	-n -b 9-13 c.dat
		OUTPUT - $'f2\xe2\x82\xac\n'

	EXEC	-b 9-14 b.dat
		OUTPUT - $'f2\xe2\x82\xac\xc3'

	EXEC	-b 9-14 c.dat
		OUTPUT - $'f2\xe2\x82\xac\xc3\n'

	EXEC	-n -b 9-14 b.dat
		OUTPUT - $'f2\xe2\x82\xac'

	EXEC	-n -b 9-14 c.dat
		OUTPUT - $'f2\xe2\x82\xac\n'

	EXEC	-b 9-15 b.dat
		OUTPUT - $'f2\xe2\x82\xac\xc3\xbc'

	EXEC	-b 9-15 c.dat
		OUTPUT - $'f2\xe2\x82\xac\xc3\xbc\n'

	EXEC	-n -b 9-15 b.dat
		OUTPUT - $'f2\xe2\x82\xac\xc3\xbc'

	EXEC	-n -b 9-15 c.dat
		OUTPUT - $'f2\xe2\x82\xac\xc3\xbc\n'

	EXEC	-b 9-16 b.dat
		OUTPUT - $'f2\xe2\x82\xac\xc3\xbc'

	EXEC	-b 9-16 c.dat
		OUTPUT - $'f2\xe2\x82\xac\xc3\xbc\n'

	EXEC	-n -b 9-16 b.dat
		OUTPUT - $'f2\xe2\x82\xac\xc3\xbc'

	EXEC	-n -b 9-16 c.dat
		OUTPUT - $'f2\xe2\x82\xac\xc3\xbc\n'

	EXEC	-b 10-11 b.dat
		OUTPUT - $'2\xe2'

	EXEC	-b 10-11 c.dat
		OUTPUT - $'2\xe2\n'

	EXEC	-n -b 10-11 b.dat
		OUTPUT - 2

	EXEC	-n -b 10-11 c.dat
		OUTPUT - $'2\n'

	EXEC	-b 10-12 b.dat
		OUTPUT - $'2\xe2\x82'

	EXEC	-b 10-12 c.dat
		OUTPUT - $'2\xe2\x82\n'

	EXEC	-n -b 10-12 b.dat
		OUTPUT - 2

	EXEC	-n -b 10-12 c.dat
		OUTPUT - $'2\n'

	EXEC	-b 10-13 b.dat
		OUTPUT - $'2\xe2\x82\xac'

	EXEC	-b 10-13 c.dat
		OUTPUT - $'2\xe2\x82\xac\n'

	EXEC	-n -b 10-13 b.dat
		OUTPUT - $'2\xe2\x82\xac'

	EXEC	-n -b 10-13 c.dat
		OUTPUT - $'2\xe2\x82\xac\n'

	EXEC	-b 10-14 b.dat
		OUTPUT - $'2\xe2\x82\xac\xc3'

	EXEC	-b 10-14 c.dat
		OUTPUT - $'2\xe2\x82\xac\xc3\n'

	EXEC	-n -b 10-14 b.dat
		OUTPUT - $'2\xe2\x82\xac'

	EXEC	-n -b 10-14 c.dat
		OUTPUT - $'2\xe2\x82\xac\n'

	EXEC	-b 10-15 b.dat
		OUTPUT - $'2\xe2\x82\xac\xc3\xbc'

	EXEC	-b 10-15 c.dat
		OUTPUT - $'2\xe2\x82\xac\xc3\xbc\n'

	EXEC	-n -b 10-15 b.dat
		OUTPUT - $'2\xe2\x82\xac\xc3\xbc'

	EXEC	-n -b 10-15 c.dat
		OUTPUT - $'2\xe2\x82\xac\xc3\xbc\n'

	EXEC	-b 10-16 b.dat
		OUTPUT - $'2\xe2\x82\xac\xc3\xbc'

	EXEC	-b 10-16 c.dat
		OUTPUT - $'2\xe2\x82\xac\xc3\xbc\n'

	EXEC	-n -b 10-16 b.dat
		OUTPUT - $'2\xe2\x82\xac\xc3\xbc'

	EXEC	-n -b 10-16 c.dat
		OUTPUT - $'2\xe2\x82\xac\xc3\xbc\n'

	EXEC	-b 11-12 b.dat
		OUTPUT - $'\xe2\x82'

	EXEC	-b 11-12 c.dat
		OUTPUT - $'\xe2\x82\n'

	EXEC	-n -b 11-12 b.dat
		OUTPUT -n - $'\n'

	EXEC	-n -b 11-12 c.dat
		OUTPUT - $'\n'

	EXEC	-b 11-13 b.dat
		OUTPUT - $'\xe2\x82\xac'

	EXEC	-b 11-13 c.dat
		OUTPUT - $'\xe2\x82\xac\n'

	EXEC	-n -b 11-13 b.dat
		OUTPUT - $'\xe2\x82\xac'

	EXEC	-n -b 11-13 c.dat
		OUTPUT - $'\xe2\x82\xac\n'

	EXEC	-b 11-14 b.dat
		OUTPUT - $'\xe2\x82\xac\xc3'

	EXEC	-b 11-14 c.dat
		OUTPUT - $'\xe2\x82\xac\xc3\n'

	EXEC	-n -b 11-14 b.dat
		OUTPUT - $'\xe2\x82\xac'

	EXEC	-n -b 11-14 c.dat
		OUTPUT - $'\xe2\x82\xac\n'

	EXEC	-b 11-15 b.dat
		OUTPUT - $'\xe2\x82\xac\xc3\xbc'

	EXEC	-b 11-15 c.dat
		OUTPUT - $'\xe2\x82\xac\xc3\xbc\n'

	EXEC	-n -b 11-15 b.dat
		OUTPUT - $'\xe2\x82\xac\xc3\xbc'

	EXEC	-n -b 11-15 c.dat
		OUTPUT - $'\xe2\x82\xac\xc3\xbc\n'

	EXEC	-b 11-16 b.dat
		OUTPUT - $'\xe2\x82\xac\xc3\xbc'

	EXEC	-b 11-16 c.dat
		OUTPUT - $'\xe2\x82\xac\xc3\xbc\n'

	EXEC	-n -b 11-16 b.dat
		OUTPUT - $'\xe2\x82\xac\xc3\xbc'

	EXEC	-n -b 11-16 c.dat
		OUTPUT - $'\xe2\x82\xac\xc3\xbc\n'

	EXEC	-b 12-13 b.dat
		OUTPUT - $'\x82\xac'

	EXEC	-b 12-13 c.dat
		OUTPUT - $'\x82\xac\n'

	EXEC	-n -b 12-13 b.dat
		OUTPUT - $'\xe2\x82\xac'

	EXEC	-n -b 12-13 c.dat
		OUTPUT - $'\xe2\x82\xac\n'

	EXEC	-b 12-14 b.dat
		OUTPUT - $'\x82\xac\xc3'

	EXEC	-b 12-14 c.dat
		OUTPUT - $'\x82\xac\xc3\n'

	EXEC	-n -b 12-14 b.dat
		OUTPUT - $'\xe2\x82\xac'

	EXEC	-n -b 12-14 c.dat
		OUTPUT - $'\xe2\x82\xac\n'

	EXEC	-b 12-15 b.dat
		OUTPUT - $'\x82\xac\xc3\xbc'

	EXEC	-b 12-15 c.dat
		OUTPUT - $'\x82\xac\xc3\xbc\n'

	EXEC	-n -b 12-15 b.dat
		OUTPUT - $'\xe2\x82\xac\xc3\xbc'

	EXEC	-n -b 12-15 c.dat
		OUTPUT - $'\xe2\x82\xac\xc3\xbc\n'

	EXEC	-b 12-16 b.dat
		OUTPUT - $'\x82\xac\xc3\xbc'

	EXEC	-b 12-16 c.dat
		OUTPUT - $'\x82\xac\xc3\xbc\n'

	EXEC	-n -b 12-16 b.dat
		OUTPUT - $'\xe2\x82\xac\xc3\xbc'

	EXEC	-n -b 12-16 c.dat
		OUTPUT - $'\xe2\x82\xac\xc3\xbc\n'

	EXEC	-b 13-14 b.dat
		OUTPUT - $'\xac\xc3'

	EXEC	-b 13-14 c.dat
		OUTPUT - $'\xac\xc3\n'

	EXEC	-n -b 13-14 b.dat
		OUTPUT - $'\xe2\x82\xac'

	EXEC	-n -b 13-14 c.dat
		OUTPUT - $'\xe2\x82\xac\n'

	EXEC	-b 13-15 b.dat
		OUTPUT - $'\xac\xc3\xbc'

	EXEC	-b 13-15 c.dat
		OUTPUT - $'\xac\xc3\xbc\n'

	EXEC	-n -b 13-15 b.dat
		OUTPUT - $'\xe2\x82\xac\xc3\xbc'

	EXEC	-n -b 13-15 c.dat
		OUTPUT - $'\xe2\x82\xac\xc3\xbc\n'

	EXEC	-b 13-16 b.dat
		OUTPUT - $'\xac\xc3\xbc'

	EXEC	-b 13-16 c.dat
		OUTPUT - $'\xac\xc3\xbc\n'

	EXEC	-n -b 13-16 b.dat
		OUTPUT - $'\xe2\x82\xac\xc3\xbc'

	EXEC	-n -b 13-16 c.dat
		OUTPUT - $'\xe2\x82\xac\xc3\xbc\n'

	EXEC	-b 14-15 b.dat
		OUTPUT - $'\xc3\xbc'

	EXEC	-b 14-15 c.dat
		OUTPUT - $'\xc3\xbc\n'

	EXEC	-n -b 14-15 b.dat
		OUTPUT - $'\xc3\xbc'

	EXEC	-n -b 14-15 c.dat
		OUTPUT - $'\xc3\xbc\n'

	EXEC	-b 14-16 b.dat
		OUTPUT - $'\xc3\xbc'

	EXEC	-b 14-16 c.dat
		OUTPUT - $'\xc3\xbc\n'

	EXEC	-n -b 14-16 b.dat
		OUTPUT - $'\xc3\xbc'

	EXEC	-n -b 14-16 c.dat
		OUTPUT - $'\xc3\xbc\n'

	EXEC	-b 15-16 b.dat
		OUTPUT - $'\xbc'

	EXEC	-b 15-16 c.dat
		OUTPUT - $'\xbc\n'

	EXEC	-n -b 15-16 b.dat
		OUTPUT - $'\xc3\xbc'

	EXEC	-n -b 15-16 c.dat
		OUTPUT - $'\xc3\xbc\n'

TEST 12 'multibyte euc'

EXPORT LC_CTYPE=ja_JP.eucJP

	EXEC	-d $'\xa4\xa4' -f1 a.dat
		INPUT -n -
		INPUT a.dat $'\xa4\xa2\xa4\xa4\xa4\xa4\xa4\xa6\xa4\xa8'
		OUTPUT - $'\xa4\xa2'
		ERROR -n -
