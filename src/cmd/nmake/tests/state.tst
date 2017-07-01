# ast nmake state tests

INCLUDE cc.def

TEST 01 'state views'

	EXEC	--nojobs one code=2
		INPUT Makefile $'all : one two
one : .ALWAYS .VIRTUAL .FORCE
	cd $(<)
	MAKEPATH=.. $(MAKE) $(-) -f main.mk CODE==$(code)
two : .ALWAYS .VIRTUAL .FORCE
	$(MAKE) $(-) -f main.mk CODE==$(code)'
		INPUT cc.probe
		INPUT one/
		INPUT main.mk $'%.o : %.c
	: $(CCFLAGS) > $(<)
.COMMAND.o :
	: > $(<)
app :: a.c b.c c.c'
		INPUT a.c $'extern int b(); main() { return b() + CODE; }'
		INPUT b.c $'extern int c(); int b() { return c() + CODE; }'
		INPUT c.c $'int c() { return CODE; }'
		ERROR - $'+ cd one
+ nmake --nojobs \'--regress=message\' \'--native-pp=-1\' --noprefix-include --novirtual -f main.mk CODE==2
+ MAKEPATH=..
+ : \'-DCODE=2\'
+ 1> a.o
+ : \'-DCODE=2\'
+ 1> b.o
+ : \'-DCODE=2\'
+ 1> c.o
+ :
+ 1> app'

	EXEC	--nojobs one code=2
		ERROR - $'+ cd one
+ nmake --nojobs \'--regress=message\' \'--native-pp=-1\' --noprefix-include --novirtual -f main.mk CODE==2
+ MAKEPATH=..'

	EXEC	--nojobs one code=2

	EXEC	--nojobs two code=0
		ERROR - $'+ nmake --nojobs \'--regress=message\' \'--native-pp=-1\' --noprefix-include --novirtual -f main.mk CODE==0
+ : \'-DCODE=0\'
+ 1> a.o
+ : \'-DCODE=0\'
+ 1> b.o
+ : \'-DCODE=0\'
+ 1> c.o
+ :
+ 1> app'

	EXEC	--nojobs two code=0
		INPUT two/
		ERROR - $'+ nmake --nojobs \'--regress=message\' \'--native-pp=-1\' --noprefix-include --novirtual -f main.mk CODE==0'

	EXEC	--nojobs one code=2
		ERROR - $'+ cd one
+ nmake --nojobs \'--regress=message\' \'--native-pp=-1\' --noprefix-include --novirtual -f main.mk CODE==2
+ MAKEPATH=..
+ : \'-DCODE=2\'
+ 1> a.o
+ : \'-DCODE=2\'
+ 1> b.o
+ : \'-DCODE=2\'
+ 1> c.o
+ :
+ 1> app'

TEST 02 'state actions'

	EXEC
		INPUT Makefile $'all : (XX) (YY)
XX = xx
(XX) : Makefile
	: XX is $(;)
YY = yy
(YY) :
	: YY is $(;)'
		ERROR - $'+ : XX is xx
+ : YY is yy'

	EXEC	--
		ERROR -

	EXEC	XX=aa
		ERROR - $'+ : XX is aa'

	EXEC	--
		ERROR - $'+ : XX is xx'

	EXEC	YY=zz
		ERROR - $'+ : YY is zz'

	EXEC	--
		ERROR - $'+ : YY is yy'

TEST 03 'error status'

	EXEC	--regress=sync -NFMstatic
		INPUT Makefile $'a : b y z
	echo aha $(*)
b :
	true > $(<)
y :
	false > $(<)
z :
	true > $(<)'
		OUTPUT - $'info mam static 00000
setv INSTALLROOT ../../../..
setv PACKAGEROOT ../../../../../..
setv AR ${mam_cc_AR} ${mam_cc_AR_ARFLAGS}
setv ARFLAGS rc
setv AS as
setv ASFLAGS
setv CC cc
setv mam_cc_FLAGS
setv CCFLAGS ${-debug-symbols?1?${mam_cc_DEBUG} -D_BLD_DEBUG?${mam_cc_OPTIMIZE}?}
setv CCLDFLAGS ${-strip-symbols?1?${mam_cc_LD_STRIP}??}
setv COTEMP $$
setv CPIO cpio
setv CPIOFLAGS
setv CPP "${CC} -E"
setv F77 f77
setv HOSTCC ${CC}
setv IGNORE
setv LD ld
setv LDFLAGS
setv LEX lex
setv LEXFLAGS
setv LPR lpr
setv LPRFLAGS
setv M4FLAGS
setv NMAKE nmake
setv NMAKEFLAGS
setv PR pr
setv PRFLAGS
setv SHELL /bin/sh
setv SILENT
setv TAR tar
setv YACC yacc
setv YACCFLAGS -d
make a
make b
exec - true > b
done b virtual
make y
exec - false > y
done y virtual
make z
exec - true > z
done z virtual
exec - echo aha b y z
done a virtual'

	EXEC	--
		OUTPUT -
		ERROR - $'+ true
+ 1> b
+ false
+ 1> y
make: *** exit code 1 making y'
		EXIT 1

	EXEC
		ERROR - $'+ false
+ 1> y
make: *** exit code 1 making y'

TEST 04 'cancelled error status'

	EXEC
		INPUT Makefile $'%.z : %.x (DISABLE.%)
	$(DISABLE.$(%):?false?touch $(<)?)
z : a.z b.z c.z d.z
	touch $(<)'
		INPUT a.x
		INPUT b.x
		INPUT c.x
		INPUT d.x
		ERROR - $'+ touch a.z
+ touch b.z
+ touch c.z
+ touch d.z
+ touch z'
	
	EXEC	DISABLE.a=1 DISABLE.b=1 DISABLE.c=1 DISABLE.d=1 -k
		EXIT 1
		ERROR - $'+ false
make: *** exit code 1 making a.z
+ false
make: *** exit code 1 making b.z
+ false
make: *** exit code 1 making c.z
+ false
make: *** exit code 1 making d.z
make: *** 4 actions failed'
	
	EXEC	DISABLE.a=1 DISABLE.b=1 DISABLE.c=1 DISABLE.d=1
		ERROR - $'+ false
make: *** exit code 1 making a.z'
	
	EXEC	DISABLE.a=1 DISABLE.b=1 DISABLE.c=1 DISABLE.d=1 -k
		ERROR - $'+ false
make: *** exit code 1 making a.z
+ false
make: *** exit code 1 making b.z
+ false
make: *** exit code 1 making c.z
+ false
make: *** exit code 1 making d.z
make: *** 4 actions failed'
	
	EXEC	DISABLE.a=  DISABLE.b=  DISABLE.c=1 DISABLE.d=1
		ERROR - $'+ touch a.z
+ touch b.z
+ false
make: *** exit code 1 making c.z'
	
	EXEC	DISABLE.a=  DISABLE.b=  DISABLE.c=1 DISABLE.d=1
		ERROR - $'+ false
make: *** exit code 1 making c.z'
	
	EXEC	DISABLE.a=  DISABLE.b=  DISABLE.c=  DISABLE.d=1
		ERROR - $'+ touch c.z
+ false
make: *** exit code 1 making d.z'

	EXEC	DISABLE.a=  DISABLE.b=  DISABLE.c=  DISABLE.d=1
		ERROR - $'+ false
make: *** exit code 1 making d.z'
	
	EXEC	DISABLE.a=  DISABLE.b=  DISABLE.c=  DISABLE.d=
		EXIT 0
		ERROR - $'+ touch d.z
+ touch z'
	
	EXEC	DISABLE.a=  DISABLE.b=  DISABLE.c=  DISABLE.d=
		ERROR -

TEST 05 'virtual state'

	EXEC
		INPUT Makefile $'all : (S0)
(S0) : .MAKE (S1) (S2) (S3) (S4) (S5)
	print : $(<) : $(~:T>T=$(<)) :'
		OUTPUT - $': (S0) : (S1) (S2) (S3) (S4) (S5) :'

	EXEC
		OUTPUT -

	EXEC	S1=1
		OUTPUT - $': (S0) : (S1) :'

	EXEC	S1=1
		OUTPUT -

	EXEC	S1=1 S2=2
		OUTPUT - $': (S0) : (S2) :'

	EXEC	S1=1 S2=2
		OUTPUT -

	EXEC	S1=1
		OUTPUT - $': (S0) : (S2) :'

	EXEC	S1=1
		OUTPUT -

	EXEC	--
		OUTPUT - $': (S0) : (S1) :'

	EXEC	--
		OUTPUT -

TEST 06 'virtual state with long actions'

	EXEC	--silent
		INPUT Makefile $'all : (S)
(S) : so
	sleep 2
	echo load $(*)
so : si
	echo "s output" > $(<)'
		INPUT si $'s input'
		OUTPUT - $'load so'

	EXEC	--silent
		OUTPUT -

	DO	touch si

	EXEC	--silent
		OUTPUT - $'load so'

	EXEC	--silent
		OUTPUT -

TEST 07 'virtual targets with errors'

	EXEC	ERROR=0
		INPUT Makefile $'set keepgoing
:ALL: V0
V0 : V1 V2 V3
V1 V2 V3 : .VIRTUAL (ERROR)
	case "$(<)" in
	*$(ERROR)*)	false ;;
	*)		: "$(<)" ok ;;
	esac'
		ERROR - $'+ : V1 ok
+ : V2 ok
+ : V3 ok'

	EXEC	ERROR=0
		ERROR -

	EXEC	ERROR=1
		ERROR - $'+ false
make: *** exit code 1 making V1
+ : V2 ok
+ : V3 ok
make: *** 1 action failed'
		EXIT 1

	EXEC	ERROR=1
		ERROR - $'+ false
make: *** exit code 1 making V1
make: *** 1 action failed'

	EXEC	ERROR=2
		ERROR - $'+ : V1 ok
+ false
make: *** exit code 1 making V2
+ : V3 ok
make: *** 1 action failed'

	EXEC	ERROR=2
		ERROR - $'+ false
make: *** exit code 1 making V2
make: *** 1 action failed'

	EXEC	ERROR=3
		ERROR - $'+ : V1 ok
+ : V2 ok
+ false
make: *** exit code 1 making V3
make: *** 1 action failed'

	EXEC	ERROR=3
		ERROR - $'+ false
make: *** exit code 1 making V3
make: *** 1 action failed'

	EXEC	ERROR=0
		ERROR - $'+ : V1 ok
+ : V2 ok
+ : V3 ok'
		EXIT 0

	EXEC	ERROR=0
		ERROR -

TEST 08 'statevar targets with errors'

	EXEC	ERROR=0
		INPUT Makefile $'set keepgoing
:ALL: (S0)
(S0) : (S1) (S2) (S3)
(S1) (S2) (S3) : (ERROR)
	case "$(<)" in
	*$(ERROR)*)	false ;;
	*)		: "$(<)" ok ;;
	esac'
		ERROR - $'+ : \'(S1)\' ok
+ : \'(S2)\' ok
+ : \'(S3)\' ok'

	EXEC	ERROR=0
		ERROR -

	EXEC	ERROR=1
		ERROR - $'+ false
make: *** exit code 1 making (S1)
+ : \'(S2)\' ok
+ : \'(S3)\' ok
make: *** 1 action failed'
		EXIT 1

	EXEC	ERROR=1
		ERROR - $'+ false
make: *** exit code 1 making (S1)
make: *** 1 action failed'

	EXEC	ERROR=2
		ERROR - $'+ : \'(S1)\' ok
+ false
make: *** exit code 1 making (S2)
+ : \'(S3)\' ok
make: *** 1 action failed'

	EXEC	ERROR=2
		ERROR - $'+ false
make: *** exit code 1 making (S2)
make: *** 1 action failed'

	EXEC	ERROR=3
		ERROR - $'+ : \'(S1)\' ok
+ : \'(S2)\' ok
+ false
make: *** exit code 1 making (S3)
make: *** 1 action failed'

	EXEC	ERROR=3
		ERROR - $'+ false
make: *** exit code 1 making (S3)
make: *** 1 action failed'

	EXEC	ERROR=0
		ERROR - $'+ : \'(S1)\' ok
+ : \'(S2)\' ok
+ : \'(S3)\' ok'
		EXIT 0

	EXEC	ERROR=0
		ERROR -
