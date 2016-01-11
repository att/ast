# regression tests for the tsort utilitiy

KEEP "*.dat"

TEST 01 'basics'
	EXEC
		INPUT - $'a a'
		OUTPUT - $'a'
	EXEC
		INPUT - $'a a b b'
		OUTPUT - $'a\nb'
	EXEC
		INPUT - $'a c\na b\nb c'
		OUTPUT - $'a\nb\nc'
	EXEC
		INPUT - $'a b\nb c\nc a'
		OUTPUT - $'a\nb\nc'
		ERROR - $'tsort: warning: cycle in data\ntsort:  a\ntsort:  b\ntsort:  c'
		EXIT 1
	EXEC
		INPUT - $'a'
		OUTPUT -
		ERROR - $'tsort: odd data'

TEST 02 'how did this slip through?'
	EXEC
		INPUT - $'b c\na c\na b'
		OUTPUT - $'a\nb\nc'
