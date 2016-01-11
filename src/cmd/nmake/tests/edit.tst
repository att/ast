# ast nmake edit operator tests

INCLUDE test.def

TEST 01 ':H: sort'

	EXEC	-n -f - . 'A = c b a a' 'print -- $(A:H)'
		OUTPUT - $'a a b c'
	EXEC	-n -f - . 'A = c b a a' 'print -- $(A:H=)'
		OUTPUT - $'a a b c'
	EXEC	-n -f - . 'A = c b a a' 'print -- $(A:H<)'
		OUTPUT - $'a a b c'
	EXEC	-n -f - . 'A = c b a a' 'print -- $(A:H>)'
		OUTPUT - $'c b a a'
	EXEC	-n -f - . 'A = c b a a' 'print -- $(A:H!)'
		OUTPUT - $'a b c'
	EXEC	-n -f - . 'A = c b a a' 'print -- $(A:H!=)'
		OUTPUT - $'a b c'
	EXEC	-n -f - . 'A = c b a a' 'print -- $(A:H=F)'
		OUTPUT - $'a'
	EXEC	-n -f - . 'A = c b a a' 'print -- $(A:H=I)'
		OUTPUT - $'c b a a'
	EXEC	-n -f - . 'A = c b a a' 'print -- $(A:H=N)'
		OUTPUT - $'a a b c'
	EXEC	-n -f - . 'A = c b a a' 'print -- $(A:H=U)'
		OUTPUT - $'a b c'
	EXEC	-n -f - . 'A = c b a a' 'print -- $(A:H=V)'
		OUTPUT - $'c b a a'
	EXEC	-n -f - . 'A = c b a a' 'print -- $(A:H=NF)'
		OUTPUT - $'a'
	EXEC	-n -f - . 'A = c b a a' 'print -- $(A:H=NI)'
		OUTPUT - $'c b a a'
	EXEC	-n -f - . 'A = c b a a' 'print -- $(A:H=NU)'
		OUTPUT - $'a b c'
	EXEC	-n -f - . 'A = c b a a' 'print -- $(A:H=VF)'
		OUTPUT - $'c'
	EXEC	-n -f - . 'A = c b a a' 'print -- $(A:H=VI)'
		OUTPUT - $'a a b c'
	EXEC	-n -f - . 'A = c b a a' 'print -- $(A:H=VU)'
		OUTPUT - $'c b a'

	EXEC	-n -f - . 'A = 2 10 02 1' 'print -- $(A:H)'
		OUTPUT - $'02 1 10 2'
	EXEC	-n -f - . 'A = 2 10 02 1' 'print -- $(A:H=)'
		OUTPUT - $'1 02 2 10'
	EXEC	-n -f - . 'A = 2 10 02 1' 'print -- $(A:H<)'
		OUTPUT - $'02 1 10 2'
	EXEC	-n -f - . 'A = 2 10 02 1' 'print -- $(A:H>)'
		OUTPUT - $'2 10 1 02'
	EXEC	-n -f - . 'A = 2 10 02 1' 'print -- $(A:H!)'
		OUTPUT - $'02 1 10 2'
	EXEC	-n -f - . 'A = 2 10 02 1' 'print -- $(A:H!=)'
		OUTPUT - $'1 02 10'
	EXEC	-n -f - . 'A = 2 10 02 1' 'print -- $(A:H=F)'
		OUTPUT - $'02'
	EXEC	-n -f - . 'A = 2 10 02 1' 'print -- $(A:H=I)'
		OUTPUT - $'2 10 1 02'
	EXEC	-n -f - . 'A = 2 10 02 1' 'print -- $(A:H=N)'
		OUTPUT - $'1 02 2 10'
	EXEC	-n -f - . 'A = 2 10 02 1' 'print -- $(A:H=U)'
		OUTPUT - $'02 1 10 2'
	EXEC	-n -f - . 'A = 2 10 02 1' 'print -- $(A:H=V)'
		OUTPUT - $'10 2 02 1'
	EXEC	-n -f - . 'A = 2 10 02 1' 'print -- $(A:H=NF)'
		OUTPUT - $'1'
	EXEC	-n -f - . 'A = 2 10 02 1' 'print -- $(A:H=NI)'
		OUTPUT - $'10 2 02 1'
	EXEC	-n -f - . 'A = 2 10 02 1' 'print -- $(A:H=NU)'
		OUTPUT - $'1 02 10'
	EXEC	-n -f - . 'A = 2 10 02 1' 'print -- $(A:H=VF)'
		OUTPUT - $'10'
	EXEC	-n -f - . 'A = 2 10 02 1' 'print -- $(A:H=VI)'
		OUTPUT - $'1 02 2 10'
	EXEC	-n -f - . 'A = 2 10 02 1' 'print -- $(A:H=VU)'
		OUTPUT - $'10 2 1'

	EXEC	-n -f - . 'V = a-1.2.3 a-02.3.4 a-01.2.3' 'print -- $(V:H)'
		OUTPUT - $'a-01.2.3 a-02.3.4 a-1.2.3'
	EXEC	-n -f - . 'V = a-1.2.3 a-02.3.4 a-01.2.3' 'print -- $(V:H=)'
		OUTPUT - $'a-01.2.3 a-02.3.4 a-1.2.3'
	EXEC	-n -f - . 'V = a-1.2.3 a-02.3.4 a-01.2.3' 'print -- $(V:H<)'
		OUTPUT - $'a-01.2.3 a-02.3.4 a-1.2.3'
	EXEC	-n -f - . 'V = a-1.2.3 a-02.3.4 a-01.2.3' 'print -- $(V:H>)'
		OUTPUT - $'a-1.2.3 a-02.3.4 a-01.2.3'
	EXEC	-n -f - . 'V = a-1.2.3 a-02.3.4 a-01.2.3' 'print -- $(V:H!)'
		OUTPUT - $'a-01.2.3 a-02.3.4 a-1.2.3'
	EXEC	-n -f - . 'V = a-1.2.3 a-02.3.4 a-01.2.3' 'print -- $(V:H=F)'
		OUTPUT - $'a-01.2.3'
	EXEC	-n -f - . 'V = a-1.2.3 a-02.3.4 a-01.2.3' 'print -- $(V:H=I)'
		OUTPUT - $'a-1.2.3 a-02.3.4 a-01.2.3'
	EXEC	-n -f - . 'V = a-1.2.3 a-02.3.4 a-01.2.3' 'print -- $(V:H=N)'
		OUTPUT - $'a-01.2.3 a-02.3.4 a-1.2.3'
	EXEC	-n -f - . 'V = a-1.2.3 a-02.3.4 a-01.2.3' 'print -- $(V:H=U)'
		OUTPUT - $'a-01.2.3 a-02.3.4 a-1.2.3'
	EXEC	-n -f - . 'V = a-1.2.3 a-02.3.4 a-01.2.3' 'print -- $(V:H=V)'
		OUTPUT - $'a-02.3.4 a-1.2.3 a-01.2.3'
	EXEC	-n -f - . 'V = a-1.2.3 a-02.3.4 a-01.2.3' 'print -- $(V:H=NF)'
		OUTPUT - $'a-01.2.3'
	EXEC	-n -f - . 'V = a-1.2.3 a-02.3.4 a-01.2.3' 'print -- $(V:H=NI)'
		OUTPUT - $'a-1.2.3 a-02.3.4 a-01.2.3'
	EXEC	-n -f - . 'V = a-1.2.3 a-02.3.4 a-01.2.3' 'print -- $(V:H=NU)'
		OUTPUT - $'a-01.2.3 a-02.3.4 a-1.2.3'
	EXEC	-n -f - . 'V = a-1.2.3 a-02.3.4 a-01.2.3' 'print -- $(V:H=VF)'
		OUTPUT - $'a-02.3.4'
	EXEC	-n -f - . 'V = a-1.2.3 a-02.3.4 a-01.2.3' 'print -- $(V:H=VI)'
		OUTPUT - $'a-1.2.3 a-01.2.3 a-02.3.4'
	EXEC	-n -f - . 'V = a-1.2.3 a-02.3.4 a-01.2.3' 'print -- $(V:H=VU)'
		OUTPUT - $'a-02.3.4 a-1.2.3'

	EXEC	-n -f - . 'A = /bbb /bbb/xxx/yyy/zzz /bbb/aa /bbb /aaa /bbb /zzz' 'print -- $(A:H=P)'
		OUTPUT - $'/bbb/xxx/yyy/zzz /bbb/aa /bbb /bbb /aaa /bbb /zzz'
	EXEC	-n -f - . 'A = /bbb /bbb/xxx/yyy/zzz /bbb/aa /bbb /aaa /bbb /zzz' 'print -- $(A:H=PU)'
		OUTPUT - $'/bbb/xxx/yyy/zzz /bbb/aa /bbb /aaa /bbb /zzz'
	EXEC	-n -f - . 'A = /bbb /bbb/xxx/yyy/zzz /bbb/aa /bbb /aaa /bbb /zzz' 'print -- $(A:H=P:U)'
		OUTPUT - $'/bbb/xxx/yyy/zzz /bbb/aa /bbb /aaa /zzz'

TEST 02 ':I: intersection'

	EXEC	-n -f - . 'A = c 2 b 10 a 02 1 a' 'print -- $(A:I)'
		OUTPUT - $''
	EXEC	-n -f - . 'A = c 2 b 10 a 02 1 a' 'print -- $(A:I<)'
		OUTPUT - $''

TEST 03 ':L: glob list'

	EXEC	-n -f - . 'print -- $("L.dir":L)'
		INPUT L.dir/001
		INPUT L.dir/002
		INPUT L.dir/003
		INPUT L.dir/004
		OUTPUT - $'001 002 003 004'
	EXEC	-n -f - . 'print -- $("L.dir":L=)'
	EXEC	-n -f - . 'print -- $("L.dir":L>)'
		OUTPUT - $'004'
	EXEC	-n -f - . 'print -- $("L.dir":L<)'
		OUTPUT - $'001'
	EXEC	-n -f - . 'print -- $("L.dir":L!=)'
		OUTPUT - $'L.dir/001 L.dir/002 L.dir/003 L.dir/004'
	EXEC	-n -f - . 'print -- $("L.dir":L!<=)'
	EXEC	-n -f - . 'print -- $("L.dir":L!>=)'
		OUTPUT - $'L.dir/004 L.dir/003 L.dir/002 L.dir/001'
	EXEC	-n -f - . 'print -- $("L.dir":L!)'
		OUTPUT - $'L.dir/001'
	EXEC	-n -f - . 'print -- $("L.dir":L!<)'
	EXEC	-n -f - . 'print -- $("L.dir":L!>)'
		OUTPUT - $'L.dir/004'

TEST 04 ':N: file match'

	EXEC	-n -f - . 'A = c 2 b 10 a 02 1 a' 'print -- $(A:N)'
		OUTPUT - $''

TEST 05 ':O: ordinal'

	EXEC	-n -f - . 'A = c 2 b 10 a 02 1 a' 'print -- $(A:O)'
		OUTPUT - $'8'
	EXEC	-n -f - . 'A = c 2 b 10 a 02 1 a' 'print -- $(A:O=)'
		OUTPUT - $'8'

TEST 06 ':Q: quoting'

	EXEC	-n -f - . 'A = "a z"' 'print -- $(A:Q)'
		OUTPUT - $'\\""a z"\\"'
	EXEC	-n -f - . "A = 'a z'" 'print -- $(A:Q)'
		OUTPUT - $'"\'a z\'"'
	EXEC	-n -f - . 'A = a z' 'print -- $(A:Q)'
		OUTPUT - $'a z'
	EXEC	-n -f - . 'A = a\z' 'print -- $(A:Q)'
		OUTPUT - $'\'a\\z\''
	EXEC	-n -f - . 'A = a$z' 'print -- $(A:Q)'
		OUTPUT - $'\'a\$z\''
	EXEC	-n -f - . 'A = "a z"' 'print -- $(A:@Q)'
		OUTPUT - $'\\""a z"\\"'
	EXEC	-n -f - . "A = 'a z'" 'print -- $(A:@Q)'
		OUTPUT - $'"\'a z\'"'
	EXEC	-n -f - . 'A = a z' 'print -- $(A:@Q)'
		OUTPUT - $'\'a z\''
	EXEC	-n -f - . 'A = a\z' 'print -- $(A:@Q)'
		OUTPUT - $'\'a\\z\''
	EXEC	-n -f - . 'A = a$z' 'print -- $(A:@Q)'
		OUTPUT - $'\'a\$z\''

TEST 07 ':T=D: quoting'

	EXEC	-n -f - . 'A == "a z"' 'print -- $("(A)":T=D)'
		OUTPUT - -DA=$'\\""a z"\\"'
	EXEC	-n -f - . "A == 'a z'" 'print -- $("(A)":T=D)'
		OUTPUT - -DA=$'"\'a z\'"'
	EXEC	-n -f - . 'A == a z' 'print -- $("(A)":T=D)'
		OUTPUT - -DA=$'\'a z\''
	EXEC	-n -f - . 'A == a\z' 'print -- $("(A)":T=D)'
		OUTPUT - -DA=$'\'a\\z\''
	EXEC	-n -f - . 'A == a$z' 'print -- $("(A)":T=D)'
		OUTPUT - -DA=$'\'a\$z\''

TEST 08 ':T=E: quoting'

	EXEC	-n -f - . 'A == "a z"' 'print -- $("(A)":T=E)'
		OUTPUT - A=$'"a z"'
	EXEC	-n -f - . "A == 'a z'" 'print -- $("(A)":T=E)'
		OUTPUT - A=$'\'a z\''
	EXEC	-n -f - . 'A == a z' 'print -- $("(A)":T=E)'
	EXEC	-n -f - . 'A == a\z' 'print -- $("(A)":T=E)'
		OUTPUT - A=$'\'a\\z\''
	EXEC	-n -f - . 'A == a$z' 'print -- $("(A)":T=E)'
		OUTPUT - A=$'\'a\$z\''

TEST 09 ':C:'

	EXEC	-n
		INPUT Makefile $'A = ../../hdr
t :
	: $(X)
	: $(Y)
	: $(Z)
X : .FUNCTION
	return $(A:/^/-I)
Y : .FUNCTION
	.Y : $(A:/^/-I)
	return $(~.Y)
Z : .FUNCTION
	.Z : $(A:/^/-I)
	return $(*.Z)'
		OUTPUT - $'+ : -I../../hdr
+ : -I../../hdr
+ : -I../../hdr'

TEST 10 ':F:'

	EXEC -n
		INPUT Makefile $'LOWERCASE = GO LOWER CASE
UPPERCASE = go upper case
FILENAME = $$*/usr/people/login
COUNT = 23
STRING = "this is a string"
all :
	: obsolete lower :$(LOWERCASE:F=L):
	: lower :$(LOWERCASE:F=%(lower)s):
	: obsolete upper :$(UPPERCASE:F=U):
	: upper :$(UPPERCASE:F=%(upper)s):
	: string :$(STRING:F=%s):
	: hex :$(COUNT:F=%x):
	: dec :$(COUNT:F=%10.5d):
	: oct :$(COUNT:F=%o):
	: right justify :$("var1 var2":F=%20.10s):
	: left justify :$("var1 var2":F=%-20.10s):
	: obsolete variable :$(FILENAME:F=V):
	: variable :$(FILENAME:F=%(variable)s):'
		OUTPUT - $'+ : obsolete lower :go lower case:
+ : lower :go lower case:
+ : obsolete upper :GO UPPER CASE:
+ : upper :GO UPPER CASE:
+ : string :"this is a string":
+ : hex :17:
+ : dec :     00023:
+ : oct :27:
+ : right justify :                var1                 var2:
+ : left justify :var1                var2                :
+ : obsolete variable :....usr.people.login:
+ : variable :....usr.people.login:'

TEST 11 'empty op values'

	EXEC	-n
		INPUT Makefile $'all : .MAKE
	A = a b c
	print $(A:A)
	print $(A:I)'
		OUTPUT - $'
'

TEST 12 ':P=G:'

	EXPORT VPATH=$TWD/dev:$TWD/ofc

	CD ofc

	EXEC	-n
		INPUT Makefile $'all : .MAKE
	X = *.c
	print :P=G: $(X:P=G)
	print :P=G:T=F: $(X:P=G:T=F)'
		INPUT ofc.c
		INPUT dup.c
		OUTPUT - $':P=G: dup.c ofc.c
:P=G:T=F: dup.c ofc.c'

	CD ../dev

	EXEC	-n
		INPUT dev.c
		INPUT dup.c
		OUTPUT - $':P=G: dev.c dup.c ofc.c
:P=G:T=F: dev.c dup.c '$TWD$'/ofc/ofc.c'

TEST 13 ':P=S:'

	EXPORT VPATH=$TWD/dev:$TWD/ofc

	CD ofc

	EXEC	-ns
		INPUT Makefile $'FILES = a.h ../a.h
all : $(FILES)
../a.h : a.h
	cp $(*) $(<)
.DONE : .done
.done : .MAKE
	print :P=S: $(FILES:T=F:P=S)
	print :P!=S: $(FILES:T=F:P!=S)'
		INPUT a.h
		INPUT $TWD/a.h
		OUTPUT - $':P=S: a.h
:P!=S: ../a.h'

	CD ../dev

	EXEC	-ns
		OUTPUT - $':P=S: '$TWD$'/ofc/a.h
:P!=S: ../a.h'

TEST 14 'rebind variants'

	EXEC
		INPUT Makefile $'set nowriteobject nowritestate
all : tst
	rm -f notfound $(*.SOURCE:L=e*)
tst : .VIRTUAL .FORCE
	touch exists'
		ERROR - $'+ touch exists\n+ rm -f notfound'

	EXEC
		ERROR - $'+ touch exists\n+ rm -f notfound exists'

	EXEC
		INPUT Makefile $'set nowriteobject nowritestate
all : tst
	rm -f notfound $("rebind $(*.SOURCE)":R)$(*.SOURCE:L=e*)
tst : .VIRTUAL .FORCE
	touch exists'
		ERROR - $'+ touch exists\n+ rm -f notfound exists'

	EXEC

	EXEC
		INPUT Makefile $'set nowriteobject nowritestate
all : tst
	rm -f notfound $(*.SOURCE:T=B:L=e*)
tst : .VIRTUAL .FORCE
	touch exists'
		ERROR - $'+ touch exists\n+ rm -f notfound exists'

	EXEC

	EXEC
		INPUT Makefile $'set nowriteobject nowritestate
all : tst
	rm -f notfound $(*.SOURCE:L^=e*)
tst : .VIRTUAL .FORCE
	touch exists'
		ERROR - $'+ touch exists\n+ rm -f notfound exists'

	EXEC

TEST 15 ':-val: :+val: :!val: :???Z:'

	EXEC	-n
		INPUT Makefile $'A =
B = 0
C = 00
D = 0 0
E = 1
F = 1 1
all :
	echo :$(A:~.):$(B:~.):$(C:~.):$(D:~.):$(E:~.):$(F:~.):
	echo :$(A:-.):$(B:-.):$(C:-.):$(D:-.):$(E:-.):$(F:-.):
	echo :$(A:+.):$(B:+.):$(C:+.):$(D:+.):$(E:+.):$(F:+.):
	echo :$(A:@?Y?N?):$(B:@?Y?N?):$(C:@?Y?N?):$(D:@?Y?N?):$(E:@?Y?N?):$(F:@?Y?N?):
	echo :$(A:@?Y?N?Z):$(B:@?Y?N?Z):$(C:@?Y?N?Z):$(D:@?Y?N?Z):$(E:@?Y?N?Z):$(F:@?Y?N?Z):
	echo :$(A:@?Y?N?O):$(B:@?Y?N?O):$(C:@?Y?N?O):$(D:@?Y?N?O):$(E:@?Y?N?O):$(F:@?Y?N?O):
	echo :$(A:@?Y?N?OZ):$(B:@?Y?N?OZ):$(C:@?Y?N?OZ):$(D:@?Y?N?OZ):$(E:@?Y?N?OZ):$(F:@?Y?N?OZ):
	echo :$(A:?Y?N?):$(B:?Y?N?):$(C:?Y?N?):$(D:?Y?N?):$(E:?Y?N?):$(F:?Y?N?):
	echo :$(A:?Y?N?Z):$(B:?Y?N?Z):$(C:?Y?N?Z):$(D:?Y?N?Z):$(E:?Y?N?Z):$(F:?Y?N?Z):
	echo :$(A:?Y?N?O):$(B:?Y?N?O):$(C:?Y?N?O):$(D:?Y?N?O):$(E:?Y?N?O):$(F:?Y?N?O):
	echo :$(A:?Y?N?OZ):$(B:?Y?N?OZ):$(C:?Y?N?OZ):$(D:?Y?N?OZ):$(E:?Y?N?OZ):$(F:?Y?N?OZ):'
		OUTPUT - $'+ echo :.:.:::::
+ echo :.:.:00:0 0:1:1 1:
+ echo ::0:.:.:.:.:
+ echo :N:Y:Y:Y:Y:Y:
+ echo :N:N:Y:Y:Y:Y:
+ echo :N:0:00:0 0:1:1 1:
+ echo :N:N:00:0 0:1:1 1:
+ echo :N:Y:Y:Y Y:Y:Y Y:
+ echo :N:N:Y:N N:Y:Y Y:
+ echo :N:0:00:0 0:1:1 1:
+ echo :N:N:00:N N:1:1 1:'

TEST 16 ':T=Q*:'

	EXEC	-n
		INPUT Makefile $'X = foo_yes_nothing .foo.yes.internal foo_yes_variable (foo_yes_variable) foo_yes_rule ()foo_yes_rule foo_no_nothing .foo.no.internal foo_no_variable (foo_no_variable) foo_no_rule ()foo_no_rule
.foo.yes.internal = 1
foo_yes_variable = 2
(foo_yes_variable) = 3
foo_yes_rule : .SPECIAL
all :
	: T=XQ : $(X:T=XQ) :
	: T=XQO : $(X:T=XQO) :
	: T=XQR : $(X:T=XQR) :
	: T=XQS : $(X:T=XQS) :
	: T=XQSA : $(X:T=XQSA) :
	: T=XQSR : $(X:T=XQSR) :
	: T=XQSV : $(X:T=XQSV) :
	: T=XQVI : $(X:T=XQVI) :
	: T=XQV : $(X:T=XQV) :
	: T!=XQ : $(X:T!=XQ) :
	: T!=XQO : $(X:T!=XQO) :
	: T!=XQR : $(X:T!=XQR) :
	: T!=XQS : $(X:T!=XQS) :
	: T!=XQSA : $(X:T!=XQSA) :
	: T!=XQSR : $(X:T!=XQSR) :
	: T!=XQSV : $(X:T!=XQSV) :
	: T!=XQVI : $(X:T!=XQVI) :
	: T!=XQV : $(X:T!=XQV) :
	: T=Q : $(X:T=Q) :
	: T=QO : $(X:T=QO) :
	: T=QR : $(X:T=QR) :
	: T=QS : $(X:T=QS) :
	: T=QSA : $(X:T=QSA) :
	: T=QSR : $(X:T=QSR) :
	: T=QSV : $(X:T=QSV) :
	: T=QVI : $(X:T=QVI) :
	: T=QV : $(X:T=QV) :
	: T!=Q : $(X:T!=Q) :
	: T!=QO : $(X:T!=QO) :
	: T!=QR : $(X:T!=QR) :
	: T!=QS : $(X:T!=QS) :
	: T!=QSA : $(X:T!=QSA) :
	: T!=QSR : $(X:T!=QSR) :
	: T!=QSV : $(X:T!=QSV) :
	: T!=QVI : $(X:T!=QVI) :
	: T!=QV : $(X:T!=QV) :'
		OUTPUT - $'+ : T=XQ : (foo_yes_variable) foo_yes_rule :
+ : T=XQO :  :
+ : T=XQR : foo_yes_nothing .foo.yes.internal foo_yes_variable foo_yes_rule foo_no_nothing .foo.no.internal foo_no_variable foo_no_rule :
+ : T=XQS : (foo_yes_variable) ()foo_yes_rule (foo_no_variable) ()foo_no_rule :
+ : T=XQSA :  :
+ : T=XQSR : ()foo_yes_rule ()foo_no_rule :
+ : T=XQSV : (foo_yes_variable) (foo_no_variable) :
+ : T=XQVI :  :
+ : T=XQV : .foo.yes.internal foo_yes_variable :
+ : T!=XQ : foo_yes_nothing .foo.yes.internal foo_yes_variable ()foo_yes_rule foo_no_nothing .foo.no.internal foo_no_variable (foo_no_variable) foo_no_rule ()foo_no_rule :
+ : T!=XQO : foo_yes_nothing .foo.yes.internal foo_yes_variable (foo_yes_variable) foo_yes_rule ()foo_yes_rule foo_no_nothing .foo.no.internal foo_no_variable (foo_no_variable) foo_no_rule ()foo_no_rule :
+ : T!=XQR : (foo_yes_variable) ()foo_yes_rule (foo_no_variable) ()foo_no_rule :
+ : T!=XQS : foo_yes_nothing .foo.yes.internal foo_yes_variable foo_yes_rule foo_no_nothing .foo.no.internal foo_no_variable foo_no_rule :
+ : T!=XQSA : foo_yes_nothing .foo.yes.internal foo_yes_variable (foo_yes_variable) foo_yes_rule ()foo_yes_rule foo_no_nothing .foo.no.internal foo_no_variable (foo_no_variable) foo_no_rule ()foo_no_rule :
+ : T!=XQSR : foo_yes_nothing .foo.yes.internal foo_yes_variable (foo_yes_variable) foo_yes_rule foo_no_nothing .foo.no.internal foo_no_variable (foo_no_variable) foo_no_rule :
+ : T!=XQSV : foo_yes_nothing .foo.yes.internal foo_yes_variable foo_yes_rule ()foo_yes_rule foo_no_nothing .foo.no.internal foo_no_variable foo_no_rule ()foo_no_rule :
+ : T!=XQVI : foo_yes_nothing .foo.yes.internal foo_yes_variable (foo_yes_variable) foo_yes_rule ()foo_yes_rule foo_no_nothing .foo.no.internal foo_no_variable (foo_no_variable) foo_no_rule ()foo_no_rule :
+ : T!=XQV : foo_yes_nothing (foo_yes_variable) foo_yes_rule ()foo_yes_rule foo_no_nothing .foo.no.internal foo_no_variable (foo_no_variable) foo_no_rule ()foo_no_rule :
+ : T=Q : (foo_yes_variable) foo_yes_rule :
+ : T=QO :  :
+ : T=QR : foo_yes_rule :
+ : T=QS : (foo_yes_variable) :
+ : T=QSA :  :
+ : T=QSR :  :
+ : T=QSV : (foo_yes_variable) :
+ : T=QVI :  :
+ : T=QV : .foo.yes.internal foo_yes_variable :
+ : T!=Q : foo_yes_nothing .foo.yes.internal foo_yes_variable ()foo_yes_rule foo_no_nothing .foo.no.internal foo_no_variable (foo_no_variable) foo_no_rule ()foo_no_rule :
+ : T!=QO : foo_yes_nothing .foo.yes.internal foo_yes_variable (foo_yes_variable) foo_yes_rule ()foo_yes_rule foo_no_nothing .foo.no.internal foo_no_variable (foo_no_variable) foo_no_rule ()foo_no_rule :
+ : T!=QR : foo_yes_nothing .foo.yes.internal foo_yes_variable (foo_yes_variable) ()foo_yes_rule foo_no_nothing .foo.no.internal foo_no_variable (foo_no_variable) foo_no_rule ()foo_no_rule :
+ : T!=QS : foo_yes_nothing .foo.yes.internal foo_yes_variable foo_yes_rule ()foo_yes_rule foo_no_nothing .foo.no.internal foo_no_variable (foo_no_variable) foo_no_rule ()foo_no_rule :
+ : T!=QSA : foo_yes_nothing .foo.yes.internal foo_yes_variable (foo_yes_variable) foo_yes_rule ()foo_yes_rule foo_no_nothing .foo.no.internal foo_no_variable (foo_no_variable) foo_no_rule ()foo_no_rule :
+ : T!=QSR : foo_yes_nothing .foo.yes.internal foo_yes_variable (foo_yes_variable) foo_yes_rule ()foo_yes_rule foo_no_nothing .foo.no.internal foo_no_variable (foo_no_variable) foo_no_rule ()foo_no_rule :
+ : T!=QSV : foo_yes_nothing .foo.yes.internal foo_yes_variable foo_yes_rule ()foo_yes_rule foo_no_nothing .foo.no.internal foo_no_variable (foo_no_variable) foo_no_rule ()foo_no_rule :
+ : T!=QVI : foo_yes_nothing .foo.yes.internal foo_yes_variable (foo_yes_variable) foo_yes_rule ()foo_yes_rule foo_no_nothing .foo.no.internal foo_no_variable (foo_no_variable) foo_no_rule ()foo_no_rule :
+ : T!=QV : foo_yes_nothing (foo_yes_variable) foo_yes_rule ()foo_yes_rule foo_no_nothing .foo.no.internal foo_no_variable (foo_no_variable) foo_no_rule ()foo_no_rule :'

TEST 17 ':<op><sep><value>:'

	EXEC	-n
		INPUT Makefile $'X = ^a ~b <c >d !e =f
all :
	: $(X:N=^*)
	: $(X:N=~*)
	: $(X:N=<*)
	: $(X:N=>*)
	: $(X:N=!*)
	: $(X:N==*)
	: $(X:M=^.*)
	: $(X:M=~.*)
	: $(X:M=<.*)
	: $(X:M=>.*)
	: $(X:M=!.*)
	: $(X:M==.*)'
		OUTPUT - $'+ : ^a
+ : ~b
+ : <c
+ : >d
+ : !e
+ : =f
+ : ^a ~b <c >d !e =f
+ : ~b
+ : ^a ~b <c >d !e =f
+ : ^a ~b <c >d !e =f
+ : 
+ : =f'
		ERROR - $'make: regular expression: !.*: unary op not preceded by re'

TEST 18 ':P=D:'

	EXEC	-n
		INPUT Makefile $'.SOURCE : a z
all : a.x z.x z/z.x
	: $(*) : $(*:P=D) :'
		INPUT a/a.x a
		INPUT z/z.x z
		OUTPUT - $'+ : a/a.x z/z.x z/z.x : a z z :'
