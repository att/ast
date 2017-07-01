# : : generated from /home/gsf/src/cmd/builtin/uniq.rt by mktest : : #

UNIT uniq

TEST 01 '-f -s'

	EXEC	-f 3 -s 3 tst.dat
		INPUT -n -
		INPUT tst.dat $'field1 field2 field3 abcx\nfield1 field2 field3 pdqx\nabc'
		OUTPUT - $'field1 field2 field3 abcx\nfield1 field2 field3 pdqx\nabc'
		ERROR -n -

	EXEC	-f 3 -s 4 tst.dat
		OUTPUT - $'field1 field2 field3 abcx\nabc'

TEST 02 -i

	EXEC	-i -f 3 -s 3 tst.dat
		INPUT -n -
		INPUT tst.dat $'field1 field2 field3 abcx
field1 field2 field3 pdqx
FIELD1 FIELD2 FIELD3 PDQX
abc
AbC'
		OUTPUT - $'field1 field2 field3 abcx\nfield1 field2 field3 pdqx\nabc'
		ERROR -n -

	EXEC	-i -f 3 -s 4 tst.dat
		OUTPUT - $'field1 field2 field3 abcx\nabc'

TEST 03 'gnu tests'

	EXEC
		INPUT -n -
		OUTPUT -
		ERROR -n -

	EXEC
		INPUT - $'a\na'
		OUTPUT - a

	EXEC
		INPUT -n tst.dat 'a a'

	EXEC
		INPUT -n tst.dat 'a b'

	EXEC
		INPUT -n tst.dat 'a a b'

	EXEC
		INPUT - $'b\na\na'
		OUTPUT - $'b\na'

	EXEC
		INPUT - $'a\nb\nc'
		SAME OUTPUT INPUT

	EXEC
		INPUT tst.dat $'\xf6\nv'
		SAME OUTPUT INPUT

TEST 04 -u

	EXEC	-u
		INPUT - $'a\na'
		OUTPUT -
		ERROR -n -

	EXEC	-u
		INPUT - $'a\nb'
		SAME OUTPUT INPUT

	EXEC	-u
		INPUT - $'a\nb\na'
		SAME OUTPUT INPUT

	EXEC	-u
		INPUT - $'a\na'
		OUTPUT -

	EXEC	-u

TEST 05 -d

	EXEC	-d
		INPUT - $'a\na'
		OUTPUT - a
		ERROR -n -

	EXEC	-d
		INPUT - $'a\nb'
		OUTPUT -

	EXEC	-d
		INPUT - $'a\nb\na'

	EXEC	-d
		INPUT - $'a\na\nb'
		OUTPUT - a

TEST 06 -1

	EXEC	-1
		INPUT - $'a a\nb a'
		OUTPUT - 'a a'
		ERROR -n -

TEST 07 '-f 1'

	EXEC	-f 1
		INPUT - $'a a\nb a'
		OUTPUT - 'a a'
		ERROR -n -

	EXEC	-f 1
		INPUT - $'a a\nb b'
		SAME OUTPUT INPUT

	EXEC	-f 1
		INPUT - $'a a a\nb a c'
		SAME OUTPUT INPUT

	EXEC	-f 1
		INPUT - $'b a\na a'
		OUTPUT - 'b a'

TEST 08 '-f 2'

	EXEC	-f 2
		INPUT - $'a a c\nb a c'
		OUTPUT - 'a a c'
		ERROR -n -

TEST 09 +1

	EXEC	+1
		INPUT - $'aaa\naaa'
		OUTPUT - aaa
		ERROR -n -

	EXEC	+1
		INPUT - $'baa\naaa'
		OUTPUT - baa

TEST 10 '-s 1'

	EXEC	-s 1
		INPUT - $'aaa\naaa'
		OUTPUT - aaa
		ERROR -n -

TEST 11 '-s 2'

	EXEC	-s 2
		INPUT - $'baa\naaa'
		OUTPUT - baa
		ERROR -n -

TEST 12 '+1 --'

	EXEC	+1 --
		INPUT - $'aaa\naaa'
		OUTPUT - aaa
		ERROR -n -

	EXEC	+1 --
		INPUT - $'baa\naaa'
		OUTPUT - baa

TEST 13 '-f 1 -s 1'

	EXEC	-f 1 -s 1
		INPUT - $'a aaa\nb ab'
		SAME OUTPUT INPUT
		ERROR -n -

	EXEC	-f 1 -s 1
		INPUT - $'a aaa\nb aaa'
		OUTPUT - 'a aaa'

TEST 14 '-s 1 -f 1'

	EXEC	-s 1 -f 1
		INPUT - $'a aaa\nb ab'
		SAME OUTPUT INPUT
		ERROR -n -

	EXEC	-s 1 -f 1
		INPUT - $'a aaa\nb aaa'
		OUTPUT - 'a aaa'

TEST 15 '-s 4'

	EXEC	-s 4
		INPUT - $'abc\nabcd'
		OUTPUT - abc
		ERROR -n -

TEST 16 '-s 0'

	EXEC	-s 0
		INPUT - $'abc\nabcd'
		SAME OUTPUT INPUT
		ERROR -n -

	EXEC	-s 0
		INPUT - abc
		SAME OUTPUT INPUT

TEST 17 '-w 0'

	EXEC	-w 0
		INPUT - $'abc\nabcd'
		OUTPUT - abc
		ERROR -n -

TEST 18 '-w 1'

	EXEC	-w 1
		INPUT - $'a a\nb a'
		SAME OUTPUT INPUT
		ERROR -n -

TEST 19 '-w 3'

	EXEC	-w 3
		INPUT - $'a a\nb a'
		SAME OUTPUT INPUT
		ERROR -n -

TEST 20 '-w 1 -f 1'

	EXEC	-w 1 -f 1
		INPUT - $'a a a\nb a c'
		OUTPUT - 'a a a'
		ERROR -n -

TEST 21 '-f 1 -w 1'

	EXEC	-f 1 -w 1
		INPUT - $'a a a\nb a c'
		OUTPUT - 'a a a'
		ERROR -n -

TEST 22 '-f 1 -w 4'

	EXEC	-f 1 -w 4
		INPUT - $'a a a\nb a c'
		SAME OUTPUT INPUT
		ERROR -n -

TEST 23 '-f 1 -w 3'

	EXEC	-f 1 -w 3
		INPUT - $'a a a\nb a c'
		OUTPUT - 'a a a'
		ERROR -n -

TEST 24 'embedded nul'

	EXEC
		INPUT -n -
		INPUT $'-fa%Za\na\n' tst.dat
		OUTPUT -
		ERROR -n -

	EXEC
		INPUT - $'a\ta\na a'
		SAME OUTPUT INPUT

TEST 25 '-f 1'

	EXEC	-f 1
		INPUT - $'a\ta\na a'
		SAME OUTPUT INPUT
		ERROR -n -

TEST 26 '-f 2'

	EXEC	-f 2
		INPUT - $'a\ta a\na a a'
		OUTPUT - $'a\ta a'
		ERROR -n -

TEST 27 '-f 1'

	EXEC	-f 1
		INPUT - $'a\ta\na\ta'
		OUTPUT - $'a\ta'
		ERROR -n -

TEST 28 -c

	EXEC	-c
		INPUT - $'a\nb'
		OUTPUT - $'   1 a\n   1 b'
		ERROR -n -

	EXEC	-c
		INPUT - $'a\na'
		OUTPUT - '   2 a'

TEST 29 -D

	EXEC	-D
		INPUT - $'a\na'
		SAME OUTPUT INPUT
		ERROR -n -

TEST 30 '-D -w1'

	EXEC	-D -w1
		INPUT - $'a a\na b'
		SAME OUTPUT INPUT
		ERROR -n -

TEST 31 '-D -c'

	EXEC	-D -c
		INPUT - $'a a\na b'
		OUTPUT -
		ERROR - $'uniq: -c and -D are mutually exclusive
Usage: uniq [-cdiu] [-D[delimit]] [-f fields] [-s chars] [-w chars]
            [infile [outfile]]'
		EXIT 2

TEST 32 --all-repeated

	EXEC	'--all-repeated=separate'
		INPUT - $'a\na'
		SAME OUTPUT INPUT
		ERROR -n -

	EXEC	'--all-repeated=separate'
		INPUT - $'a\na\nb\nc\nc'
		OUTPUT - $'a\na\n\nc\nc'

	EXEC	'--all-repeated=separate'
		INPUT - $'a\na\nb\nb\nc'
		OUTPUT - $'a\na\n\nb\nb'

	EXEC	'--all-repeated=prepend'
		INPUT - $'a\na'
		OUTPUT - $'\na\na'

	EXEC	'--all-repeated=prepend'
		INPUT - $'a\na\nb\nc\nc'
		OUTPUT - $'\na\na\n\nc\nc'

	EXEC	'--all-repeated=prepend'
		INPUT - $'a\nb'
		OUTPUT -

	EXEC	'--all-repeated=none' '--check-chars=1'
		INPUT - $'a1\nb2\nb3\nc4\nd5\nd6\nd7\ne8\nf9\nfA\ngB'
		OUTPUT - $'b2\nb3\nd5\nd6\nd7\nf9\nfA'

	EXEC	'--all-repeated=prepend' '--check-chars=1'
		OUTPUT - $'\nb2\nb3\n\nd5\nd6\nd7\n\nf9\nfA'

	EXEC	'--all-repeated=separate' '--check-chars=1'
		OUTPUT - $'b2\nb3\n\nd5\nd6\nd7\n\nf9\nfA'

	EXEC	'--all-repeated=boofar' '--check-chars=1'
		OUTPUT -
		ERROR - $'uniq: --all-repeated: boofar: unknown option argument value
Usage: uniq [--count] [--repeated|duplicates] [--all-repeated[=delimit]]
            [--skip-fields=fields] [--ignore-case] [--skip-chars=chars]
            [--unique] [--check-chars=chars] [infile [outfile]]'
		EXIT 2
