# ast nmake attribute tests

INCLUDE cc.def

TEST 01 '.ACCEPT'

	EXEC	--silent
		INPUT Makefile $'all : target1 target2 target3
target1 : prereq1 
	echo making $(<)
	: > $(<)
target2 : prereq2 .VIRTUAL
	echo making $(<)
target3 : .VIRTUAL
	echo making $(<)
prereq1 : prereq3 .ACCEPT
	echo making $(<)
	: > $(<)
prereq2 :
	echo making $(<)
	: > $(<)
prereq3 :
	echo making $(<)
	: > $(<)'
		OUTPUT - $'making prereq3
making prereq1
making target1
making prereq2
making target2
making target3'

	EXEC	--silent
		OUTPUT -

	DO	touch prereq1 prereq2

	EXEC	--silent
		OUTPUT - $'making target1
making prereq2
making target2'

	DO	touch prereq3

	EXEC	--silent
		OUTPUT - $'making prereq3
making prereq1
making target1'

	EXEC	--silent
		OUTPUT -

TEST 02 '.ACCEPT'

	EXEC	--silent
		INPUT Makefile $'prog : prog.o
	echo "link $(*) > $(<)"
	echo link $(*) > $(<)
prog.o : .ACCEPT'
		INPUT prog.o '<!obj>'
		OUTPUT - $'link prog.o > prog'

	EXEC	--silent
		OUTPUT -

	DO	touch prog.c

	EXEC	--silent

TEST 03 '.ACCEPT'

	EXEC	--silent -f a.mk
		INPUT a.mk $'SFILES = main.c x.c y.c z.c
objs : $(SFILES:B:S=.o)
	: $(*)
	cat <<-EOD > $(<)
	$(*)
	EOD'
		INPUT main.o '<!obj>main'
		INPUT x.o '<!obj>x'
		INPUT y.o '<!obj>y'
		INPUT z.o '<!obj>z'
		OUTPUT objs 'main.o x.o y.o z.o'

	EXEC	--silent -f b.mk
		INPUT b.mk $'OFILES :COMMAND:
	cat objs
.MAIN : xeq
$(OFILES) : .ACCEPT
xeq : $(OFILES)
	echo "link $(*) > $(<)"
	echo link $(*) > $(<)'
		OUTPUT - $'link main.o x.o y.o z.o > xeq'

	EXEC	--silent -f a.mk
		OUTPUT -

	EXEC	--silent -f b.mk

TEST 04 '.PARAMETER'

	EXEC	-n
		INPUT param.G $'#define aaa 1
#define zzz 2'
		INPUT Makefile $'.ATTRIBUTE.%.G : .TERMINAL
.MAKEINIT : .init
.init : .MAKE .VIRTUAL .FORCE
	param.G : .SCAN.c .PARAMETER .SPECIAL
all : .MAKE
	local v
	print PARAMETER : $(...:A=.PARAMETER:A=.REGULAR)
	for v $(...:A=.STATEVAR:A=.SCAN:P=L:H)
		print $(v) = $($(v))
	end'
		OUTPUT - $'PARAMETER : param.G
(aaa) = 1
(zzz) = 2'

TEST 05 '.DONTCARE'

	EXEC	-n RET=1
		INPUT Makefile $'all : yes.bb no.xx
no.xx : no.aa
	cp $(*) $(<)
%.bb : %.aa
	cp $(>) $(<)
if "$(RET)"
.DONTCARE.%.aa : .FUNCTION
	return $(RET)
end'
		INPUT yes.aa
		OUTPUT - $'+ cp yes.aa yes.bb
+ cp no.aa no.xx'

	EXEC	-n
		OUTPUT - $'+ cp yes.aa yes.bb'
		ERROR - $'make: don\'t know how to make all : no.xx : no.aa'
		EXIT 1

TEST 06 'change => clash'

	EXEC	old=1
		INPUT Makefile $'.ATTR.aaa : .ATTRIBUTE
if old
.ATTR.xxx : .ATTRIBUTE
.SCAN.xxx : .SCAN
	OM
end
.ATTR.yyy : .ATTRIBUTE
.SCAN.yyy : .SCAN
	OM
.ATTR.zzz : .ATTRIBUTE
.SCAN.zzz : .SCAN
	OM
all : x y z
	: $(*)
x : .ATTR.aaa
if old
x : .ATTR.xxx .SCAN.xxx
end
y : .ATTR.aaa .ATTR.yyy .SCAN.yyy
z : .ATTR.aaa .ATTR.zzz .SCAN.zzz'
		INPUT x
		INPUT y
		INPUT z
		ERROR - $'+ : x y z'

	EXEC	-n old=0
		OUTPUT - $'+ : x y z'
		ERROR - $'make: warning: Makefile.mo: frozen command argument variable old changed
make: warning: Makefile.mo: recompiling
make: warning: Makefile.ms: .ATTR.xxx .ATTRIBUTE definition clashes with .ATTR.yyy
make: warning: Makefile.ms: .ATTR.yyy .ATTRIBUTE definition changed
make: warning: Makefile.ms: .ATTR.zzz .ATTRIBUTE definition changed
make: warning: Makefile.ms: .SCAN.xxx .SCAN definition clashes with .SCAN.yyy
make: warning: Makefile.ms: .SCAN.yyy .SCAN definition changed
make: warning: Makefile.ms: .SCAN.zzz .SCAN definition changed'

	EXEC	old=0
		OUTPUT -
		ERROR - $'+ : x y z'

	EXEC	-n old=1
		OUTPUT - $'+ : x y z'
		ERROR - $'make: warning: Makefile.mo: frozen command argument variable old changed
make: warning: Makefile.mo: recompiling
make: warning: Makefile.ms: .ATTR.yyy .ATTRIBUTE definition changed
make: warning: Makefile.ms: .ATTR.zzz .ATTRIBUTE definition changed
make: warning: Makefile.ms: .SCAN.yyy .SCAN definition changed
make: warning: Makefile.ms: .SCAN.zzz .SCAN definition changed'

	EXEC	old=1
		OUTPUT -
		ERROR - $'+ : x y z'

TEST 07 '.READ'

	EXEC	--silent
		INPUT Makefile $'all : .READ
	echo result = the result is $( echo M* )
.INIT .DONE :
	echo $(<) : $(result)'
		OUTPUT - $'.INIT :
.DONE : the result is Makefile Makefile.ml Makefile.mo'

TEST 08 '.VIRTUAL'

	EXEC
		INPUT Makefile $'all : huh/yes huh/no
huh/yes : .VIRTUAL yes.src
	cp $(>) $(<)
huh/no : .VIRTUAL no.src
	: no override necessary'
		INPUT no.src
		INPUT yes.src
		INPUT huh/
		ERROR - $'+ cp yes.src huh/yes
+ : no override necessary'

	EXEC
		ERROR -

TEST 09 '.VIRTUAL state'

	EXEC
		INPUT Makefile $'.MAIN : .FOO
.FOO : .VIRTUAL a b
	echo HIT $(>)
a  b :
	echo $(<) > $(<)'
		OUTPUT - $'HIT a b'
		ERROR - $'+ echo a
+ 1> a
+ echo b
+ 1> b
+ echo HIT a b'

	EXEC
		OUTPUT -
		ERROR -

	EXEC

TEST 10 '.PARAMETER file'

	EXEC
		INPUT Makefile $'h :: m.c h.c
h.h : .PARAMETER'
		INPUT m.c $'#include "h.h"
extern int h();
int main()
{
	return h();
}'
		INPUT h.c $'#include "h.h"
#include <stdio.h>
int h()
{
#if GERMAN
	printf("Guten Tag\\n");
#elif SPANISH
	printf("Buenos Dias\\n");
#else
	printf("hey\\n");
#endif
	return 0;
}'
		INPUT h.h $'#define ENGLISH 1'
		ERROR - $'+ cc -O -I. -c m.c
+ cc -O -I. -c h.c
+ cc -O -o h m.o h.o'

	EXEC	--regress=sync
		INPUT h.h $'#define YODA 1'
		ERROR -

	EXEC	--regress=sync
		INPUT h.h $'#define GERMAN 1'
		ERROR - $'+ cc -O -I. -c h.c
+ cc -O -o h m.o h.o'

TEST 11 'attribute propagation'

	EXEC	--regress=sync -n
		INPUT Makefile $'all : foo.bar
.SRC : .ATTRIBUTE
.SOURCE.%.SRC : src
foo : .SRC
%.bar : src/%
	cat $(>) > $(<)
.DONE : .show
.show : .MAKE
	query .SRC foo src/foo'
		INPUT src/foo stuff
		OUTPUT - $'+ cat src/foo > foo.bar'
		ERROR - $'
.SRC : [not found] attribute=0x00000020 index unbound

src/foo==foo : [recent] .SRC alias regular 

src/foo : [current] .SRC must=1 regular EXISTS

()src/foo : [recent] .SRC event=[current] force compiled state
'

TEST 12 'target status'

	EXEC	--silent
		INPUT Makefile $'all : foo
foo : bar
bar :
	false
.DONE : .report
.report : .MAKE
	print FAILED : $(...:A=.FAILED:H)'
		OUTPUT - $'FAILED : all bar foo'
		ERROR - $'make: *** exit code 1 making bar'
		EXIT 1

TEST 13 '.MEMBER'

	EXEC	--regress=sync
		INPUT Makefile $'x :: main.c -ltst
tst :LIBRARY: a.c b.c
.DONE : .done
.done : .MAKE .VIRTUAL .FORCE .REPEAT
	print : $(...:A=.MEMBER:H) :'
		INPUT a.c $'int a(){return 0;}'
		INPUT b.c $'int b(){return 0;}'
		INPUT main.c $'int main(){return 0;}'
		OUTPUT - $':  :'
		ERROR - $'+ cc -O -c main.c
+ echo \'\' -ltst
+ 1> tst.req
+ cc -O -c a.c
+ cc -O -c b.c
+ ar cr libtst.a a.o b.o
+ ignore ranlib libtst.a
+ rm -f a.o b.o
+ cc -O -o x main.o libtst.a'

	EXEC	--regress=sync
		OUTPUT - $': a.o b.o :'
		ERROR -

TEST 14 '.PARAMETER + state + views'

	DO	{ mkdir v0 v1 v2 || FATAL cannot initialize views ;}

	CD	v2

	EXEC
		INPUT Makefile $'.ATTRIBUTE.%.G : .TERMINAL
.MAKEINIT : .init
.init : .MAKE .VIRTUAL .FORCE
	param.G : .SCAN.c .PARAMETER .SPECIAL
all : .MAKE .VIRTUAL .FORCE
	local v
	print PARAMETER : $(...:A=.PARAMETER:A=.REGULAR:H)
	for v $(...:A=.STATEVAR:A=.SCAN:P=L:H)
		print $(v) = $($(v))
	end'
		INPUT param.G $'#define aaa 1
#define zzz 2'
		OUTPUT - $'PARAMETER : param.G
(aaa) = 1
(zzz) = 2'

	EXEC	--regress=sync

	CD	../v0

	EXPORT	VPATH=$TWD/v0:$TWD/v1:$TWD/v2

	EXEC
		INPUT param.G $'#define aaa NEW
#define zzz 2'
		OUTPUT - $'PARAMETER : param.G
(aaa) = NEW
(zzz) = 2'

	EXEC

TEST 15 '.AFTER .FAILURE'

	EXEC	GEN=true
		INPUT Makefile $'GEN = false
t : t.i
	touch $(<)
t.i : t.x (GEN)
	$(GEN)
.PASS.AFTER.t.i : .VIRTUAL .FORCE .AFTER
	: $(<) : $(**) : $(<<) :
	cp $(**) $(<<)
.FAIL.AFTER.t.i : .VIRTUAL .FORCE .AFTER .FAILURE
	: $(<) : $(**) : $(<<) :
	cp /dev/null $(<<)
t.i : .PASS.AFTER.t.i .FAIL.AFTER.t.i'
		INPUT t.x $'SOURCE'
		OUTPUT -
		OUTPUT t.i $'SOURCE'
		ERROR - $'+ true
+ : .PASS.AFTER.t.i : t.x : t.i :
+ cp t.x t.i
+ touch t'

	EXEC	GEN=true
		ERROR -

	EXEC	--regress=sync GEN=false
		OUTPUT t.i
		ERROR - '+ false
+ : .FAIL.AFTER.t.i : t.x : t.i :
+ cp /dev/null t.i
+ touch t'

	EXEC	GEN=false
		ERROR -

TEST 16 '.SEMAPHORE'

	EXEC
		INPUT Makefile $'
set jobs=4
all : a1 b1 c1
x : .SEMAPHORE
a1 : x
	silent sleep 1
	: $(<) :
b1 : x
	: $(<) :
c1 :
	: $(<) :'
		ERROR - $'+ : c1 :
+ : a1 :
+ : b1 :'

	EXEC
		INPUT Makefile $'
set jobs=4
all : a2 b2 c2
x : .SEMAPHORE .SEMAPHORE
a2 : x
	silent sleep 1
	: $(<) :
b2 : x
	: $(<) :
c2 : x
	: $(<) :'
		ERROR - $'+ : b2 :
+ : c2 :
+ : a2 :'

TEST 17 '.AFTER + .SEMAPHORE'

	EXEC
		INPUT Makefile $'
set jobs=2
all : a b c
x : .SEMAPHORE
fix : .AFTER .FORCE .REPEAT
	: $(<) : $(<<) :
	silent sleep 1
a b : x fix
	silent sleep 1
	: $(<) : $(<<) :
c :
	: $(<) : $(<<) :
	silent sleep 1'
		ERROR - $'+ : c : all :
+ : a : all :
+ : fix : a :
+ : b : all :
+ : fix : b :'

TEST 18 'pattern association'

	EXEC	-n
		INPUT Makefile $'.ATTRIBUTE.RECS.% : .SCAN.IGNORE
all : a.1.done a.2.done
.ATTRIBUTE.RECS.%.A.N .ATTRIBUTE.RECS.%.A.C : .DONTCARE
a.%.done : RECS.%.A.C RECS.%.A.N
	touch $(<)
a.%.done : RECS.%.A.N RECS.%.A.C
	touch $(<)'
		INPUT RECS.1.A.C
		INPUT RECS.2.A.N
		OUTPUT - $'+ touch a.1.done
+ touch a.2.done'

TEST 19 'assignment intercepts'

	EXEC	-n
		INPUT Makefile $'O1 = o1
.ASSIGN.% : ":TEST_ASSIGN:"
":TEST_ASSIGN:" : .MAKE .OPERATOR
	print <$(<:V)> <$(%:V)> <$(>:V)>
I1 = i1 $(O1)
I2 := i2 $(O1)
I3 = i3 $(O1)
I3 += aa $(O1)
I4 &= i4 $(O1)
.ASSIGN.% : .DELETE ":TEST_ASSIGN:"
N1 = n1 $(O1)
N2 := n2 $(O1)
N3 = n3 $(O1)
N3 += bb $(O1)
N4 &= n4 $(O1)
all :'
	OUTPUT - $'<I1> <=> <i1 $(O1)>
<I2> <=> <i2 o1>
<I3> <=> <i3 $(O1)>
<I3> <+=> <aa o1>
<I4> <&=> <i4 o1>'

TEST 20 'assignment intercepts'

	EXEC	-n
		INPUT Makefile $'O1 = o1
.ASSIGN.% : ":TEST_ASSIGN:"
":TEST_ASSIGN:" : .MAKE .OPERATOR
	eval
		TEST.$(<) $(%) $(>:V)
	end
I1 = i1 $(O1)
I2 := i2 $(O1)
I3 = i3 $(O1)
I3 += aa $(O1)
I4 &= i4 $(O1)
.ASSIGN.% : .DELETE ":TEST_ASSIGN:"
N1 = n1 $(O1)
N2 := n2 $(O1)
N3 = n3 $(O1)
N3 += bb $(O1)
N4 &= n4 $(O1)
all : .MAKE
	local V I P
	for V I N
		for I 1 2 3 4
			for P "" TEST.
				print :$(P)$(V)$(I):$($(P)$(V)$(I)):
			end
		end
	end'
	OUTPUT - $':I1::
:TEST.I1:i1 o1:
:I2::
:TEST.I2:i2 o1:
:I3::
:TEST.I3:i3 o1 aa o1:
:I4::
:TEST.I4:i4 o1:
:N1:n1 o1:
:TEST.N1::
:N2:n2 o1:
:TEST.N2::
:N3:n3 o1 bb o1:
:TEST.N3::
:N4:n4 o1:
:TEST.N4::'

TEST 21 'assertion intercepts'

	EXEC	-n
		INPUT Makefile $'.ASSERT..TEST% : ":TEST_ASSERT:"
":TEST_ASSERT:" : .MAKE .OPERATOR
	print <$(<)> <$(>:V)> <$(@:V)>
.NOTEST : n0
.TEST : i1
.TEST.I2 : i2
.TEST.I3 :INSTALL: i3
	a3
.ASSERT..SOURCE% : .DELETE ":TEST_ASSERT:"
.NOTEST : i1
.NOTEST.I2 : i2
.NOTEST.I3 :INSTALL: i3
all :'
	OUTPUT - $'<.TEST> <i1> <>
<.TEST.I2> <i2> <>
<.TEST.I3> <.SPECIAL .SCAN.IGNORE .DO.INSTALL.DIR..  i3> <a3>'

TEST 22 '.RETAIN'

	EXEC
		INPUT Makefile $'
retain AAA .BBB .CCC.

.set : .MAKE .VIRTUAL
	AAA = aaa
	.BBB = bbb
	.CCC. = ccc

all : .set
	: $(AAA) : $(.BBB) : $(.CCC.) :
'
		ERROR - $'+ : aaa : bbb : ccc :'

	EXEC
