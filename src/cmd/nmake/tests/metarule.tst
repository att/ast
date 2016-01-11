# ast nmake pattern metarule tests

INCLUDE cc.def

TEST 01 'multiple targets'

	EXEC	-n
		INPUT Makefile $'%.c c_%.c %.h c_%.h : %.sch .TERMINAL
	$(CP) $(>) $(>:B:S=.c)
	$(CP) $(>) c_$(>:B:S=.c)
	$(CP) $(>) $(>:B:S=.h)
	$(CP) $(>) c_$(>:B:S=.h)
tst :: main.c g.sch'
		INPUT g.sch
		INPUT main.c
		OUTPUT - $'+ cc -O   -c main.c
+ cp g.sch g.c
+ cp g.sch c_g.c
+ cp g.sch g.h
+ cp g.sch c_g.h
+ cc -O   -c g.c
+ cc -O   -c c_g.c
+ cc  -O   -o tst main.o g.o c_g.o'

TEST 02 'chain'

	EXEC
		INPUT Makefile $'% : %.z .TERMINAL
	: convert $(>) > $(<)
%.q : %
	: convert $(>) > $(<)
all : m.x.q'
		INPUT m.x.z
		ERROR - $'+ : convert m.x.z
+ 1> m.x
+ : convert m.x
+ 1> m.x.q'

	EXEC
		ERROR -

	EXEC

TEST 03 'state var prerequisite'

	EXEC	--silent
		INPUT Makefile $'FLAGS = --init
all : t.tso
%.tso : %.ts (FLAGS)
	echo : convert $(>) to $(<) : $(&:T=D) : $(&:T=E) :
	: > $(<)'
		INPUT t.ts
		OUTPUT - $': convert t.ts to t.tso : --init : FLAGS=--init :'

	EXEC	--silent
		OUTPUT -

	DO	touch t.ts

	EXEC	--silent
		OUTPUT - $': convert t.ts to t.tso : --init : FLAGS=--init :'

	EXEC	--silent
		OUTPUT -

	EXEC	--silent FLAGS=--done
		OUTPUT - $': convert t.ts to t.tso : --done : FLAGS=--done :'

	EXEC	--silent FLAGS=--done
		OUTPUT -

TEST 04 'stem exercizes'

	EXEC	-n
		INPUT Makefile $'rules
% : s.%.c
	get $(>)
	cc -O -o $(<) $(%).c
% : s.%.l .TERMINAL
	get $(>)
	lex $(%).l
	mv lex.yy.c $(<).c
	cc -O -o $(<) $(<).c -ll
	rm $(<).c
% : s.%.sh
	get $(>)
	cp $(%).sh $(<)
	chmod 0700 $(<)
all : xlate crs'
		INPUT s.xlate.l
		INPUT s.crs.sh
		OUTPUT - $'+ get s.xlate.l
+ lex xlate.l
+ mv lex.yy.c xlate.c
+ cc -O -o xlate xlate.c -ll
+ rm xlate.c
+ get s.crs.sh
+ cp crs.sh crs
+ chmod 0700 crs'

TEST 05 'optional joint prerequisites'

	EXEC	-n
		INPUT Makefile $'%.c %.h : %.g
	: order : $(<) :
	cat $(>) > $(<:O=1)
cmd :: g.g'
		INPUT g.g $'int main(){return 0;}'
		OUTPUT - $'+ : order : g.c g.h :
+ cat g.g > g.c
+ cc -O   -c g.c
+ cc  -O   -o cmd g.o'

	EXEC	--
		OUTPUT -
		ERROR - $'+ : order : g.c g.h :
+ cat g.g
+ 1> g.c
+ cc -O -c g.c
+ cc -O -o cmd g.o'

	EXEC	--
		ERROR -

	EXEC	clobber
		ERROR - $'+ ignore rm -f -r cmd g.c g.o Makefile.mo Makefile.ms'

	EXEC	-n
		INPUT Makefile $'%.c %.h : %.g
	: order : $(<) :
	cat $(>) > $(<:O=1)
	test -f $(<:O=2) || echo "extern int g();" > $(<:O=2)
cmd :: a.c g.g'
		INPUT a.c $'extern int g();
main(){return g();}'
		INPUT g.g $'int g(){return 0;}'
		OUTPUT - $'+ cc -O   -c a.c
+ : order : g.c g.h :
+ cat g.g > g.c
+ test -f g.h || echo "extern int g();" > g.h
+ cc -O   -c g.c
+ cc  -O   -o cmd a.o g.o'
		ERROR -

	EXEC	-n
		INPUT a.c $'#include "g.h"
extern int g();
main(){return g();}'
		OUTPUT - $'+ : order : g.c g.h :
+ cat g.g > g.c
+ test -f g.h || echo "extern int g();" > g.h
+ cc -O -I.  -c a.c
+ cc -O   -c g.c
+ cc  -O   -o cmd a.o g.o'

	EXEC	--
		OUTPUT -
		ERROR - $'+ : order : g.c g.h :
+ cat g.g
+ 1> g.c
+ test -f g.h
+ echo \'extern int g();\'
+ 1> g.h
+ cc -O -I. -c a.c
+ cc -O -c g.c
+ cc -O -o cmd a.o g.o'

	EXEC	--regress=sync
		ERROR -

	DO	touch g.g

	EXEC
		ERROR - $'+ : order : g.c g.h :
+ cat g.g
+ 1> g.c
+ test -f g.h
+ cc -O -c g.c
+ cc -O -o cmd a.o g.o'

	EXEC
		ERROR -

TEST 06 'another chain'

	EXEC
		INPUT Makefile $'
CHILE = cat
CHILEFLAGS =
%.c : %.ch %.sz
	$(CHILE) $(CHILEFLAGS) $(*:N=*.sz) $(*:N=*.ch) > $(<)
a :: a.ch z.ch'
		INPUT a.ch $'int a(){return 0;}'
		INPUT a.sz $'int main(){return a();}'
		INPUT z.ch $'int z(){return 0;}'
		INPUT z.sz
		ERROR - $'+ cat a.sz a.ch
+ 1> a.c
+ cc -O -c a.c
+ cat z.sz z.ch
+ 1> z.c
+ cc -O -c z.c
+ cc -O -o a a.o z.o'

	EXEC	--regress=sync
		ERROR -

	DO	touch z.sz

	EXEC
		ERROR - $'+ cat z.sz z.ch
+ 1> z.c
+ cc -O -c z.c
+ cc -O -o a a.o z.o'

	EXEC
		ERROR -

TEST 07 'explicit scope'

	EXEC	-n
		INPUT Makefile $'GEN = generatesource
GENFLAGS =
%.c %.h : %.spec
	$(GEN) $(GENFLAGS) $(GENTYPE:?-type $$(GENTYPE)??) $(>)
%gen : %.o a.o -lsomelib
	$(@.COMMAND.o)
:ALL: foogen bargen
bar.c : GENTYPE=raw'
		INPUT foo.spec
		INPUT bar.spec
		INPUT a.c
		INPUT libsomelib.a
		OUTPUT - $'+ generatesource   foo.spec
+ cc -O   -c foo.c
+ cc -O   -c a.c
+ cc  -O   -o foogen foo.o a.o libsomelib.a
+ generatesource  -type raw bar.spec
+ cc -O   -c bar.c
+ cc  -O   -o bargen bar.o a.o libsomelib.a'

TEST 08 'idl'

	EXEC	-n
		INPUT Makefile $'include "idl.mk"
cmd :: src.idl'
		INPUT idl.mk $'/* orbix */
ORBIXDIR = /software/Orbix

.SOURCE.idl : . $$(ORBIXDIR)/corba2/include
.SOURCE.h : $$(ORBIXDIR)/corba2/include

/* idl variables */
IDL = $(ORBIXDIR)/bin/idl
IDLFLAGS = -minterOp -B -S -cC.cpp -sS.cpp

/* Make ORBIX files hosted to reduce compile warnings! */
CCFLAGS += -I-H$$(ORBIXDIR)/corba2/include

/* Keep IDL command variables from being expanded */
(IDL) (IDLFLAGS) : .PARAMETER

/* Suffixes of files created by idl */
.IDL.SUFFIXES = .hh C.cpp S.cpp .ic .ih

/* IDL transformation rule */
%.C.cpp %.S.cpp %.hh %.ic %.ih : %.idl (IDL) (IDLFLAGS)
	$(IDL) $(IDLFLAGS) $(*)

/* SCAN for idl--HACK\'ed from SCAN for c (in Scanrules.mk & Makerules.mk) */
/* Do not need .PFX.INCLUDE as standard one appears to work! */
.LCL.IDL.INCLUDE : .ATTRIBUTE
.STD.IDL.INCLUDE : .ATTRIBUTE

.ACCEPT .IGNORE .RETAIN : .LCL.IDL.INCLUDE .STD.IDL.INCLUDE

.PREFIX.IDL.INCLUDE. : .FUNCTION
	$(%%) : .LCL.IDL.INCLUDE
	/*
	    Only difference from .PREFIX.INCLUDE except for removal of
	    targetcontext

	    Keep .hh files from other directories from trigging an idl compile
	    in that directory

	    Turned off for now until current scanning problem is fixed!

	if ! "$(%%:T=F:D:I=.)"
		print $(%%:D:B:S=.hh) : .TERMINAL
		$(%%:D:B:S=.hh) : .TERMINAL
	end
	*/
	if "$(%%)" != "/*" && "$(<<:P=U)" != "." && ! "$(%%:T=G)" && ( "$(<<:A:A=.SCAN)" == "$(<<<:A:A=.SCAN)" && "$(%%:P=U:D)" == ".|../*" )
		local B
		B := $(<<`;P=U;D;B=$$(%%);P=C)
		$(B) : .PFX.INCLUDE
		return $(B)
	else
		return $(%%)
	end

.SOURCE.%.LCL.IDL.INCLUDE : .FORCE $$(*.SOURCE) $$(*.SOURCE.idl)
.SOURCE.%.STD.IDL.INCLUDE : .FORCE $$(*.SOURCE.idl)

.SCAN.idl : .SCAN
	Q|/*|*/||C|
	Q|//||\\\\|LC|
	Q|"|"|\\\\|LQ|
	Q|\\\'|\\\'|\\\\|LQ|
	Q|\\\\|||CS|
	I| \\# include <%>|A.STD.IDL.INCLUDE|
	I| \\# include "%"|A.LCL.IDL.INCLUDE|$(-prefix-include:/0//:?M$$$(.PREFIX.IDL.INCLUDE.)|??)

.ATTRIBUTE.%.idl : .SCAN.idl

/* HACK\'ed from .MAKEINIT CCFLAGS setting! */
.MAKEINIT : .IDLINIT
.IDLINIT : .FORCE .MAKE .VIRTUAL
	local T3
	/* Again, remove targetcontext reference */
	T3 += $$(*:A=.SCAN.idl:@?$$$(*.SOURCE.%.LCL.IDL.INCLUDE:I=$$$$(!$$$$(*):A=.LCL.IDL.INCLUDE:P=D):/^/-I/) $$$(*.SOURCE.%.STD.IDL.INCLUDE:I=$$$$(!$$$$(*):A=.STD.IDL.INCLUDE:P=D):/^/-I/)??) $$(&:T=D)
	if T3
		IDLFLAGS &= $(T3:V)
	end

/* Softbench */
.ATTRIBUTE.%.cpp : .SCAN.c
%.o : %.cpp (CC) (CCFLAGS)
	$(CC) $(CCFLAGS) -c $(>) -o $(<)
%.C.o : %.C.cpp (CC) (CCFLAGS)
	$(CC) $(CCFLAGS) -c $(>) -o $(<)
%.S.o : %.S.cpp (CC) (CCFLAGS)
	$(CC) $(CCFLAGS) -c $(>) -o $(<)

/* Add exception handling */
CCFLAGS += +eh

/* hpux bug for long comments */
.SOURCE.h : .INSERT /opt/dce/include

/* sidni */
SID_INC_DIR = $(VROOT)/inc

.SOURCE.idl : $$(SID_INC_DIR)
.SOURCE.h : $$(SID_INC_DIR)'
		INPUT src.idl $'#include "tst/foo.idl"'
		INPUT tst/foo.idl
		OUTPUT - $'+ /software/Orbix/bin/idl -minterOp -B -S -cC.cpp -sS.cpp -I.   src.idl
+ cc -O -I-H/software/Orbix/corba2/include +eh   -c src.C.cpp -o src.C.o
+ cc -O -I-H/software/Orbix/corba2/include +eh   -c src.S.cpp -o src.S.o
+ cc  -O +eh   -o cmd src.C.o src.S.o'

TEST 09 'stem vs ..'

	EXEC	-n
		INPUT Makefile $'SUFFIX = h
../%.$(SUFFIX) : %.$(SUFFIX)
	: $(<) : $(>) :
tst : ../hdr.$(SUFFIX)'
		INPUT hdr.h
		OUTPUT - $'+ : ../hdr.h : hdr.h :'

TEST 10 'stem vs path'

	EXEC	-n
		INPUT Makefile $'../../lib/schema/%.libschema: %.c
	: $(<) : $(>) :
schema: ../../lib/schema/libUtils.libschema'
		INPUT libUtils.c
		OUTPUT - $'+ : ../../lib/schema/libUtils.libschema : libUtils.c :'

TEST 11 'sccs stem prefix'

	EXEC	-n
		INPUT Makefile $':SCCS:
hello :: hello.c'
		INPUT SCCS.mk $'":SCCS:" : .MAKE .OPERATOR
GET = get
GETFLAGS =
.SCCS.UNGET =
.SCCS.get : .FUNCTION
	local ( I O ) $(%)
	if "$(O:P=X)"
		return : $(O) ok
	end
	if ! .SCCS.UNGET
		.DONE : .SCCS.DONE
		.SCCS.DONE :
			$(RM) $(RMFLAGS) $(.SCCS.UNGET)
	end
	.SCCS.UNGET += $(O)
	return $(GET) $(GETFLAGS) $(I) > $(O)
% : .TERMINAL s.%
	$(.SCCS.get $(>) $(<))'
		INPUT s.hello.c
		OUTPUT - $'+ get  s.hello.c > hello.c
+ cc -O   -c hello.c
+ cc  -O   -o hello hello.o
+ rm -f hello.c'

TEST 12 'iffe support'

	EXEC
		INPUT Makefile $'t :: t.c
%.h : FEATURE/%
	cp $(>) $(<)'
		INPUT t.c $'#include "a.h"
int main(){return 0;}'
		INPUT features/a $'cat{
	#include "b.h"
}end'
		ERROR - $'+ iffe -v -c \'cc -O   \' -S \'\' run features/a
iffe: test: is sys/types.h a header ... yes
iffe: test: cat{ ... }end ... yes
+ iffe -v -c \'cc -O   \' -S \'\' def b
iffe: test: is sys/types.h a header ... yes
iffe: test: is b a command ... no
iffe: test: is b a library data symbol ... no
iffe: test: is b.h a header ... no
iffe: test: is b a reserved keyword ... no
iffe: test: is b a library function ... no
iffe: test: is b a math library symbol ... no
iffe: test: is sys/b.h a header ... no
iffe: test: is b a type or typedef ... no
+ cp FEATURE/b b.h
+ cp FEATURE/a a.h
+ cc -O -I. -c t.c
+ cc -O -o t t.o'

	EXEC
		ERROR -

	DO	touch features/a

	EXEC
		ERROR - $'+ iffe -v -c \'cc -O   \' -S \'\' run features/a
iffe: test: is sys/types.h a header ... yes
iffe: test: cat{ ... }end ... yes
iffe: FEATURE/a: unchanged'

TEST 13 'overrides'

	EXEC	-n
		INPUT Makefile $'%.o : %.C .CLEAR
%.c : %.C
	cp $(*) $(<) # cfront style
t : main.o
	: $(!)'
		INPUT main.C $'int main(){return 0;}'
		OUTPUT - $'+ cp main.C main.c
+ cc -O   -c main.c
+ : main.o'

TEST 14 'yacc/lex generated implicit prereqs'

	EXEC
		INPUT Makefile $'%.c %.h : %.y
	cp $(>) $(%).c
	echo "#define YACC 0" > $(%).h
%.c : %.l
	cp $(>) $(<)
cmd :: cmd.y'
		INPUT cmd.y $'#include "lex.c"'
		INPUT lex.l $'#include "cmd.h"
main() { return YACC; }'
		ERROR - $'+ cp cmd.y cmd.c
+ echo \'#define YACC 0\'
+ 1> cmd.h
+ cp lex.l lex.c
+ cc -O -I. -c cmd.c
+ cc -O -o cmd cmd.o'

TEST 15 'cobol'

	EXEC	-n
		INPUT Makefile $'CC.REQUIRE.cob = -
:PACKAGE: cobol
.SOURCE.cob : cobinc
cmd :: a.cbl i.cbl t.cob x.cbl y.cbl t.cob'
		INPUT a.cbl $'COMMENT
	EXEC SQL foo.'
		INPUT cobinc/A.cbl
		INPUT i.cbl $'COMMENT
	INVOKE foo'
		INPUT t.cob $'       IDENTIFICATION TEST.                                             12345678
      * COPY TEST                                                       12345679'
		INPUT x.cbl $'OMMENT THIS IS A COPY c.
1234 COPY a.
4567 *COPY b.
        COPY z REPLACING a == b.
1234 COPY A.'
		INPUT y.cbl $'COMMENT THIS IS A COPY c.cbl.
4567 * COPY b.cbl.
        COPY z.cbl.'
		INPUT z.cbl
		OUTPUT - $'+ cobc -static  -C -O  a.cbl
+ cc -O   -c a.c
+ cobc -static  -C -O  i.cbl
+ cc -O   -c i.c
+ cobc -static  -C -O  t.cob
+ cc -O   -c t.c
+ cobc -static  -C -O -I. -Icobinc x.cbl
+ cc -O   -c x.c
+ cobc -static  -C -O -I. y.cbl
+ cc -O   -c y.c
+ cc  -O    -o cmd a.o i.o t.o x.o y.o'

TEST 16 'patterns vs. directories'

	EXEC	-n x.O
		INPUT Makefile $'.SOURCE.I : xx
%.O : %.I
	: ">" : $(>)
	: "*" : $(*)
	: "%" : $(%)
	: "<" : $(<)'
		INPUT xx/x.I
		OUTPUT - $'+ : ">" : xx/x.I
+ : "*" : xx/x.I
+ : "%" : x
+ : "<" : x.O'

	EXEC	-n xx/x.O
		OUTPUT -
		ERROR - $'make: don\'t know how to make xx/x.O'
		EXIT 1

TEST 17 'multiple pattern prereqs'

	EXEC	-n t.x2
		INPUT Makefile $'%.x2 : %.x0 %.x1 t.xx
	: $(*) :
	cat $(*) > $(<)'
		INPUT t.x0
		INPUT t.x1
		INPUT t.xx
		OUTPUT - $'+ : t.x0 t.x1 t.xx :
+ cat t.x0 t.x1 t.xx > t.x2'

TEST 18 'metarule with a view'

	DO	{ mkdir -p top/lib && cd top/lib || FATAL cannot initialize view ;}

	EXPORT	VPATH=$TWD/top:$TWD/bot INSTALLROOT=../..

	EXEC	--regress=sync install
		INPUT $TWD/bot/lib/Makefile $'
G2DIR = /usr/add-on/g2
G2CC = $(G2DIR)/bin/g2comp
G2CCFLAGS =
(G2CC) (G2CCFLAGS) : .PARAMETER
.SOURCE.g : ../src
%.c %.h : %.g (G2CC) (G2CCFLAGS) .TERMINAL
	cp $(*) $(*:B:S=.c)
	cp $(*) $(*:B:S=.h)
G2SRC = p_e.g p_f.g
a :LIBRARY: $(G2SRC)
$(INCLUDEDIR)/g2 :INSTALLDIR: $(G2SRC:B:S=.h)
$(LIBDIR)/g2 :INSTALLDIR: $(G2SRC)'
		INPUT $TWD/bot/src/p_e.g $'int p_e() { return 0; }'
		INPUT $TWD/bot/src/p_f.g $'int p_f() { return 0; }'
		ERROR - $'+ echo \'\' -la
+ 1> a.req
+ cp '$TWD$'/bot/src/p_e.g p_e.c
+ cp '$TWD$'/bot/src/p_e.g p_e.h
+ cc -O -c p_e.c
+ cp '$TWD$'/bot/src/p_f.g p_f.c
+ cp '$TWD$'/bot/src/p_f.g p_f.h
+ cc -O -c p_f.c
+ ar cr liba.a p_e.o p_f.o
+ ignore ranlib liba.a
+ rm -f p_e.o p_f.o
+ mkdir -p ../../lib
+ ignore cp liba.a ../../lib/liba.a
+ ignore ranlib ../../lib/liba.a
+ mkdir -p ../../lib/lib
+ ignore cp a.req ../../lib/lib/a
+ mkdir -p ../../include/g2
+ ignore cp p_e.h ../../include/g2/p_e.h
+ ignore cp p_f.h ../../include/g2/p_f.h
+ mkdir -p ../../lib/g2
+ ignore cp '$TWD$'/bot/src/p_e.g ../../lib/g2/p_e.g
+ ignore cp '$TWD$'/bot/src/p_f.g ../../lib/g2/p_f.g'

	EXEC	--regress=sync install
		ERROR -

	DO	{ sleep 1; touch p_e.h; } # ccs *.[ao] 1 sec granulatity

	EXEC	--regress=sync install
		ERROR - $'+ cp '$TWD$'/bot/src/p_e.g p_e.c
+ cp '$TWD$'/bot/src/p_e.g p_e.h
+ cc -O -c p_e.c
+ ar cr liba.a p_e.o
+ ignore ranlib liba.a
+ rm -f p_e.o
+ mv -f ../../lib/liba.a ../../lib/liba.a.old
+ ignore cp liba.a ../../lib/liba.a
+ ignore ranlib ../../lib/liba.a'

TEST 19 'direct to your object'

	EXEC	-n
		INPUT Makefile $'%_a.o %.h : %.idl
	cat $(>) > $(%)_a.c
	$(CC) -c $(%)_a.c
	$(RM) $(%)_a.c
	echo "extern int $(%)();" > $(%).h
x :: x.idl main.c'
		INPUT main.c $'#include "x.h"
main() { return x(); }'
		INPUT x.idl $'int f() { return 0; }'
		OUTPUT - $'+ cat x.idl > x_a.c
+ cc -c x_a.c
+ rm x_a.c
+ echo "extern int x();" > x.h
+ cc -O -I.  -c main.c
+ cc  -O   -o x x_a.o main.o'

TEST 20 'base metarule augmentation'

	EXEC	-n
		INPUT Makeargs $'MAKEPATH=bot'
		INPUT Makefile $'%.o : %.c .DEBUG.TOP
.DEBUG.TOP : .MAKE .LOCAL
	if "$(>:N=*.c:P=L)"
		CCFLAGS := $(CC.DEBUG) $(CCFLAGS:VP:N!=-[gO]*|$(CC.DEBUG)|$\(CC.DEBUG\)|$(CC.OPTIMIZE)|$\(CC.OPTIMIZE\))
	end
cmd :: a.c b.c'
		INPUT a.c
		INPUT bot/b.c
		OUTPUT - $'+ cc -g -O   -c a.c
+ cc -O   -c bot/b.c
+ cc  -O   -o cmd a.o b.o'

TEST 21 'target equivalents'

	EXEC	-n tst.out
		INPUT Makefile $'%.out : % .NULL
% : %.src
	: generate $(<) from $(*) :
tst :
	: generate $(<) :'
		INPUT tst.out -
		INPUT aha.src -
		OUTPUT - $'+ : generate tst :'

	EXEC	-n tst

	EXEC	-n aha
		OUTPUT - $'+ : generate aha from aha.src :'

	EXEC	-n aha.out

	EXEC	-n -Q0x08000000 aha

	EXEC	-n -Q0x08000000 aha.out
		OUTPUT -
		ERROR - $'make: don\'t know how to make aha.out'
		EXIT 1

TEST 22 'yacc/lex .AFTER [.FAILURE] fallbacks with semaphores'

	EXEC	DISABLE_YY= DISABLE_LL=
		INPUT Makefile $'.YY.SEMAPHORE .LL.SEMAPHORE : .SEMAPHORE
%.c %.h : %.yy (DISABLE_YY) .YY.SEMAPHORE
	$(DISABLE_YY)
	cp $(>) $(%).c
	cp $(>) $(%).h
%.c : %.ll (DISABLE_LL) .LL.SEMAPHORE
	$(DISABLE_LL)
	cp $(>) $(<)
cmd :: yy.yy ll.ll
yy.c : .PASS.AFTER.yy.c .FAIL.AFTER.yy.c
.PASS.AFTER.yy.c : .VIRTUAL .FORCE .AFTER
	: generate $(<<:N=*.c) fallback :
	cp $(<<:N=*.c) $(<<:N=*.c:B:S=.yy.c)
	: generate $(<<:N=*.h) fallback :
	cp $(<<:N=*.h) $(<<:N=*.h:B:S=.yy.h)
.FAIL.AFTER.yy.c : .VIRTUAL .FORCE .AFTER .FAILURE
	: falling back to previous $(<<:N=*.c) :
	cp $(<<:N=*.c:B:S=.yy.c) $(<<:N=*.c)
	: falling back to previous $(<<:N=*.h) :
	cp $(<<:N=*.h:B:S=.yy.h) $(<<:N=*.h)
ll.c : .PASS.AFTER.ll.c .FAIL.AFTER.ll.c
.PASS.AFTER.ll.c : .VIRTUAL .FORCE .AFTER
	: generate $(<<) fallback :
	cp $(<<) $(<<:B:S=.ll.c)
.FAIL.AFTER.ll.c : .VIRTUAL .FORCE .AFTER .FAILURE
	: falling back to previous $(<<) :
	cp $(<<:B:S=.ll.c) $(<<)'
		INPUT yy.yy $'int main(){return 0;}'
		INPUT ll.ll $'int ll;'
		ERROR - $'+ cp yy.yy yy.c
+ cp yy.yy yy.h
+ : generate yy.c fallback :
+ cp yy.c yy.yy.c
+ : generate yy.h fallback :
+ cp yy.h yy.yy.h
+ cc -O -c yy.c
+ cp ll.ll ll.c
+ : generate ll.c fallback :
+ cp ll.c ll.ll.c
+ cc -O -c ll.c
+ cc -O -o cmd yy.o ll.o'

	EXEC	--regress=sync DISABLE_YY= DISABLE_LL=
		ERROR -

	EXEC	DISABLE_YY=false DISABLE_LL=
		ERROR - $'+ false
+ : falling back to previous yy.c :
+ cp yy.yy.c yy.c
+ : falling back to previous yy.h :
+ cp yy.yy.h yy.h
+ cc -O -c yy.c
+ cc -O -o cmd yy.o ll.o'

	EXEC	--regress=sync DISABLE_YY=false DISABLE_LL=
		ERROR -

	EXEC	DISABLE_YY= DISABLE_LL=false
		ERROR - $'+ cp yy.yy yy.c
+ cp yy.yy yy.h
+ : generate yy.c fallback :
+ cp yy.c yy.yy.c
+ : generate yy.h fallback :
+ cp yy.h yy.yy.h
+ cc -O -c yy.c
+ false
+ : falling back to previous ll.c :
+ cp ll.ll.c ll.c
+ cc -O -c ll.c
+ cc -O -o cmd yy.o ll.o'

	EXEC	--regress=sync DISABLE_YY= DISABLE_LL=false
		ERROR -

	EXEC	DISABLE_YY=false DISABLE_LL=false
		ERROR - $'+ false
+ : falling back to previous yy.c :
+ cp yy.yy.c yy.c
+ : falling back to previous yy.h :
+ cp yy.yy.h yy.h
+ cc -O -c yy.c
+ cc -O -o cmd yy.o ll.o'

	EXEC	--regress=sync DISABLE_YY=false DISABLE_LL=false
		ERROR -

	EXEC	DISABLE_YY= DISABLE_LL=
		ERROR - $'+ cp yy.yy yy.c
+ cp yy.yy yy.h
+ : generate yy.c fallback :
+ cp yy.c yy.yy.c
+ : generate yy.h fallback :
+ cp yy.h yy.yy.h
+ cc -O -c yy.c
+ cp ll.ll ll.c
+ : generate ll.c fallback :
+ cp ll.c ll.ll.c
+ cc -O -c ll.c
+ cc -O -o cmd yy.o ll.o'

	EXEC	DISABLE_YY= DISABLE_LL=
		ERROR -

TEST 23 'assertion order sensitivity'

	EXEC	-n
		INPUT Makefile $'a% : b%
	: generate $(*) > $(<)
b% : c%
	: generate $(*) > $(<)
b%-x : c%
	: generate $(*) > $(<)

all : at at-x'
		INPUT ct
		OUTPUT - $'+ : generate ct > bt
+ : generate bt > at
+ : generate ct > bt-x
+ : generate bt-x > at-x'

	EXEC	-n
		INPUT Makefile $'b%-x : c%
	: generate $(*) > $(<)
b% : c%
	: generate $(*) > $(<)
a% : b%
	: generate $(*) > $(<)

all : at at-x'
