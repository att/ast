# tests for the xargs utility

TEST 01 'basics'
	EXEC echo test
		INPUT -
		OUTPUT - $'test'
	EXEC echo test
		INPUT - $'first\nlast'
		OUTPUT - $'test first last'

TEST 02 'arg replacement'
	EXEC -i echo test {}
		INPUT - $'first\nlast'
		OUTPUT - $'test first\ntest last'
	EXEC -i{} echo test {}
	EXEC -iARG echo test ARG
	EXEC -i echo { {} }
		OUTPUT - $'{ first }\n{ last }'
	EXEC -i{} echo { {} }
	EXEC -iARG echo { ARG }
	EXEC -i echo {{}}
		OUTPUT - $'{first}\n{last}'
	EXEC -i echo A{}Z
		OUTPUT - $'AfirstZ\nAlastZ'
	EXEC -IARG echo AARGZ
		OUTPUT - $'AfirstZ\nAlastZ'
	EXEC -I ARG echo AARGZ
		OUTPUT - $'AfirstZ\nAlastZ'
	EXEC -i echo test {}
		INPUT - $'aaa\tbbb\nyyy zzz'
		OUTPUT - $'test aaa\tbbb\ntest yyy zzz'

TEST 03 'arg limit'
	EXEC -n2 echo
		INPUT - $'1\n2\n3\n4'
		OUTPUT - $'1 2\n3 4'
	EXEC -n 2 echo
	EXEC -n2 echo
		INPUT - $'1\n2\n3\n4\n5'
		OUTPUT - $'1 2\n3 4\n5'

#TBD#TEST 04 'size limit'
#TBD#	EXEC
#TBD#	size=(
#TBD#		$($COMMAND -s1 echo 1 </dev/null 2>&1 | sed 's/[^0-9]//g') 
#TBD#		$($COMMAND -s1 echo 1 2 </dev/null 2>&1 | sed 's/[^0-9]//g') 
#TBD#		$($COMMAND -s1 echo 1 2 3 </dev/null 2>&1 | sed 's/[^0-9]//g') 
#TBD#		$($COMMAND -s1 echo 1 2 3 4 </dev/null 2>&1 | sed 's/[^0-9]//g') 
#TBD#	)
#TBD#	EXEC -s${size[0]} echo
#TBD#		INPUT - $'1\n2\n3\n4'
#TBD#		OUTPUT - $'1\n2\n3\n4'
#TBD#	EXEC -s${size[1]} echo
#TBD#		INPUT - $'1\n2\n3\n4'
#TBD#		OUTPUT - $'1 2\n3 4'
#TBD#	EXEC -s${size[2]} echo
#TBD#		INPUT - $'1\n2\n3\n4'
#TBD#		OUTPUT - $'1 2 3\n4'
#TBD#	EXEC -s${size[3]} echo
#TBD#		INPUT - $'1\n2\n3\n4'
#TBD#		OUTPUT - $'1 2 3 4'

TEST 05 'extensions'
	EXEC -z echo test
		INPUT -
		INPUT -
		OUTPUT -
