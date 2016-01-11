# ast nmake statement tests

INCLUDE cc.def

TEST 01 'for with break and continue'

	EXEC	-n
		INPUT Makefile $'all : t.1 t.2 t.3 t.4 t.5 t.6
t.1 : .MAKE
	local i j
	print $(<).beg
	for i 1 2
		for j 3 4
			print $(<).$(i).$(j)
		end
	end
	print $(<).end
t.2 : .MAKE
	local i j
	print $(<).beg
	for i 1 2
		for j 3 4
			print $(<).$(i).$(j)
			break
			print $(<).j.break.error
		end
	end
	print $(<).end
t.3 : .MAKE
	local i j
	print $(<).beg
	for i 1 2
		for j 3 4
			print $(<).$(i).$(j)
			break
			print $(<).j.break.error
		end
		break
		print $(<).i.break.error
	end
	print $(<).end
t.4 : .MAKE
	local i j
	print $(<).beg
	for i 1 2
		for j 3 4
			print $(<).$(i).$(j)
			break 2
			print $(<).j.break.error
		end
		print $(<).i.break.error
	end
	print $(<).end
t.5 : .MAKE
	local i j
	print $(<).beg
	for i 1 2
		for j 3 4
			print $(<).$(i).$(j)
			continue
			print $(<).j.continue.error
		end
		continue
		print $(<).i.continue.error
	end
	print $(<).end
t.6 : .MAKE
	local i j
	print $(<).beg
	for i 1 2
		for j 3 4
			print $(<).$(i).$(j)
			continue 2
			print $(<).j.continue.error
		end
		print $(<).i.continue.error
	end
	print $(<).end'
		OUTPUT - $'t.1.beg
t.1.1.3
t.1.1.4
t.1.2.3
t.1.2.4
t.1.end
t.2.beg
t.2.1.3
t.2.2.3
t.2.end
t.3.beg
t.3.1.3
t.3.end
t.4.beg
t.4.1.3
t.4.end
t.5.beg
t.5.1.3
t.5.1.4
t.5.2.3
t.5.2.4
t.5.end
t.6.beg
t.6.1.3
t.6.2.3
t.6.end'

TEST 02 'for with local variables'

	EXEC	-n
		INPUT Makefile $'all : t.1
t.1 : .MAKE .FORCE .REPEAT
	local i
	print $(<).beg
	for i 1 2
		make t.2
	end
	print $(<).end
t.2 : .MAKE .FORCE .REPEAT
	local i
	print $(<).beg
	for i 1 2
		make t.3
	end
	print $(<).end
t.3 : .MAKE .FORCE .REPEAT
	local i
	print $(<).beg
	for i 1 2
		make t.4
	end
	print $(<).end
t.4 : .MAKE .FORCE .REPEAT
	local i
	print $(<).beg
	for i 1 2
		make t.5
	end
	print $(<).end
t.5 : .MAKE .FORCE .REPEAT
	local i
	print $(<).beg
	for i 1 2
		make t.6
	end
	print $(<).end
t.6 : .MAKE .FORCE .REPEAT
	local i
	print $(<).beg
	for i 1 2
		print $(<).$(i)
	end
	print $(<).end'
		OUTPUT - $'t.1.beg
t.2.beg
t.3.beg
t.4.beg
t.5.beg
t.6.beg
t.6.1
t.6.2
t.6.end
t.6.beg
t.6.1
t.6.2
t.6.end
t.5.end
t.5.beg
t.6.beg
t.6.1
t.6.2
t.6.end
t.6.beg
t.6.1
t.6.2
t.6.end
t.5.end
t.4.end
t.4.beg
t.5.beg
t.6.beg
t.6.1
t.6.2
t.6.end
t.6.beg
t.6.1
t.6.2
t.6.end
t.5.end
t.5.beg
t.6.beg
t.6.1
t.6.2
t.6.end
t.6.beg
t.6.1
t.6.2
t.6.end
t.5.end
t.4.end
t.3.end
t.3.beg
t.4.beg
t.5.beg
t.6.beg
t.6.1
t.6.2
t.6.end
t.6.beg
t.6.1
t.6.2
t.6.end
t.5.end
t.5.beg
t.6.beg
t.6.1
t.6.2
t.6.end
t.6.beg
t.6.1
t.6.2
t.6.end
t.5.end
t.4.end
t.4.beg
t.5.beg
t.6.beg
t.6.1
t.6.2
t.6.end
t.6.beg
t.6.1
t.6.2
t.6.end
t.5.end
t.5.beg
t.6.beg
t.6.1
t.6.2
t.6.end
t.6.beg
t.6.1
t.6.2
t.6.end
t.5.end
t.4.end
t.3.end
t.2.end
t.2.beg
t.3.beg
t.4.beg
t.5.beg
t.6.beg
t.6.1
t.6.2
t.6.end
t.6.beg
t.6.1
t.6.2
t.6.end
t.5.end
t.5.beg
t.6.beg
t.6.1
t.6.2
t.6.end
t.6.beg
t.6.1
t.6.2
t.6.end
t.5.end
t.4.end
t.4.beg
t.5.beg
t.6.beg
t.6.1
t.6.2
t.6.end
t.6.beg
t.6.1
t.6.2
t.6.end
t.5.end
t.5.beg
t.6.beg
t.6.1
t.6.2
t.6.end
t.6.beg
t.6.1
t.6.2
t.6.end
t.5.end
t.4.end
t.3.end
t.3.beg
t.4.beg
t.5.beg
t.6.beg
t.6.1
t.6.2
t.6.end
t.6.beg
t.6.1
t.6.2
t.6.end
t.5.end
t.5.beg
t.6.beg
t.6.1
t.6.2
t.6.end
t.6.beg
t.6.1
t.6.2
t.6.end
t.5.end
t.4.end
t.4.beg
t.5.beg
t.6.beg
t.6.1
t.6.2
t.6.end
t.6.beg
t.6.1
t.6.2
t.6.end
t.5.end
t.5.beg
t.6.beg
t.6.1
t.6.2
t.6.end
t.6.beg
t.6.1
t.6.2
t.6.end
t.5.end
t.4.end
t.3.end
t.2.end
t.1.end'

TEST 03 'print, read and .VIRTUAL status'

	EXEC	-n
		INPUT Makefile $'.ASK : .MAKE .VIRTUAL .FORCE .REPEAT
	local ans
	print -n -u 2 Do you really want to make $(<<)?$(" ")
	read -u 0 ans
	return $(ans:N=[Yy]*:?0?-1?)
all : a b c
a : .ASK
	echo make $(<)
b :
	echo make $(<)
c : .ASK
	echo make $(<)'
		INPUT - $'no
yes'
		OUTPUT - $'+ echo make b
+ echo make c'
		ERROR -n - $'Do you really want to make a? Do you really want to make c? '

TEST 04 'here actions'

	EXEC	-n
		INPUT Makefile $'all : a b
a :: a.c  <<  !  junk
this is the action
!
b : << EOF
and the other action
EOF'
		INPUT a.c
		OUTPUT - $'+ cc -O   -c a.c
+ this is the action
+ and the other action'

	EXEC	-n
		INPUT BABEL.mk $'":BABEL:" : .MAKE .OPERATOR
	eval
	$(<) : $(>)
		$(@:V)
	end'
		INPUT Makefile $'a :BABEL: b <<!!
/* this is not a comment
// neither is this
or /* this
or */ that
this # is not a comment
# neither is this
this " is not a quote
neither " is this
\' nor this
or \' this
! this is not a here eof
!!! neither is this
finally convert $(*) to $(<)
!!'
		INPUT b
		OUTPUT - $'+ /* this is not a comment
+ // neither is this
+ or /* this
+ or */ that
+ this # is not a comment
+ # neither is this
+ this " is not a quote
+ neither " is this
+ \' nor this
+ or \' this
+ ! this is not a here eof
+ !!! neither is this
+ finally convert b to a'

TEST 05 'include + .SOURCE.mk'

	EXEC	-n
		INPUT Makefile $'.SOURCE.mk : mk
include "sans"
include "tst.mk"
all :
	: SANS=$(SANS) :
	: TEST=$(TEST) :'
		INPUT mk/sans $'SANS = 1'
		INPUT mk/tst.mk $'TEST = 1'
		OUTPUT - $'+ : SANS=1 :
+ : TEST=1 :'

TEST 06 'open and print'

	EXEC
		INPUT Makefile $'all : .MAKE
	print -n -u9 -ofoo.dat
	print -u9 aha'
		OUTPUT foo.dat $'aha'

TEST 07 'obsolete makefile pp'

	EXEC	-n MAKEPP=cpp MAKEPPFLAGS="-I-D -I."
		INPUT Makefile $'#include "pp.mk"'
		INPUT pp.mk $'all :
	: pp'
		OUTPUT - $'+ : pp'
		ERROR - $'+ cpp -I-D -I. Makefile'

TEST 08 'include vs. .SOURCE.mk'

	EXEC	-n
		INPUT Makefile $'.SOURCE.mk : include\ninclude "rules.mk"'
		INPUT include/rules.mk $'all :\n\t: rules'
		OUTPUT - $'+ : rules'

TEST 09 'rules'

	EXEC	-n do.nothing
		INPUT Makefile $'
all :
	: $(<)'

	EXEC	-n do.nothing
		INPUT Makefile $'rules
all :
	: $(<)'
		ERROR - $'make: don\'t know how to make do.nothing'
		EXIT 1

	EXEC	-n do.nothing
		INPUT Makefile $'/* comment counts as a statement -- ouch */
rules
all :
	: $(<)'
		ERROR - $'make: "Makefile", line 2: : incompatible with current base rules makerules'
		EXIT 1

TEST 10 'local ( formals ) actuals'

	EXEC	-n ARGS="1 2"
		INPUT Makefile $'.TEST. : .FUNCTION
	local ( A B ) $(%)
	return A $(A) :
all :
	: $(.TEST. $(ARGS))'
		OUTPUT - $'+ : A 1 :'

	EXEC	-n ARGS=""
		OUTPUT -
		ERROR - $'make: ".TEST.", line 1: A: actual argument expected'
		EXIT 1

	EXEC	-n ARGS="1"
		ERROR - $'make: ".TEST.", line 1: B: actual argument expected'

	EXEC	-n ARGS="1 2 3"
		ERROR - $'make: ".TEST.", line 1: 3: only 2 actual arguments expected'

	EXEC	-n ARGS=""
		INPUT Makefile $'C = OOPS
.TEST. : .FUNCTION
	local ( A B C ... ) $(%)
	return A $(A) : B $(B) : C $(C) :
all :
	: $(.TEST. $(ARGS))'
		ERROR - $'make: ".TEST.", line 1: A: actual argument expected'

	EXEC	-n ARGS="1"
		ERROR - $'make: ".TEST.", line 1: B: actual argument expected'

	EXEC	-n ARGS="1 2"
		OUTPUT - $'+ : A 1 : B 2 : C  :'
		ERROR -
		EXIT 0

	EXEC	-n ARGS="1 2 3"
		OUTPUT - $'+ : A 1 : B 2 : C 3 :'

	EXEC	-n ARGS="1 2 3 4"
		OUTPUT - $'+ : A 1 : B 2 : C 3 4 :'
