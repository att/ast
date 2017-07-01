# : : generated from /home/gsf/src/cmd/builtin/cmp.rt by mktest : : #

UNIT cmp

TEST 01 basics

	EXEC	i j
		INPUT -n -
		INPUT i $'a\x89b\xc9c\tdIe'
		INPUT j $'a\x8ab\xcac\ndJe'
		OUTPUT - 'i j differ: char 2, line 1'
		ERROR -n -
		EXIT 1

	EXEC	-b i j
		OUTPUT - 'i j differ: char 2, line 1, 211 212'

	EXEC	-c i j
		OUTPUT - 'i j differ: char 2, line 1, M^I M^J'

	EXEC	-bc i j
		OUTPUT - 'i j differ: char 2, line 1, 211 M^I  212 M^J'

	EXEC	-n 1 -d 0 -l i j
		OUTPUT -
		EXIT 0

	EXEC	-n 1 -d 0 -lb i j

	EXEC	-n 1 -d 0 -lc i j

	EXEC	-n 1 -d 0 -lbc i j

	EXEC	-n 1 -d 1 -l i j

	EXEC	-n 1 -d 1 -lb i j

	EXEC	-n 1 -d 1 -lc i j

	EXEC	-n 1 -d 1 -lbc i j

	EXEC	-n 1 -d 2 -l i j

	EXEC	-n 1 -d 2 -lb i j

	EXEC	-n 1 -d 2 -lc i j

	EXEC	-n 1 -d 2 -lbc i j

	EXEC	-n 1 -d 3 -l i j

	EXEC	-n 1 -d 3 -lb i j

	EXEC	-n 1 -d 3 -lc i j

	EXEC	-n 1 -d 3 -lbc i j

	EXEC	-n 1 -d 4 -l i j

	EXEC	-n 1 -d 4 -lb i j

	EXEC	-n 1 -d 4 -lc i j

	EXEC	-n 1 -d 4 -lbc i j

	EXEC	-n 1 -d 5 -l i j

	EXEC	-n 1 -d 5 -lb i j

	EXEC	-n 1 -d 5 -lc i j

	EXEC	-n 1 -d 5 -lbc i j

	EXEC	-n 2 -d 0 -l i j
		EXIT 1

	EXEC	-n 2 -d 0 -lb i j

	EXEC	-n 2 -d 0 -lc i j

	EXEC	-n 2 -d 0 -lbc i j

	EXEC	-n 2 -d 1 -l i j
		OUTPUT - '     2  211 212'

	EXEC	-n 2 -d 1 -lb i j

	EXEC	-n 2 -d 1 -lc i j
		OUTPUT - '     2  M^I M^J'

	EXEC	-n 2 -d 1 -lbc i j
		OUTPUT - '     2  211 M^I  212 M^J'

	EXEC	-n 2 -d 2 -l i j
		OUTPUT - '     2  211 212'

	EXEC	-n 2 -d 2 -lb i j

	EXEC	-n 2 -d 2 -lc i j
		OUTPUT - '     2  M^I M^J'

	EXEC	-n 2 -d 2 -lbc i j
		OUTPUT - '     2  211 M^I  212 M^J'

	EXEC	-n 2 -d 3 -l i j
		OUTPUT - '     2  211 212'

	EXEC	-n 2 -d 3 -lb i j

	EXEC	-n 2 -d 3 -lc i j
		OUTPUT - '     2  M^I M^J'

	EXEC	-n 2 -d 3 -lbc i j
		OUTPUT - '     2  211 M^I  212 M^J'

	EXEC	-n 2 -d 4 -l i j
		OUTPUT - '     2  211 212'

	EXEC	-n 2 -d 4 -lb i j

	EXEC	-n 2 -d 4 -lc i j
		OUTPUT - '     2  M^I M^J'

	EXEC	-n 2 -d 4 -lbc i j
		OUTPUT - '     2  211 M^I  212 M^J'

	EXEC	-n 2 -d 5 -l i j
		OUTPUT - '     2  211 212'

	EXEC	-n 2 -d 5 -lb i j

	EXEC	-n 2 -d 5 -lc i j
		OUTPUT - '     2  M^I M^J'

	EXEC	-n 2 -d 5 -lbc i j
		OUTPUT - '     2  211 M^I  212 M^J'

	EXEC	-n 3 -d 0 -l i j
		OUTPUT -

	EXEC	-n 3 -d 0 -lb i j

	EXEC	-n 3 -d 0 -lc i j

	EXEC	-n 3 -d 0 -lbc i j

	EXEC	-n 3 -d 1 -l i j
		OUTPUT - '     2  211 212'

	EXEC	-n 3 -d 1 -lb i j

	EXEC	-n 3 -d 1 -lc i j
		OUTPUT - '     2  M^I M^J'

	EXEC	-n 3 -d 1 -lbc i j
		OUTPUT - '     2  211 M^I  212 M^J'

	EXEC	-n 3 -d 2 -l i j
		OUTPUT - '     2  211 212'

	EXEC	-n 3 -d 2 -lb i j

	EXEC	-n 3 -d 2 -lc i j
		OUTPUT - '     2  M^I M^J'

	EXEC	-n 3 -d 2 -lbc i j
		OUTPUT - '     2  211 M^I  212 M^J'

	EXEC	-n 3 -d 3 -l i j
		OUTPUT - '     2  211 212'

	EXEC	-n 3 -d 3 -lb i j

	EXEC	-n 3 -d 3 -lc i j
		OUTPUT - '     2  M^I M^J'

	EXEC	-n 3 -d 3 -lbc i j
		OUTPUT - '     2  211 M^I  212 M^J'

	EXEC	-n 3 -d 4 -l i j
		OUTPUT - '     2  211 212'

	EXEC	-n 3 -d 4 -lb i j

	EXEC	-n 3 -d 4 -lc i j
		OUTPUT - '     2  M^I M^J'

	EXEC	-n 3 -d 4 -lbc i j
		OUTPUT - '     2  211 M^I  212 M^J'

	EXEC	-n 3 -d 5 -l i j
		OUTPUT - '     2  211 212'

	EXEC	-n 3 -d 5 -lb i j

	EXEC	-n 3 -d 5 -lc i j
		OUTPUT - '     2  M^I M^J'

	EXEC	-n 3 -d 5 -lbc i j
		OUTPUT - '     2  211 M^I  212 M^J'

	EXEC	-n 4 -d 0 -l i j
		OUTPUT -

	EXEC	-n 4 -d 0 -lb i j

	EXEC	-n 4 -d 0 -lc i j

	EXEC	-n 4 -d 0 -lbc i j

	EXEC	-n 4 -d 1 -l i j
		OUTPUT - '     2  211 212'

	EXEC	-n 4 -d 1 -lb i j

	EXEC	-n 4 -d 1 -lc i j
		OUTPUT - '     2  M^I M^J'

	EXEC	-n 4 -d 1 -lbc i j
		OUTPUT - '     2  211 M^I  212 M^J'

	EXEC	-n 4 -d 2 -l i j
		OUTPUT - $'     2  211 212\n     4  311 312'

	EXEC	-n 4 -d 2 -lb i j

	EXEC	-n 4 -d 2 -lc i j
		OUTPUT - $'     2  M^I M^J\n     4  M-I M-J'

	EXEC	-n 4 -d 2 -lbc i j
		OUTPUT - $'     2  211 M^I  212 M^J\n     4  311 M-I  312 M-J'

	EXEC	-n 4 -d 3 -l i j
		OUTPUT - $'     2  211 212\n     4  311 312'

	EXEC	-n 4 -d 3 -lb i j

	EXEC	-n 4 -d 3 -lc i j
		OUTPUT - $'     2  M^I M^J\n     4  M-I M-J'

	EXEC	-n 4 -d 3 -lbc i j
		OUTPUT - $'     2  211 M^I  212 M^J\n     4  311 M-I  312 M-J'

	EXEC	-n 4 -d 4 -l i j
		OUTPUT - $'     2  211 212\n     4  311 312'

	EXEC	-n 4 -d 4 -lb i j

	EXEC	-n 4 -d 4 -lc i j
		OUTPUT - $'     2  M^I M^J\n     4  M-I M-J'

	EXEC	-n 4 -d 4 -lbc i j
		OUTPUT - $'     2  211 M^I  212 M^J\n     4  311 M-I  312 M-J'

	EXEC	-n 4 -d 5 -l i j
		OUTPUT - $'     2  211 212\n     4  311 312'

	EXEC	-n 4 -d 5 -lb i j

	EXEC	-n 4 -d 5 -lc i j
		OUTPUT - $'     2  M^I M^J\n     4  M-I M-J'

	EXEC	-n 4 -d 5 -lbc i j
		OUTPUT - $'     2  211 M^I  212 M^J\n     4  311 M-I  312 M-J'

	EXEC	-n 5 -d 0 -l i j
		OUTPUT -

	EXEC	-n 5 -d 0 -lb i j

	EXEC	-n 5 -d 0 -lc i j

	EXEC	-n 5 -d 0 -lbc i j

	EXEC	-n 5 -d 1 -l i j
		OUTPUT - '     2  211 212'

	EXEC	-n 5 -d 1 -lb i j

	EXEC	-n 5 -d 1 -lc i j
		OUTPUT - '     2  M^I M^J'

	EXEC	-n 5 -d 1 -lbc i j
		OUTPUT - '     2  211 M^I  212 M^J'

	EXEC	-n 5 -d 2 -l i j
		OUTPUT - $'     2  211 212\n     4  311 312'

	EXEC	-n 5 -d 2 -lb i j

	EXEC	-n 5 -d 2 -lc i j
		OUTPUT - $'     2  M^I M^J\n     4  M-I M-J'

	EXEC	-n 5 -d 2 -lbc i j
		OUTPUT - $'     2  211 M^I  212 M^J\n     4  311 M-I  312 M-J'

	EXEC	-n 5 -d 3 -l i j
		OUTPUT - $'     2  211 212\n     4  311 312'

	EXEC	-n 5 -d 3 -lb i j

	EXEC	-n 5 -d 3 -lc i j
		OUTPUT - $'     2  M^I M^J\n     4  M-I M-J'

	EXEC	-n 5 -d 3 -lbc i j
		OUTPUT - $'     2  211 M^I  212 M^J\n     4  311 M-I  312 M-J'

	EXEC	-n 5 -d 4 -l i j
		OUTPUT - $'     2  211 212\n     4  311 312'

	EXEC	-n 5 -d 4 -lb i j

	EXEC	-n 5 -d 4 -lc i j
		OUTPUT - $'     2  M^I M^J\n     4  M-I M-J'

	EXEC	-n 5 -d 4 -lbc i j
		OUTPUT - $'     2  211 M^I  212 M^J\n     4  311 M-I  312 M-J'

	EXEC	-n 5 -d 5 -l i j
		OUTPUT - $'     2  211 212\n     4  311 312'

	EXEC	-n 5 -d 5 -lb i j

	EXEC	-n 5 -d 5 -lc i j
		OUTPUT - $'     2  M^I M^J\n     4  M-I M-J'

	EXEC	-n 5 -d 5 -lbc i j
		OUTPUT - $'     2  211 M^I  212 M^J\n     4  311 M-I  312 M-J'

	EXEC	-n 6 -d 0 -l i j
		OUTPUT -

	EXEC	-n 6 -d 0 -lb i j

	EXEC	-n 6 -d 0 -lc i j

	EXEC	-n 6 -d 0 -lbc i j

	EXEC	-n 6 -d 1 -l i j
		OUTPUT - '     2  211 212'

	EXEC	-n 6 -d 1 -lb i j

	EXEC	-n 6 -d 1 -lc i j
		OUTPUT - '     2  M^I M^J'

	EXEC	-n 6 -d 1 -lbc i j
		OUTPUT - '     2  211 M^I  212 M^J'

	EXEC	-n 6 -d 2 -l i j
		OUTPUT - $'     2  211 212\n     4  311 312'

	EXEC	-n 6 -d 2 -lb i j

	EXEC	-n 6 -d 2 -lc i j
		OUTPUT - $'     2  M^I M^J\n     4  M-I M-J'

	EXEC	-n 6 -d 2 -lbc i j
		OUTPUT - $'     2  211 M^I  212 M^J\n     4  311 M-I  312 M-J'

	EXEC	-n 6 -d 3 -l i j
		OUTPUT - $'     2  211 212\n     4  311 312\n     6  011 012'

	EXEC	-n 6 -d 3 -lb i j

	EXEC	-n 6 -d 3 -lc i j
		OUTPUT - $'     2  M^I M^J\n     4  M-I M-J\n     6   ^I  ^J'

	EXEC	-n 6 -d 3 -lbc i j
		OUTPUT - $'     2  211 M^I  212 M^J\n     4  311 M-I  312 M-J\n     6  011  ^I  012  ^J'

	EXEC	-n 6 -d 4 -l i j
		OUTPUT - $'     2  211 212\n     4  311 312\n     6  011 012'

	EXEC	-n 6 -d 4 -lb i j

	EXEC	-n 6 -d 4 -lc i j
		OUTPUT - $'     2  M^I M^J\n     4  M-I M-J\n     6   ^I  ^J'

	EXEC	-n 6 -d 4 -lbc i j
		OUTPUT - $'     2  211 M^I  212 M^J\n     4  311 M-I  312 M-J\n     6  011  ^I  012  ^J'

	EXEC	-n 6 -d 5 -l i j
		OUTPUT - $'     2  211 212\n     4  311 312\n     6  011 012'

	EXEC	-n 6 -d 5 -lb i j

	EXEC	-n 6 -d 5 -lc i j
		OUTPUT - $'     2  M^I M^J\n     4  M-I M-J\n     6   ^I  ^J'

	EXEC	-n 6 -d 5 -lbc i j
		OUTPUT - $'     2  211 M^I  212 M^J\n     4  311 M-I  312 M-J\n     6  011  ^I  012  ^J'

	EXEC	-n 7 -d 0 -l i j
		OUTPUT -

	EXEC	-n 7 -d 0 -lb i j

	EXEC	-n 7 -d 0 -lc i j

	EXEC	-n 7 -d 0 -lbc i j

	EXEC	-n 7 -d 1 -l i j
		OUTPUT - '     2  211 212'

	EXEC	-n 7 -d 1 -lb i j

	EXEC	-n 7 -d 1 -lc i j
		OUTPUT - '     2  M^I M^J'

	EXEC	-n 7 -d 1 -lbc i j
		OUTPUT - '     2  211 M^I  212 M^J'

	EXEC	-n 7 -d 2 -l i j
		OUTPUT - $'     2  211 212\n     4  311 312'

	EXEC	-n 7 -d 2 -lb i j

	EXEC	-n 7 -d 2 -lc i j
		OUTPUT - $'     2  M^I M^J\n     4  M-I M-J'

	EXEC	-n 7 -d 2 -lbc i j
		OUTPUT - $'     2  211 M^I  212 M^J\n     4  311 M-I  312 M-J'

	EXEC	-n 7 -d 3 -l i j
		OUTPUT - $'     2  211 212\n     4  311 312\n     6  011 012'

	EXEC	-n 7 -d 3 -lb i j

	EXEC	-n 7 -d 3 -lc i j
		OUTPUT - $'     2  M^I M^J\n     4  M-I M-J\n     6   ^I  ^J'

	EXEC	-n 7 -d 3 -lbc i j
		OUTPUT - $'     2  211 M^I  212 M^J\n     4  311 M-I  312 M-J\n     6  011  ^I  012  ^J'

	EXEC	-n 7 -d 4 -l i j
		OUTPUT - $'     2  211 212\n     4  311 312\n     6  011 012'

	EXEC	-n 7 -d 4 -lb i j

	EXEC	-n 7 -d 4 -lc i j
		OUTPUT - $'     2  M^I M^J\n     4  M-I M-J\n     6   ^I  ^J'

	EXEC	-n 7 -d 4 -lbc i j
		OUTPUT - $'     2  211 M^I  212 M^J\n     4  311 M-I  312 M-J\n     6  011  ^I  012  ^J'

	EXEC	-n 7 -d 5 -l i j
		OUTPUT - $'     2  211 212\n     4  311 312\n     6  011 012'

	EXEC	-n 7 -d 5 -lb i j

	EXEC	-n 7 -d 5 -lc i j
		OUTPUT - $'     2  M^I M^J\n     4  M-I M-J\n     6   ^I  ^J'

	EXEC	-n 7 -d 5 -lbc i j
		OUTPUT - $'     2  211 M^I  212 M^J\n     4  311 M-I  312 M-J\n     6  011  ^I  012  ^J'

	EXEC	-n 8 -d 0 -l i j
		OUTPUT -

	EXEC	-n 8 -d 0 -lb i j

	EXEC	-n 8 -d 0 -lc i j

	EXEC	-n 8 -d 0 -lbc i j

	EXEC	-n 8 -d 1 -l i j
		OUTPUT - '     2  211 212'

	EXEC	-n 8 -d 1 -lb i j

	EXEC	-n 8 -d 1 -lc i j
		OUTPUT - '     2  M^I M^J'

	EXEC	-n 8 -d 1 -lbc i j
		OUTPUT - '     2  211 M^I  212 M^J'

	EXEC	-n 8 -d 2 -l i j
		OUTPUT - $'     2  211 212\n     4  311 312'

	EXEC	-n 8 -d 2 -lb i j

	EXEC	-n 8 -d 2 -lc i j
		OUTPUT - $'     2  M^I M^J\n     4  M-I M-J'

	EXEC	-n 8 -d 2 -lbc i j
		OUTPUT - $'     2  211 M^I  212 M^J\n     4  311 M-I  312 M-J'

	EXEC	-n 8 -d 3 -l i j
		OUTPUT - $'     2  211 212\n     4  311 312\n     6  011 012'

	EXEC	-n 8 -d 3 -lb i j

	EXEC	-n 8 -d 3 -lc i j
		OUTPUT - $'     2  M^I M^J\n     4  M-I M-J\n     6   ^I  ^J'

	EXEC	-n 8 -d 3 -lbc i j
		OUTPUT - $'     2  211 M^I  212 M^J\n     4  311 M-I  312 M-J\n     6  011  ^I  012  ^J'

	EXEC	-n 8 -d 4 -l i j
		OUTPUT - $'     2  211 212\n     4  311 312\n     6  011 012\n     8  111 112'

	EXEC	-n 8 -d 4 -lb i j

	EXEC	-n 8 -d 4 -lc i j
		OUTPUT - $'     2  M^I M^J\n     4  M-I M-J\n     6   ^I  ^J\n     8    I   J'

	EXEC	-n 8 -d 4 -lbc i j
		OUTPUT - $'     2  211 M^I  212 M^J
     4  311 M-I  312 M-J
     6  011  ^I  012  ^J
     8  111   I  112   J'

	EXEC	-n 8 -d 5 -l i j
		OUTPUT - $'     2  211 212\n     4  311 312\n     6  011 012\n     8  111 112'

	EXEC	-n 8 -d 5 -lb i j

	EXEC	-n 8 -d 5 -lc i j
		OUTPUT - $'     2  M^I M^J\n     4  M-I M-J\n     6   ^I  ^J\n     8    I   J'

	EXEC	-n 8 -d 5 -lbc i j
		OUTPUT - $'     2  211 M^I  212 M^J
     4  311 M-I  312 M-J
     6  011  ^I  012  ^J
     8  111   I  112   J'

	EXEC	-n 9 -d 0 -l i j
		OUTPUT -

	EXEC	-n 9 -d 0 -lb i j

	EXEC	-n 9 -d 0 -lc i j

	EXEC	-n 9 -d 0 -lbc i j

	EXEC	-n 9 -d 1 -l i j
		OUTPUT - '     2  211 212'

	EXEC	-n 9 -d 1 -lb i j

	EXEC	-n 9 -d 1 -lc i j
		OUTPUT - '     2  M^I M^J'

	EXEC	-n 9 -d 1 -lbc i j
		OUTPUT - '     2  211 M^I  212 M^J'

	EXEC	-n 9 -d 2 -l i j
		OUTPUT - $'     2  211 212\n     4  311 312'

	EXEC	-n 9 -d 2 -lb i j

	EXEC	-n 9 -d 2 -lc i j
		OUTPUT - $'     2  M^I M^J\n     4  M-I M-J'

	EXEC	-n 9 -d 2 -lbc i j
		OUTPUT - $'     2  211 M^I  212 M^J\n     4  311 M-I  312 M-J'

	EXEC	-n 9 -d 3 -l i j
		OUTPUT - $'     2  211 212\n     4  311 312\n     6  011 012'

	EXEC	-n 9 -d 3 -lb i j

	EXEC	-n 9 -d 3 -lc i j
		OUTPUT - $'     2  M^I M^J\n     4  M-I M-J\n     6   ^I  ^J'

	EXEC	-n 9 -d 3 -lbc i j
		OUTPUT - $'     2  211 M^I  212 M^J\n     4  311 M-I  312 M-J\n     6  011  ^I  012  ^J'

	EXEC	-n 9 -d 4 -l i j
		OUTPUT - $'     2  211 212\n     4  311 312\n     6  011 012\n     8  111 112'

	EXEC	-n 9 -d 4 -lb i j

	EXEC	-n 9 -d 4 -lc i j
		OUTPUT - $'     2  M^I M^J\n     4  M-I M-J\n     6   ^I  ^J\n     8    I   J'

	EXEC	-n 9 -d 4 -lbc i j
		OUTPUT - $'     2  211 M^I  212 M^J
     4  311 M-I  312 M-J
     6  011  ^I  012  ^J
     8  111   I  112   J'

	EXEC	-n 9 -d 5 -l i j
		OUTPUT - $'     2  211 212\n     4  311 312\n     6  011 012\n     8  111 112'

	EXEC	-n 9 -d 5 -lb i j

	EXEC	-n 9 -d 5 -lc i j
		OUTPUT - $'     2  M^I M^J\n     4  M-I M-J\n     6   ^I  ^J\n     8    I   J'

	EXEC	-n 9 -d 5 -lbc i j
		OUTPUT - $'     2  211 M^I  212 M^J
     4  311 M-I  312 M-J
     6  011  ^I  012  ^J
     8  111   I  112   J'

	EXEC	-n 10 -d 0 -l i j
		OUTPUT -

	EXEC	-n 10 -d 0 -lb i j

	EXEC	-n 10 -d 0 -lc i j

	EXEC	-n 10 -d 0 -lbc i j

	EXEC	-n 10 -d 1 -l i j
		OUTPUT - '     2  211 212'

	EXEC	-n 10 -d 1 -lb i j

	EXEC	-n 10 -d 1 -lc i j
		OUTPUT - '     2  M^I M^J'

	EXEC	-n 10 -d 1 -lbc i j
		OUTPUT - '     2  211 M^I  212 M^J'

	EXEC	-n 10 -d 2 -l i j
		OUTPUT - $'     2  211 212\n     4  311 312'

	EXEC	-n 10 -d 2 -lb i j

	EXEC	-n 10 -d 2 -lc i j
		OUTPUT - $'     2  M^I M^J\n     4  M-I M-J'

	EXEC	-n 10 -d 2 -lbc i j
		OUTPUT - $'     2  211 M^I  212 M^J\n     4  311 M-I  312 M-J'

	EXEC	-n 10 -d 3 -l i j
		OUTPUT - $'     2  211 212\n     4  311 312\n     6  011 012'

	EXEC	-n 10 -d 3 -lb i j

	EXEC	-n 10 -d 3 -lc i j
		OUTPUT - $'     2  M^I M^J\n     4  M-I M-J\n     6   ^I  ^J'

	EXEC	-n 10 -d 3 -lbc i j
		OUTPUT - $'     2  211 M^I  212 M^J\n     4  311 M-I  312 M-J\n     6  011  ^I  012  ^J'

	EXEC	-n 10 -d 4 -l i j
		OUTPUT - $'     2  211 212\n     4  311 312\n     6  011 012\n     8  111 112'

	EXEC	-n 10 -d 4 -lb i j

	EXEC	-n 10 -d 4 -lc i j
		OUTPUT - $'     2  M^I M^J\n     4  M-I M-J\n     6   ^I  ^J\n     8    I   J'

	EXEC	-n 10 -d 4 -lbc i j
		OUTPUT - $'     2  211 M^I  212 M^J
     4  311 M-I  312 M-J
     6  011  ^I  012  ^J
     8  111   I  112   J'

	EXEC	-n 10 -d 5 -l i j
		OUTPUT - $'     2  211 212\n     4  311 312\n     6  011 012\n     8  111 112'

	EXEC	-n 10 -d 5 -lb i j

	EXEC	-n 10 -d 5 -lc i j
		OUTPUT - $'     2  M^I M^J\n     4  M-I M-J\n     6   ^I  ^J\n     8    I   J'

	EXEC	-n 10 -d 5 -lbc i j
		OUTPUT - $'     2  211 M^I  212 M^J
     4  311 M-I  312 M-J
     6  011  ^I  012  ^J
     8  111   I  112   J'

	EXEC	-n 11 -d 0 -l i j
		OUTPUT -

	EXEC	-n 11 -d 0 -lb i j

	EXEC	-n 11 -d 0 -lc i j

	EXEC	-n 11 -d 0 -lbc i j

	EXEC	-n 11 -d 1 -l i j
		OUTPUT - '     2  211 212'

	EXEC	-n 11 -d 1 -lb i j

	EXEC	-n 11 -d 1 -lc i j
		OUTPUT - '     2  M^I M^J'

	EXEC	-n 11 -d 1 -lbc i j
		OUTPUT - '     2  211 M^I  212 M^J'

	EXEC	-n 11 -d 2 -l i j
		OUTPUT - $'     2  211 212\n     4  311 312'

	EXEC	-n 11 -d 2 -lb i j

	EXEC	-n 11 -d 2 -lc i j
		OUTPUT - $'     2  M^I M^J\n     4  M-I M-J'

	EXEC	-n 11 -d 2 -lbc i j
		OUTPUT - $'     2  211 M^I  212 M^J\n     4  311 M-I  312 M-J'

	EXEC	-n 11 -d 3 -l i j
		OUTPUT - $'     2  211 212\n     4  311 312\n     6  011 012'

	EXEC	-n 11 -d 3 -lb i j

	EXEC	-n 11 -d 3 -lc i j
		OUTPUT - $'     2  M^I M^J\n     4  M-I M-J\n     6   ^I  ^J'

	EXEC	-n 11 -d 3 -lbc i j
		OUTPUT - $'     2  211 M^I  212 M^J\n     4  311 M-I  312 M-J\n     6  011  ^I  012  ^J'

	EXEC	-n 11 -d 4 -l i j
		OUTPUT - $'     2  211 212\n     4  311 312\n     6  011 012\n     8  111 112'

	EXEC	-n 11 -d 4 -lb i j

	EXEC	-n 11 -d 4 -lc i j
		OUTPUT - $'     2  M^I M^J\n     4  M-I M-J\n     6   ^I  ^J\n     8    I   J'

	EXEC	-n 11 -d 4 -lbc i j
		OUTPUT - $'     2  211 M^I  212 M^J
     4  311 M-I  312 M-J
     6  011  ^I  012  ^J
     8  111   I  112   J'

	EXEC	-n 11 -d 5 -l i j
		OUTPUT - $'     2  211 212\n     4  311 312\n     6  011 012\n     8  111 112'

	EXEC	-n 11 -d 5 -lb i j

	EXEC	-n 11 -d 5 -lc i j
		OUTPUT - $'     2  M^I M^J\n     4  M-I M-J\n     6   ^I  ^J\n     8    I   J'

	EXEC	-n 11 -d 5 -lbc i j
		OUTPUT - $'     2  211 M^I  212 M^J
     4  311 M-I  312 M-J
     6  011  ^I  012  ^J
     8  111   I  112   J'

	EXEC	-i 0:0 -l i j
		OUTPUT - $'     2  211 212\n     4  311 312\n     6  011 012\n     8  111 112'

	EXEC	-i 0:1 -l i j
		OUTPUT - $'     1  141 212
     2  211 142
     3  142 312
     4  311 143
     5  143 012
     6  011 144
     7  144 112
     8  111 145
     9  145 012'
		ERROR - 'cmp: EOF on j'

	EXEC	-i 0:2 -l i j
		OUTPUT - $'     1  141 142
     2  211 312
     3  142 143
     4  311 012
     5  143 144
     6  011 112
     7  144 145
     8  111 012'

	EXEC	-i 0:3 -l i j
		OUTPUT - $'     1  141 312
     2  211 143
     3  142 012
     4  311 144
     5  143 112
     6  011 145
     7  144 012'

	EXEC	-i 0:4 -l i j
		OUTPUT - $'     1  141 143
     2  211 012
     3  142 144
     4  311 112
     5  143 145
     6  011 012'

	EXEC	-i 0:5 -l i j
		OUTPUT - $'     1  141 012
     2  211 144
     3  142 112
     4  311 145
     5  143 012'

	EXEC	-i 0:6 -l i j
		OUTPUT - $'     1  141 144\n     2  211 112\n     3  142 145\n     4  311 012'

	EXEC	-i 0:7 -l i j
		OUTPUT - $'     1  141 112\n     2  211 145\n     3  142 012'

	EXEC	-i 0:8 -l i j
		OUTPUT - $'     1  141 145\n     2  211 012'

	EXEC	-i 0:9 -l i j
		OUTPUT - '     1  141 012'

	EXEC	-i 0:10 -l i j
		OUTPUT -

	EXEC	-i 0:11 -l i j

	EXEC	-i 1:0 -l i j
		OUTPUT - $'     1  211 141
     2  142 212
     3  311 142
     4  143 312
     5  011 143
     6  144 012
     7  111 144
     8  145 112
     9  012 145'
		ERROR - 'cmp: EOF on i'

	EXEC	-i 1:1 -l i j
		OUTPUT - $'     1  211 212\n     3  311 312\n     5  011 012\n     7  111 112'
		ERROR -n -

	EXEC	-i 1:2 -l i j
		OUTPUT - $'     1  211 142
     2  142 312
     3  311 143
     4  143 012
     5  011 144
     6  144 112
     7  111 145
     8  145 012'
		ERROR - 'cmp: EOF on j'

	EXEC	-i 1:3 -l i j
		OUTPUT - $'     1  211 312
     2  142 143
     3  311 012
     4  143 144
     5  011 112
     6  144 145
     7  111 012'

	EXEC	-i 1:4 -l i j
		OUTPUT - $'     1  211 143
     2  142 012
     3  311 144
     4  143 112
     5  011 145
     6  144 012'

	EXEC	-i 1:5 -l i j
		OUTPUT - $'     1  211 012
     2  142 144
     3  311 112
     4  143 145
     5  011 012'

	EXEC	-i 1:6 -l i j
		OUTPUT - $'     1  211 144\n     2  142 112\n     3  311 145\n     4  143 012'

	EXEC	-i 1:7 -l i j
		OUTPUT - $'     1  211 112\n     2  142 145\n     3  311 012'

	EXEC	-i 1:8 -l i j
		OUTPUT - $'     1  211 145\n     2  142 012'

	EXEC	-i 1:9 -l i j
		OUTPUT - '     1  211 012'

	EXEC	-i 1:10 -l i j
		OUTPUT -

	EXEC	-i 1:11 -l i j

	EXEC	-i 2:0 -l i j
		OUTPUT - $'     1  142 141
     2  311 212
     3  143 142
     4  011 312
     5  144 143
     6  111 012
     7  145 144
     8  012 112'
		ERROR - 'cmp: EOF on i'

	EXEC	-i 2:1 -l i j
		OUTPUT - $'     1  142 212
     2  311 142
     3  143 312
     4  011 143
     5  144 012
     6  111 144
     7  145 112
     8  012 145'

	EXEC	-i 2:2 -l i j
		OUTPUT - $'     2  311 312\n     4  011 012\n     6  111 112'
		ERROR -n -

	EXEC	-i 2:3 -l i j
		OUTPUT - $'     1  142 312
     2  311 143
     3  143 012
     4  011 144
     5  144 112
     6  111 145
     7  145 012'
		ERROR - 'cmp: EOF on j'

	EXEC	-i 2:4 -l i j
		OUTPUT - $'     1  142 143
     2  311 012
     3  143 144
     4  011 112
     5  144 145
     6  111 012'

	EXEC	-i 2:5 -l i j
		OUTPUT - $'     1  142 012
     2  311 144
     3  143 112
     4  011 145
     5  144 012'

	EXEC	-i 2:6 -l i j
		OUTPUT - $'     1  142 144\n     2  311 112\n     3  143 145\n     4  011 012'

	EXEC	-i 2:7 -l i j
		OUTPUT - $'     1  142 112\n     2  311 145\n     3  143 012'

	EXEC	-i 2:8 -l i j
		OUTPUT - $'     1  142 145\n     2  311 012'

	EXEC	-i 2:9 -l i j
		OUTPUT - '     1  142 012'

	EXEC	-i 2:10 -l i j
		OUTPUT -

	EXEC	-i 2:11 -l i j

	EXEC	-i 3:0 -l i j
		OUTPUT - $'     1  311 141
     2  143 212
     3  011 142
     4  144 312
     5  111 143
     6  145 012
     7  012 144'
		ERROR - 'cmp: EOF on i'

	EXEC	-i 3:1 -l i j
		OUTPUT - $'     1  311 212
     2  143 142
     3  011 312
     4  144 143
     5  111 012
     6  145 144
     7  012 112'

	EXEC	-i 3:2 -l i j
		OUTPUT - $'     1  311 142
     2  143 312
     3  011 143
     4  144 012
     5  111 144
     6  145 112
     7  012 145'

	EXEC	-i 3:3 -l i j
		OUTPUT - $'     1  311 312\n     3  011 012\n     5  111 112'
		ERROR -n -

	EXEC	-i 3:4 -l i j
		OUTPUT - $'     1  311 143
     2  143 012
     3  011 144
     4  144 112
     5  111 145
     6  145 012'
		ERROR - 'cmp: EOF on j'

	EXEC	-i 3:5 -l i j
		OUTPUT - $'     1  311 012
     2  143 144
     3  011 112
     4  144 145
     5  111 012'

	EXEC	-i 3:6 -l i j
		OUTPUT - $'     1  311 144\n     2  143 112\n     3  011 145\n     4  144 012'

	EXEC	-i 3:7 -l i j
		OUTPUT - $'     1  311 112\n     2  143 145\n     3  011 012'

	EXEC	-i 3:8 -l i j
		OUTPUT - $'     1  311 145\n     2  143 012'

	EXEC	-i 3:9 -l i j
		OUTPUT - '     1  311 012'

	EXEC	-i 3:10 -l i j
		OUTPUT -

	EXEC	-i 3:11 -l i j

	EXEC	-i 4:0 -l i j
		OUTPUT - $'     1  143 141
     2  011 212
     3  144 142
     4  111 312
     5  145 143'
		ERROR - 'cmp: EOF on i'

	EXEC	-i 4:1 -l i j
		OUTPUT - $'     1  143 212
     2  011 142
     3  144 312
     4  111 143
     5  145 012
     6  012 144'

	EXEC	-i 4:2 -l i j
		OUTPUT - $'     1  143 142
     2  011 312
     3  144 143
     4  111 012
     5  145 144
     6  012 112'

	EXEC	-i 4:3 -l i j
		OUTPUT - $'     1  143 312
     2  011 143
     3  144 012
     4  111 144
     5  145 112
     6  012 145'

	EXEC	-i 4:4 -l i j
		OUTPUT - $'     2  011 012\n     4  111 112'
		ERROR -n -

	EXEC	-i 4:5 -l i j
		OUTPUT - $'     1  143 012
     2  011 144
     3  144 112
     4  111 145
     5  145 012'
		ERROR - 'cmp: EOF on j'

	EXEC	-i 4:6 -l i j
		OUTPUT - $'     1  143 144\n     2  011 112\n     3  144 145\n     4  111 012'

	EXEC	-i 4:7 -l i j
		OUTPUT - $'     1  143 112\n     2  011 145\n     3  144 012'

	EXEC	-i 4:8 -l i j
		OUTPUT - $'     1  143 145\n     2  011 012'

	EXEC	-i 4:9 -l i j
		OUTPUT - '     1  143 012'

	EXEC	-i 4:10 -l i j
		OUTPUT -

	EXEC	-i 4:11 -l i j

	EXEC	-i 5:0 -l i j
		OUTPUT - $'     1  011 141
     2  144 212
     3  111 142
     4  145 312
     5  012 143'
		ERROR - 'cmp: EOF on i'

	EXEC	-i 5:1 -l i j
		OUTPUT - $'     1  011 212\n     2  144 142\n     3  111 312\n     4  145 143'

	EXEC	-i 5:2 -l i j
		OUTPUT - $'     1  011 142
     2  144 312
     3  111 143
     4  145 012
     5  012 144'

	EXEC	-i 5:3 -l i j
		OUTPUT - $'     1  011 312
     2  144 143
     3  111 012
     4  145 144
     5  012 112'

	EXEC	-i 5:4 -l i j
		OUTPUT - $'     1  011 143
     2  144 012
     3  111 144
     4  145 112
     5  012 145'

	EXEC	-i 5:5 -l i j
		OUTPUT - $'     1  011 012\n     3  111 112'
		ERROR -n -

	EXEC	-i 5:6 -l i j
		OUTPUT - $'     1  011 144\n     2  144 112\n     3  111 145\n     4  145 012'
		ERROR - 'cmp: EOF on j'

	EXEC	-i 5:7 -l i j
		OUTPUT - $'     1  011 112\n     2  144 145\n     3  111 012'

	EXEC	-i 5:8 -l i j
		OUTPUT - $'     1  011 145\n     2  144 012'

	EXEC	-i 5:9 -l i j
		OUTPUT - '     1  011 012'

	EXEC	-i 5:10 -l i j
		OUTPUT -

	EXEC	-i 5:11 -l i j

	EXEC	-i 6:0 -l i j
		OUTPUT - $'     1  144 141\n     2  111 212\n     3  145 142\n     4  012 312'
		ERROR - 'cmp: EOF on i'

	EXEC	-i 6:1 -l i j
		OUTPUT - $'     1  144 212\n     2  111 142\n     3  145 312\n     4  012 143'

	EXEC	-i 6:2 -l i j
		OUTPUT - $'     1  144 142\n     2  111 312\n     3  145 143'

	EXEC	-i 6:3 -l i j
		OUTPUT - $'     1  144 312\n     2  111 143\n     3  145 012\n     4  012 144'

	EXEC	-i 6:4 -l i j
		OUTPUT - $'     1  144 143\n     2  111 012\n     3  145 144\n     4  012 112'

	EXEC	-i 6:5 -l i j
		OUTPUT - $'     1  144 012\n     2  111 144\n     3  145 112\n     4  012 145'

	EXEC	-i 6:6 -l i j
		OUTPUT - '     2  111 112'
		ERROR -n -

	EXEC	-i 6:7 -l i j
		OUTPUT - $'     1  144 112\n     2  111 145\n     3  145 012'
		ERROR - 'cmp: EOF on j'

	EXEC	-i 6:8 -l i j
		OUTPUT - $'     1  144 145\n     2  111 012'

	EXEC	-i 6:9 -l i j
		OUTPUT - '     1  144 012'

	EXEC	-i 6:10 -l i j
		OUTPUT -

	EXEC	-i 6:11 -l i j

	EXEC	-i 7:0 -l i j
		OUTPUT - $'     1  111 141\n     2  145 212\n     3  012 142'
		ERROR - 'cmp: EOF on i'

	EXEC	-i 7:1 -l i j
		OUTPUT - $'     1  111 212\n     2  145 142\n     3  012 312'

	EXEC	-i 7:2 -l i j
		OUTPUT - $'     1  111 142\n     2  145 312\n     3  012 143'

	EXEC	-i 7:3 -l i j
		OUTPUT - $'     1  111 312\n     2  145 143'

	EXEC	-i 7:4 -l i j
		OUTPUT - $'     1  111 143\n     2  145 012\n     3  012 144'

	EXEC	-i 7:5 -l i j
		OUTPUT - $'     1  111 012\n     2  145 144\n     3  012 112'

	EXEC	-i 7:6 -l i j
		OUTPUT - $'     1  111 144\n     2  145 112\n     3  012 145'

	EXEC	-i 7:7 -l i j
		OUTPUT - '     1  111 112'
		ERROR -n -

	EXEC	-i 7:8 -l i j
		OUTPUT - $'     1  111 145\n     2  145 012'
		ERROR - 'cmp: EOF on j'

	EXEC	-i 7:9 -l i j
		OUTPUT - '     1  111 012'

	EXEC	-i 7:10 -l i j
		OUTPUT -

	EXEC	-i 7:11 -l i j

	EXEC	-i 8:0 -l i j
		OUTPUT - $'     1  145 141\n     2  012 212'
		ERROR - 'cmp: EOF on i'

	EXEC	-i 8:1 -l i j
		OUTPUT - $'     1  145 212\n     2  012 142'

	EXEC	-i 8:2 -l i j
		OUTPUT - $'     1  145 142\n     2  012 312'

	EXEC	-i 8:3 -l i j
		OUTPUT - $'     1  145 312\n     2  012 143'

	EXEC	-i 8:4 -l i j
		OUTPUT - '     1  145 143'

	EXEC	-i 8:5 -l i j
		OUTPUT - $'     1  145 012\n     2  012 144'

	EXEC	-i 8:6 -l i j
		OUTPUT - $'     1  145 144\n     2  012 112'

	EXEC	-i 8:7 -l i j
		OUTPUT - $'     1  145 112\n     2  012 145'

	EXEC	-i 8:8 -l i j
		OUTPUT -
		ERROR -n -
		EXIT 0

	EXEC	-i 8:9 -l i j
		OUTPUT - '     1  145 012'
		ERROR - 'cmp: EOF on j'
		EXIT 1

	EXEC	-i 8:10 -l i j
		OUTPUT -

	EXEC	-i 8:11 -l i j

	EXEC	-i 9:0 -l i j
		OUTPUT - '     1  012 141'
		ERROR - 'cmp: EOF on i'

	EXEC	-i 9:1 -l i j
		OUTPUT - '     1  012 212'

	EXEC	-i 9:2 -l i j
		OUTPUT - '     1  012 142'

	EXEC	-i 9:3 -l i j
		OUTPUT - '     1  012 312'

	EXEC	-i 9:4 -l i j
		OUTPUT - '     1  012 143'

	EXEC	-i 9:5 -l i j
		OUTPUT -

	EXEC	-i 9:6 -l i j
		OUTPUT - '     1  012 144'

	EXEC	-i 9:7 -l i j
		OUTPUT - '     1  012 112'

	EXEC	-i 9:8 -l i j
		OUTPUT - '     1  012 145'

	EXEC	-i 9:9 -l i j
		OUTPUT -
		ERROR -n -
		EXIT 0

	EXEC	-i 9:10 -l i j
		ERROR - 'cmp: EOF on j'
		EXIT 1

	EXEC	-i 9:11 -l i j

	EXEC	-i 10:0 -l i j
		ERROR - 'cmp: EOF on i'

	EXEC	-i 10:1 -l i j

	EXEC	-i 10:2 -l i j

	EXEC	-i 10:3 -l i j

	EXEC	-i 10:4 -l i j

	EXEC	-i 10:5 -l i j

	EXEC	-i 10:6 -l i j

	EXEC	-i 10:7 -l i j

	EXEC	-i 10:8 -l i j

	EXEC	-i 10:9 -l i j

	EXEC	-i 10:10 -l i j
		ERROR -n -
		EXIT 0

	EXEC	-i 10:11 -l i j

	EXEC	-i 11:0 -l i j
		ERROR - 'cmp: EOF on i'
		EXIT 1

	EXEC	-i 11:1 -l i j

	EXEC	-i 11:2 -l i j

	EXEC	-i 11:3 -l i j

	EXEC	-i 11:4 -l i j

	EXEC	-i 11:5 -l i j

	EXEC	-i 11:6 -l i j

	EXEC	-i 11:7 -l i j

	EXEC	-i 11:8 -l i j

	EXEC	-i 11:9 -l i j

	EXEC	-i 11:10 -l i j
		ERROR -n -
		EXIT 0

	EXEC	-i 11:11 -l i j
