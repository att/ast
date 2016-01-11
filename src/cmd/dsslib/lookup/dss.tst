# regression tests for the dss lookup query

TITLE + lookup

export TZ=EST5EDT LC_ALL=C

VIEW data

TEST 01 'basics'

	EXEC -I$data -x xml::'<METHOD>xml</><XML><FIELD><NAME>id</><TYPE>string</></></>' '(delete==0)|{lookup "delete.dat" id}?{count}' test.json
		OUTPUT - $'5/95'

	EXEC -I$data -x xml::'<METHOD>xml</><XML><FIELD><NAME>id</><TYPE>string</></></>' '(delete==0)|{lookup "delete.dat" id}?:{count}' test.json
		OUTPUT - $'90/95'
