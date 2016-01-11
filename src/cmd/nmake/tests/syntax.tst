# ast nmake syntax tests

INCLUDE test.def

TEST 01 'comment lines'

	EXEC	-n
		INPUT Makefile $'
all : .MAKE .VIRTUAL .FORCE
	local T
	for T $(...:N=test-*:H)
		print === $(T) ===
		print "$(@$(T):V)"
		make $(T)
		print "$(@$(T):V)"
	end
test-C-1 : .MAKE .VIRTUAL .FORCE
	/*
	 * leading C style test
	 */
	print : $(<) 4 :
test-C-2 : .MAKE .VIRTUAL .FORCE
	print : $(<) 1 :
	/*
	 * embedded C style test
	 */
	print : $(<) 5 :
test-C-3 : .MAKE .VIRTUAL .FORCE
	print : $(<) 1 :
	/*
	 * trailing C style test
	 */
test-make-1 : .MAKE .VIRTUAL .FORCE
	#
	# leading make style test
	#
	print : $(<) 4 :
test-make-2 : .MAKE .VIRTUAL .FORCE
	print : $(<) 1 :
	#
	# embedded make style test
	#
	print : $(<) 5 :
test-make-3 : .MAKE .VIRTUAL .FORCE
	print : $(<) 1 :
	#
	# trailing make style test
	#'
		OUTPUT - $'=== test-C-1 ===
"


print : $(<) 4 :"
: test-C-1 4 :
"


print : $(<) 4 :"
=== test-C-2 ===
"print : $(<) 1 :



print : $(<) 5 :"
: test-C-2 1 :
: test-C-2 5 :
"print : $(<) 1 :



print : $(<) 5 :"
=== test-C-3 ===
"print : $(<) 1 :"
: test-C-3 1 :
"print : $(<) 1 :"
=== test-make-1 ===
"


print : $(<) 4 :"
: test-make-1 4 :
"


print : $(<) 4 :"
=== test-make-2 ===
"print : $(<) 1 :



print : $(<) 5 :"
: test-make-2 1 :
: test-make-2 5 :
"print : $(<) 1 :



print : $(<) 5 :"
=== test-make-3 ===
"print : $(<) 1 :"
: test-make-3 1 :
"print : $(<) 1 :"'

TEST 02 'blank lines'

	EXEC	-n
		INPUT Makefile $'
all : .MAKE .VIRTUAL .FORCE
	local T
	for T $(...:N=test-*:H)
		print === $(T) ===
		print "$(@$(T):V)"
		make $(T)
		print "$(@$(T):V)"
	end

test-1 : .MAKE .VIRTUAL .FORCE
	print : $(<) 1 :









test-2 : .MAKE .VIRTUAL .FORCE
	print : $(<) 1 :



	print : $(<) 5 :


test-3 : .MAKE .VIRTUAL .FORCE

	print : $(<) 2 :

	print : $(<) 4 :

'
		OUTPUT - $'=== test-1 ===
"print : $(<) 1 :"
: test-1 1 :
"print : $(<) 1 :"
=== test-2 ===
"print : $(<) 1 :



print : $(<) 5 :"
: test-2 1 :
: test-2 5 :
"print : $(<) 1 :



print : $(<) 5 :"
=== test-3 ===
"
print : $(<) 2 :

print : $(<) 4 :"
: test-3 2 :
: test-3 4 :
"
print : $(<) 2 :

print : $(<) 4 :"'

TEST 03 'spliced lines'

	EXEC	-n
		INPUT Makefile $'
all : .MAKE .VIRTUAL .FORCE
	local T
	for T $(...:N=test-*:H)
		print === $(T) ===
		print "$(@$(T):V)"
		make $(T)
		print "$(@$(T):V)"
	end
test-1 : .MAKE .VIRTUAL .FORCE
	A  =  a\\
	b
	print : $(<) 3 \'$(A)\' :
test-2 : .MAKE .VIRTUAL .FORCE
	A = a \\
	b \\
	c
	print : $(<) 4 \'$(A)\' :
test-3 : .MAKE .VIRTUAL .FORCE
	A = a \\
	b \\
	c'
		OUTPUT - $'=== test-1 ===
"A  =  a\\
b
print : $(<) 3 \'$(A)\' :"
: test-1 3 \'ab\' :
"A  =  ab 

print : $(<) 3 \'$(A)\' :"
=== test-2 ===
"A = a \\
b \\
c
print : $(<) 4 \'$(A)\' :"
: test-2 4 \'a b c\' :
"A = a b c 
 

print : $(<) 4 \'$(A)\' :"
=== test-3 ===
"A = a \\
b \\
c"
"A = a b c"'

TEST 04 'duplicate actions'

	EXEC	-n .
		INPUT Makefile $'all : a
a :
	foo
a :
	foo # this is benign and the comment doesn\'t count'

	EXEC	-n .
		INPUT Makefile $'all : a
a :
	foo
a :
	bar'
		ERROR - $'make: "Makefile", line 5: warning: multiple actions for a'

TEST 05 'interesting rule names'

	EXEC	-n
		INPUT Makefile $'all : a+z a++z a-z a--z
a+z a++z a-z a--z :
	: $(<)'
		OUTPUT - $'+ : a+z
+ : a++z
+ : a-z
+ : a--z'

TEST 06 '1990 "extra end required" bug'

	EXEC	-n
		INPUT Makefile $'SYS_BSD =
SYS_53 =
if SYS_BSD == 1
	/* Berkeley-derived system: SunOS mostly */
	DIRLIB =
	SYSFLAG = -DBSD42=1
	CURSLIB = /usr/5lib/libcurses.a
	CLIB = /usr/5lib/libc.a
	error 0 "Running BSD/SunOS"
else
	CURSLIB = -lcurses
	CLIB = 
	if SYS_53 == 1
		/* AT&T System V, Release 3 (and beyond?) */
		SYSFLAG = -DSYS53=1
		error 0 "Running System V Release 3"
	else
		/* AT&T System V, before Release 3 */
		DIRLIB = dir.a
		SYSFLAG =
		error 0 "Running System V before Release 3"
	end
end'
		ERROR - $'"Running System V before Release 3"
make: Makefile: a main target must be specified'
		EXIT 1

TEST 07 'recursive blank action lines'

	EXEC	-n
		INPUT Makefile $'style = cyg
all :
	cat <<\'!\'
	$(readme.$(style):@?$$(readme.$$(style))$$("\\n\\n")??)last line
	!
":DETAILS:" : .MAKE .OPERATOR
	details.$(>:O=1) := $(@:V)
	: $(details.$(style):V:R) :
":README:" : .MAKE .OPERATOR
	readme.$(style) := $(@:V)
:DETAILS: cyg
	:README:
		line one

		line three'
		OUTPUT - $'+ cat <<\'!\'
+ line one
+ 
+ line three
+ 
+ last line
+ !'

TEST 08 'expression string operand'

	EXEC	-n
		INPUT Makefile $'V = a
P = "$(V)"
if V == "$(V)"
print "$$(V)" ok
end
if V == $(V)
print $$(V) ok
end
if V == "a"
print "a" ok
end
if V == a
print a ok
end
if V == "$(P)"
print "$$(P)" ok
end
if V == $(P)
print $$(P) ok
end
all:'
		OUTPUT - $'"$(V)" ok
"a" ok
$(P) ok'

TEST 09 'shell special command'

	EXEC	--silent
		INPUT Makefile $'all : a b
a :
	print init $(<)
	{ $(COMMAND) exec < foo ;} >/dev/null 2>&1
	print done $(<)
b :
	print init $(<)
	print done $(<)'
		OUTPUT - $'init a
init b
done b'

	EXEC	--silent COMMAND=command
		OUTPUT - $'init a'
		ERROR - $'make: *** exit code 1 making a'
		EXIT 1

TEST 10 'space indentation'

	EXEC	-n
		INPUT Makefile $'T1 = t1
	T2 = t2
   T3 = t3
all :
	echo 1 -- tab
        echo 2 -- 8 spaces
         echo 3 -- 9 spaces
        echo 4 -- 8 spaces
       echo 5 -- 7 spaces
 echo 6 -- 1 space
	echo 7 -- another tab
		echo 8 -- 1 extra tab
	echo 9 -- last tab

	echo 11 T1=$(T1)
	echo 12 T2=$(T2)
	echo 13 T3=$(T3)'
		OUTPUT - $'+ echo 1 -- tab
+ echo 2 -- 8 spaces
+ echo 3 -- 9 spaces
+ echo 4 -- 8 spaces
+ echo 5 -- 7 spaces
+ echo 6 -- 1 space
+ echo 7 -- another tab
+ 	echo 8 -- 1 extra tab
+ echo 9 -- last tab
+ 
+ echo 11 T1=t1
+ echo 12 T2=t2
+ echo 13 T3=t3'
		ERROR - $'make: "Makefile", line 3: warning: <space> indentation may be non-portable'

	EXEC	-n
		INPUT Makefile $'   all : one two three four
  one :
   : 1.1 :
 two :
  : 2.1 :
   : 2.2 :
 three :
   : 3.1 :

    : 3.3 :
four :
 : 4.1 :
   : 4.2 :
    
    : 4.4 :'
		OUTPUT - $'+ : 1.1 :
+ : 2.1 :
+ : 2.2 :
+ : 3.1 :
+ 
+ : 3.3 :
+ : 4.1 :
+ : 4.2 :
+ 
+ : 4.4 :'
		ERROR - $'make: "Makefile", line 1: warning: <space> indentation may be non-portable'
