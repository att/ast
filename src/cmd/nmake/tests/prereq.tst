# ast nmake prerequisite tests

INCLUDE cc.def

TEST 01 'prereq alias'

	EXEC	--silent
		INPUT Makefile $'all : ./t.o t.o'
		INPUT t.c $'main(){return 0;}'

	EXEC	--silent

	EXEC	--silent -f t.mk
		INPUT t.mk $'all : ../$(PWD:B:S)/t.o t.o'
		ERROR - $'make: don\'t know how to make all : t.o'
		EXIT 1

	EXEC	--silent -f t.mk

TEST 02 'direct dependency'

	EXEC	--silent
		INPUT Makefile $'a.h : a.c
	: $(!)
	touch $(<)'
		INPUT a.c $'#include "a.h"\nmain(){}'
		INPUT a.h

	EXEC	--silent

TEST 03 'circular dependency'

	EXEC	--mam=regress
		INPUT Makefile $'.SOURCE.G : uhdr
%.h : %.G
	cp $(*) $(<)
.INIT : myinit
myinit : .FORCE .MAKE .ALWAYS .SPECIAL
	BUILD_INTERFACES:=$(*.SOURCE.G:L<*.G)
.MAIN : mymain
mymain : .FORCE .MAKE .ALWAYS
	$(BUILD_INTERFACES) : .SCAN.c
	.MAKE : $(BUILD_INTERFACES:B:S=.h)'
		INPUT uhdr/bar.G $'#include "foo.h"'
		INPUT uhdr/foo.G $'#include "bar.h"'
		OUTPUT - $'info mam regress 00000
info start regression
make Makefile
bind Makefile 0.3
make makerules.mo
bind makerules.mo 0.3
done makerules.mo
done Makefile
make Makefile.mo
bind Makefile.mo 0.4
exec : compile into make object
done Makefile.mo
make .OPTION.COMPATIBILITY
bind .OPTION.COMPATIBILITY 0.0
done .OPTION.COMPATIBILITY virtual
make .PROBE.INIT
bind .PROBE.INIT 0.0
make .OPTION.COMPATIBILITY
bind .OPTION.COMPATIBILITY 0.4
done .OPTION.COMPATIBILITY virtual
make cc.probe
bind cc.probe 0.3
done cc.probe dontcare
done .PROBE.INIT virtual
setv INSTALLROOT $HOME
bind (IFFEFLAGS) -v -c \'cc -O \'
make .FLAGSINIT
bind .FLAGSINIT 0.0
done .FLAGSINIT virtual
make .MAM.INIT
bind .MAM.INIT 0.0
make .FIND.
bind .FIND. 0.0
done .FIND. virtual
setv PACKAGEROOT $HOME
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
make .FIND.
bind .FIND. 0.4
done .FIND. virtual
make .CATALOG.NAME.
bind .CATALOG.NAME. 0.0
done .CATALOG.NAME. virtual
make .CATALOG.NAME.
bind .CATALOG.NAME. 0.4
done .CATALOG.NAME. virtual
bind (USAGE_LICENSE) "[--catalog?prereq]"
make (USAGE_LICENSE) state
bind "[--catalog?prereq]" 0.4 (USAGE_LICENSE)
done (USAGE_LICENSE)
done .MAM.INIT virtual
make myinit
bind myinit 0.0
done myinit generated
make mymain
bind mymain 0.0
make bar.h
bind bar.h 0.0
meta bar.h %.G>%.h uhdr/bar.G bar
make uhdr/bar.G
bind bar.G 0.3 uhdr/bar.G
make .SCAN.c
bind .SCAN.c 0.0
done .SCAN.c virtual
make .PREFIX.INCLUDE.
bind .PREFIX.INCLUDE. 0.0
done .PREFIX.INCLUDE. virtual
make foo.h implicit
bind foo.h 0.0
meta foo.h %.G>%.h uhdr/foo.G foo
make uhdr/foo.G
bind foo.G 0.3 uhdr/foo.G
done uhdr/foo.G
init foo.h 0.4
exec - cp uhdr/foo.G foo.h
code - 0 0.4 0.4
make .PREFIX.INCLUDE.
bind .PREFIX.INCLUDE. 0.4
done .PREFIX.INCLUDE. virtual
info warning "mymain", line 2: warning: mymain : bar.h : uhdr/bar.G : foo.h : bar.h: implicit reference before action completed
make bar.h implicit
bind bar.h 0.4
done bar.h virtual
done foo.h generated
done uhdr/bar.G
init bar.h 0.4
exec - cp uhdr/bar.G bar.h
code - 0 0.4 0.4
make .PREFIX.INCLUDE.
bind .PREFIX.INCLUDE. 0.4
done .PREFIX.INCLUDE. virtual
make foo.h implicit
bind foo.h 0.4
done foo.h generated
done bar.h generated
done mymain generated
make Makefile.ms
bind Makefile.ms 0.4
exec : compile into make object
done Makefile.ms
info finish regression'

	EXEC	-n --mam=regress
		OUTPUT - $'info mam regress 00000
info start regression
make Makefile
bind Makefile 0.3
make makerules.mo
bind makerules.mo 0.3
done makerules.mo
done Makefile
make .OPTION.COMPATIBILITY
bind .OPTION.COMPATIBILITY 0.3
done .OPTION.COMPATIBILITY virtual
make .PROBE.INIT
bind .PROBE.INIT 0.3
make .OPTION.COMPATIBILITY
bind .OPTION.COMPATIBILITY 0.4
done .OPTION.COMPATIBILITY virtual
make cc.probe
bind cc.probe 0.3
done cc.probe dontcare
done .PROBE.INIT virtual
setv INSTALLROOT $HOME
bind (IFFEFLAGS) -v -c \'cc -O \'
make .FLAGSINIT
bind .FLAGSINIT 0.0
done .FLAGSINIT virtual
make .MAM.INIT
bind .MAM.INIT 0.3
make .FIND.
bind .FIND. 0.3
done .FIND. virtual
setv PACKAGEROOT $HOME
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
make .FIND.
bind .FIND. 0.4
done .FIND. virtual
make .CATALOG.NAME.
bind .CATALOG.NAME. 0.3
done .CATALOG.NAME. virtual
make .CATALOG.NAME.
bind .CATALOG.NAME. 0.4
done .CATALOG.NAME. virtual
bind (USAGE_LICENSE) "[--catalog?prereq]"
make (USAGE_LICENSE) state
bind "[--catalog?prereq]" 0.3 (USAGE_LICENSE)
done (USAGE_LICENSE)
done .MAM.INIT virtual
make myinit
bind myinit 0.0
done myinit generated
make mymain
bind mymain 0.0
make bar.h
bind bar.h 0.3
meta bar.h %.G>%.h uhdr/bar.G bar
make uhdr/bar.G
bind bar.G 0.3 uhdr/bar.G
make .SCAN.c
bind .SCAN.c 0.3
done .SCAN.c virtual
make foo.h implicit
bind foo.h 0.3
meta foo.h %.G>%.h uhdr/foo.G foo
make uhdr/foo.G
bind foo.G 0.3 uhdr/foo.G
done uhdr/foo.G
make bar.h implicit
bind bar.h 0.3
done bar.h generated
done foo.h generated
done uhdr/bar.G
make foo.h implicit
bind foo.h 0.3
done foo.h generated
done bar.h generated
done mymain generated
info finish regression'

	EXEC	--mam=regress
		OUTPUT - $'info mam regress 00000
info start regression
make Makefile
bind Makefile 0.3
make makerules.mo
bind makerules.mo 0.3
done makerules.mo
done Makefile
make .OPTION.COMPATIBILITY
bind .OPTION.COMPATIBILITY 0.3
done .OPTION.COMPATIBILITY virtual
make .PROBE.INIT
bind .PROBE.INIT 0.3
make .OPTION.COMPATIBILITY
bind .OPTION.COMPATIBILITY 0.4
done .OPTION.COMPATIBILITY virtual
make cc.probe
bind cc.probe 0.3
done cc.probe dontcare
done .PROBE.INIT virtual
setv INSTALLROOT $HOME
bind (IFFEFLAGS) -v -c \'cc -O \'
make .FLAGSINIT
bind .FLAGSINIT 0.0
done .FLAGSINIT virtual
make .MAM.INIT
bind .MAM.INIT 0.3
make .FIND.
bind .FIND. 0.3
done .FIND. virtual
setv PACKAGEROOT $HOME
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
make .FIND.
bind .FIND. 0.4
done .FIND. virtual
make .CATALOG.NAME.
bind .CATALOG.NAME. 0.3
done .CATALOG.NAME. virtual
make .CATALOG.NAME.
bind .CATALOG.NAME. 0.4
done .CATALOG.NAME. virtual
bind (USAGE_LICENSE) "[--catalog?prereq]"
make (USAGE_LICENSE) state
bind "[--catalog?prereq]" 0.3 (USAGE_LICENSE)
done (USAGE_LICENSE)
done .MAM.INIT virtual
make myinit
bind myinit 0.0
done myinit generated
make mymain
bind mymain 0.0
make bar.h
bind bar.h 0.3
meta bar.h %.G>%.h uhdr/bar.G bar
make uhdr/bar.G
bind bar.G 0.3 uhdr/bar.G
make .SCAN.c
bind .SCAN.c 0.3
done .SCAN.c virtual
make foo.h implicit
bind foo.h 0.3
meta foo.h %.G>%.h uhdr/foo.G foo
make uhdr/foo.G
bind foo.G 0.3 uhdr/foo.G
done uhdr/foo.G
make bar.h implicit
bind bar.h 0.3
done bar.h generated
done foo.h generated
done uhdr/bar.G
make foo.h implicit
bind foo.h 0.3
done foo.h generated
done bar.h generated
done mymain generated
make Makefile.ms
bind Makefile.ms 0.4
exec : compile into make object
done Makefile.ms
info finish regression'

TEST 04 ': scope'

	EXEC	-n
		INPUT Makefile $'z.o : -- --nooption'
		ERROR - $'make: don\'t know how to make z.o : --nooption'
		EXIT 1

	EXEC	-n
		INPUT Makefile $'z.o : -- variable=1'
		ERROR - $'make: don\'t know how to make z.o'

	EXEC	-n
		INPUT Makefile $'USE == 1
CCFLAGS = -x
x.o : CCFLAGS+=-g .IMPLICIT'
		ERROR - $'make: can\'t find source for x.o'

	EXEC	-n
		INPUT Makefile $'USE == 1
CCFLAGS = -x
x.o : CCFLAGS+=-g'
		ERROR - $'make: don\'t know how to make x.o'

	EXEC	-n
		INPUT x.c 'int use = USE;'
		OUTPUT - $'+ cc -x -g  -DUSE -c x.c'
		ERROR -
		EXIT 0

	EXEC	-n
		INPUT Makefile $'CCFLAGS = -O
a :: a.c b.c c.c d.c
a.o : CCFLAGS+=-a
b.o : CCFLAGS+=-b
c.o : CCFLAGS+=-a CCFLAGS+=-b
d.o : CCFLAGS=-a CCFLAGS+=-b'
		INPUT a.c
		INPUT b.c
		INPUT c.c
		INPUT d.c
		OUTPUT - $'+ cc -O -a   -c a.c
+ cc -O -b   -c b.c
+ cc -O -a -b   -c c.c
+ cc -a -b   -c d.c
+ cc  -O   -o a a.o b.o c.o d.o'

	EXEC	-n --compare tst=123
		INPUT Makefile $'all : scoped notscoped
scoped notscoped :
	: $(<) --compare=$(-compare) tst=$(tst)
scoped : --nocompare tst=789'
		OUTPUT - $'+ : scoped --compare= tst=789
+ : notscoped --compare=1 tst=123'

TEST 05 'serial vs concurrent'

	EXEC	--nojobs
		INPUT Makefile $'
.unpack : .FORCE .IGNORE
	: $(<) ...
	sleep 2
	: $(<) done
all : target1 target2 target3
target1 : .unpack file1
	sleep 3
	touch $(<)
target2 : .unpack file2
	sleep 1
	touch $(<)
target3 : target1 target2
	touch $(<)'
		INPUT file1
		INPUT file2
		ERROR - $'+ : .unpack ...
+ sleep 2
+ : .unpack done
+ sleep 3
+ touch target1
+ sleep 1
+ touch target2
+ touch target3'

	EXEC	--jobs=4 --force
		INPUT Makefile $'
.unpack : .FORCE .IGNORE
	: $(<) ...
	silent sleep 2
	: $(<) done
all : target1 target2 target3
target1 : .unpack file1
	silent sleep 3
	touch $(<)
target2 : .unpack file2
	silent sleep 1
	touch $(<)
target3 : target1 target2
	touch $(<)'
		ERROR - $'+ : .unpack ...
+ : .unpack done
+ touch target2
+ touch target1
+ touch target3'

TEST 06 ':: scope'

	EXEC	-n
		INPUT Makefile $'t :: t.c CCFLAGS=-g'
		INPUT t.c
		OUTPUT - $'+ cc -g   -c t.c
+ cc  -g   -o t t.o'

TEST 07 'missing prereqs'

	EXEC	-n
		INPUT Makefile $'CC.PROBE = -
:ALL: t1 t2
getDep : .MAKE .AFTER .ALWAYS .REPEAT .FORCE
	Targets += $(<<)
	print getDep Targets = $(Targets)
t1 : t1.c getDep
	: Making $(<)
t2 : t2.c getDep 
	: Making $(<)
.DONE : .MAKE $(HIT)
	print .DONE Targets = $(Targets)'
		INPUT t1.c
		OUTPUT - $'+ : Making t1
getDep Targets = t1
.DONE Targets = t1'
		ERROR - $'make: don\'t know how to make .ALL : t2 : t2.c'
		EXIT 1

	EXEC	-n
		INPUT t2.c
		OUTPUT - $'+ : Making t1
getDep Targets = t1
+ : Making t2
getDep Targets = t1 t2
.DONE Targets = t1 t2'
		ERROR -
		EXIT 0

TEST 08 'prereq expansion'

	EXEC	-n
		INPUT Makefile $'t : t.c (MYVAR1) (MYVAR2)
	: ">" : "$(>)" :
	: "~" : "$(~)" :
	: aha : "$(~:T=S:T>T)" :'
		INPUT t.c
		OUTPUT - $'+ : ">" : "t.c" :
+ : "~" : "t.c (MYVAR1) (MYVAR2)" :
+ : aha : "(MYVAR1) (MYVAR2)" :'

TEST 09 'prereq error'

	EXEC
		INPUT Makefile $'target : file1 file2 file3 file4
	: COMPILE $(>)
	grep error $(*) >/dev/null && exit 1
	sleep 1
	touch $(<)'
		INPUT file1 $'ok'
		INPUT file2 $'ok'
		INPUT file3 $'ok'
		INPUT file4 $'ok'
		ERROR - $'+ : COMPILE file1 file2 file3 file4
+ grep error file1 file2 file3 file4
+ 1> /dev/null
+ sleep 1
+ touch target'

	EXEC
		ERROR -


	EXEC	--regress=sync
		INPUT file2 $'error'	# simulate compilation error
		INPUT file3 $'good'	# simulate good patch
		ERROR - $'+ : COMPILE file2 file3
+ grep error file1 file2 file3 file4
+ 1> /dev/null
+ exit 1
make: *** exit code 1 making target'
		EXIT 1

	EXEC	--regress=sync
		INPUT file2 $'better'	# simulate good repatch
		ERROR - $'+ : COMPILE file2 file3
+ grep error file1 file2 file3 file4
+ 1> /dev/null
+ sleep 1
+ touch target'
		EXIT 0


	EXEC
		ERROR -

TEST 10 'all prereqs'

	EXEC	-n install
		INPUT Nmakefile $'INSTALLROOT = .
bar :: bar.c
:INSTALL:
	: $(~bar:Q) :'
		INPUT bar.c
		OUTPUT - $'+ cc -O   -c bar.c
+ cc  -O   -o bar bar.o
+ if	silent test ! -d bin
+ then	mkdir -p bin 		    		   
+ fi
+ : bar.o .COMMAND.o \'(CC.SHARED.LIBS.bar)\' \'(CCLD)\' \'(CCLDFLAGS)\' :'

TEST 11 ':LIBRARY: with version variants'

	EXEC	-n install VERSION=-
		INPUT Makefile $'INSTALLROOT = .
CCFLAGS = $(CC.DLL)
foo $(VERSION) :LIBRARY: foo.c'
		INPUT foo.c
		OUTPUT - $'+ echo "" -lfoo > foo.req
+ cc -D_BLD_DLL -D_BLD_PIC   -c foo.c
+ ar  cr libfoo.a foo.o
+ ignore ranlib libfoo.a
+ rm -f foo.o
+ cc  -shared  -o libfoo.so -all libfoo.a -notall 
+ if	silent test ! -d lib
+ then	mkdir -p lib 		    		   
+ fi
+ if	silent test \'\' != "libfoo.a"
+ then	if	silent test \'\' != ""
+ 	then	: lib/libfoo.a linked to libfoo.a
+ 	elif	silent test -d "libfoo.a"
+ 	then	cp -pr libfoo.a lib
+ 	else	silent cmp -s libfoo.a lib/libfoo.a ||
+ 		{
+ 		if	silent test -f "lib/libfoo.a"
+ 		then	{ mv -f lib/libfoo.a lib/libfoo.a.old || { test -f lib/libfoo.a && ignore rm -f lib/libfoo.a.old* && mv -f lib/libfoo.a `echo lib/libfoo.a.old* | sed -e \'s/.* //\' -e \'s/old\\(z*\\)$/old\\1z/\' -e \'s/\\*$//\'`; }; }
+ 		fi
+ 		ignore cp libfoo.a lib/libfoo.a  		    		   
+ 		}
+ 	fi
+ fi
+ ignore ranlib lib/libfoo.a
+ if	silent test ! -d lib/lib
+ then	mkdir -p lib/lib 		    		   
+ fi
+ if	silent test \'\' != "foo.req"
+ then	if	silent test \'\' != ""
+ 	then	: lib/lib/foo linked to foo.req
+ 	elif	silent test -d "foo.req"
+ 	then	cp -pr foo.req lib/lib
+ 	else	silent cmp -s foo.req lib/lib/foo ||
+ 		{
+ 		if	silent test -f "lib/lib/foo"
+ 		then	{ mv -f lib/lib/foo lib/lib/foo.old || { test -f lib/lib/foo && ignore rm -f lib/lib/foo.old* && mv -f lib/lib/foo `echo lib/lib/foo.old* | sed -e \'s/.* //\' -e \'s/old\\(z*\\)$/old\\1z/\' -e \'s/\\*$//\'`; }; }
+ 		fi
+ 		ignore cp foo.req lib/lib/foo  		    		   
+ 		}
+ 	fi
+ fi
+ if	silent test -f lib/libfoo.so
+ then	/bin/rm -f lib/libfoo.so
+ fi
+ if	silent test -f lib/libfoo.so
+ then	/bin/mv lib/libfoo.so lib/libfoo.so
+ fi
+ /bin/cp libfoo.so lib/libfoo.so
+ if	silent test "lib/libfoo.so" != "lib/libfoo.so"
+ then	if	silent test -f lib/libfoo.so
+ 	then	/bin/rm -f lib/libfoo.so
+ 	fi
+ 	/bin/ln lib/libfoo.so lib/libfoo.so
+ fi
+ chmod -w lib/libfoo.so'

	EXEC	-n install VERSION=
		OUTPUT - $'+ echo "" -lfoo > foo.req
+ cc -D_BLD_DLL -D_BLD_PIC   -c foo.c
+ ar  cr libfoo.a foo.o
+ ignore ranlib libfoo.a
+ rm -f foo.o
+ cc  -shared  -o libfoo.so.1.0 -all libfoo.a -notall 
+ if	silent test ! -d lib
+ then	mkdir -p lib 		    		   
+ fi
+ if	silent test \'\' != "libfoo.a"
+ then	if	silent test \'\' != ""
+ 	then	: lib/libfoo.a linked to libfoo.a
+ 	elif	silent test -d "libfoo.a"
+ 	then	cp -pr libfoo.a lib
+ 	else	silent cmp -s libfoo.a lib/libfoo.a ||
+ 		{
+ 		if	silent test -f "lib/libfoo.a"
+ 		then	{ mv -f lib/libfoo.a lib/libfoo.a.old || { test -f lib/libfoo.a && ignore rm -f lib/libfoo.a.old* && mv -f lib/libfoo.a `echo lib/libfoo.a.old* | sed -e \'s/.* //\' -e \'s/old\\(z*\\)$/old\\1z/\' -e \'s/\\*$//\'`; }; }
+ 		fi
+ 		ignore cp libfoo.a lib/libfoo.a  		    		   
+ 		}
+ 	fi
+ fi
+ ignore ranlib lib/libfoo.a
+ if	silent test ! -d lib/lib
+ then	mkdir -p lib/lib 		    		   
+ fi
+ if	silent test \'\' != "foo.req"
+ then	if	silent test \'\' != ""
+ 	then	: lib/lib/foo linked to foo.req
+ 	elif	silent test -d "foo.req"
+ 	then	cp -pr foo.req lib/lib
+ 	else	silent cmp -s foo.req lib/lib/foo ||
+ 		{
+ 		if	silent test -f "lib/lib/foo"
+ 		then	{ mv -f lib/lib/foo lib/lib/foo.old || { test -f lib/lib/foo && ignore rm -f lib/lib/foo.old* && mv -f lib/lib/foo `echo lib/lib/foo.old* | sed -e \'s/.* //\' -e \'s/old\\(z*\\)$/old\\1z/\' -e \'s/\\*$//\'`; }; }
+ 		fi
+ 		ignore cp foo.req lib/lib/foo  		    		   
+ 		}
+ 	fi
+ fi
+ if	silent test -f lib/libfoo.oo.1.0
+ then	/bin/rm -f lib/libfoo.oo.1.0
+ fi
+ if	silent test -f lib/libfoo.so.1.0
+ then	/bin/mv lib/libfoo.so.1.0 lib/libfoo.oo.1.0
+ fi
+ /bin/cp libfoo.so.1.0 lib/libfoo.so.1.0
+ if	silent test "lib/libfoo.so.1.0" != "lib/libfoo.so"
+ then	if	silent test -f lib/libfoo.so
+ 	then	/bin/rm -f lib/libfoo.so
+ 	fi
+ 	/bin/ln lib/libfoo.so.1.0 lib/libfoo.so
+ fi
+ chmod -w lib/libfoo.so.1.0'

	EXEC	-n install VERSION=3.4.5
		OUTPUT - $'+ echo "" -lfoo > foo.req
+ cc -D_BLD_DLL -D_BLD_PIC   -c foo.c
+ ar  cr libfoo.a foo.o
+ ignore ranlib libfoo.a
+ rm -f foo.o
+ cc  -shared  -o libfoo.so.3.4.5 -all libfoo.a -notall 
+ if	silent test ! -d lib
+ then	mkdir -p lib 		    		   
+ fi
+ if	silent test \'\' != "libfoo.a"
+ then	if	silent test \'\' != ""
+ 	then	: lib/libfoo.a linked to libfoo.a
+ 	elif	silent test -d "libfoo.a"
+ 	then	cp -pr libfoo.a lib
+ 	else	silent cmp -s libfoo.a lib/libfoo.a ||
+ 		{
+ 		if	silent test -f "lib/libfoo.a"
+ 		then	{ mv -f lib/libfoo.a lib/libfoo.a.old || { test -f lib/libfoo.a && ignore rm -f lib/libfoo.a.old* && mv -f lib/libfoo.a `echo lib/libfoo.a.old* | sed -e \'s/.* //\' -e \'s/old\\(z*\\)$/old\\1z/\' -e \'s/\\*$//\'`; }; }
+ 		fi
+ 		ignore cp libfoo.a lib/libfoo.a  		    		   
+ 		}
+ 	fi
+ fi
+ ignore ranlib lib/libfoo.a
+ if	silent test ! -d lib/lib
+ then	mkdir -p lib/lib 		    		   
+ fi
+ if	silent test \'\' != "foo.req"
+ then	if	silent test \'\' != ""
+ 	then	: lib/lib/foo linked to foo.req
+ 	elif	silent test -d "foo.req"
+ 	then	cp -pr foo.req lib/lib
+ 	else	silent cmp -s foo.req lib/lib/foo ||
+ 		{
+ 		if	silent test -f "lib/lib/foo"
+ 		then	{ mv -f lib/lib/foo lib/lib/foo.old || { test -f lib/lib/foo && ignore rm -f lib/lib/foo.old* && mv -f lib/lib/foo `echo lib/lib/foo.old* | sed -e \'s/.* //\' -e \'s/old\\(z*\\)$/old\\1z/\' -e \'s/\\*$//\'`; }; }
+ 		fi
+ 		ignore cp foo.req lib/lib/foo  		    		   
+ 		}
+ 	fi
+ fi
+ if	silent test -f lib/libfoo.oo.3.4.5
+ then	/bin/rm -f lib/libfoo.oo.3.4.5
+ fi
+ if	silent test -f lib/libfoo.so.3.4.5
+ then	/bin/mv lib/libfoo.so.3.4.5 lib/libfoo.oo.3.4.5
+ fi
+ /bin/cp libfoo.so.3.4.5 lib/libfoo.so.3.4.5
+ if	silent test "lib/libfoo.so.3.4.5" != "lib/libfoo.so"
+ then	if	silent test -f lib/libfoo.so
+ 	then	/bin/rm -f lib/libfoo.so
+ 	fi
+ 	/bin/ln lib/libfoo.so.3.4.5 lib/libfoo.so
+ fi
+ chmod -w lib/libfoo.so.3.4.5'

TEST 12 '-l prereq libs'

	EXEC	-n
		INPUT Makefile $'baz :: baz.c -lfoo'
		INPUT baz.c $'int main() { return 0; }'
		INPUT foo.req $' -lfoo -lbar'
		INPUT libfoo.a
		INPUT bar.req $' -lbar'
		INPUT libbar.a
		OUTPUT - $'+ cc -O   -c baz.c
+ cc  -O   -o baz baz.o libfoo.a libbar.a'

	EXEC	-n
		INPUT foo.req $' -lbar'
		OUTPUT - $'+ cc -O   -c baz.c
+ cc  -O   -o baz baz.o libbar.a'

	EXEC	-n
		INPUT foo.req
		OUTPUT - $'+ cc -O   -c baz.c
+ cc  -O   -o baz baz.o'

	EXEC	-n
		INPUT foo.req $' -lfoo -lbar'
		INPUT bar.req $' -lbar -lfoo'
		OUTPUT - $'+ cc -O   -c baz.c
+ cc  -O   -o baz baz.o libfoo.a libfoo.a libbar.a'

TEST 13 'nested : scope'

	EXEC	-n c
		INPUT Makefile $'VAR = 1
a :
b : a
	: $(<) : $(*) : VAR=$(VAR) :
c : b VAR=2
	: $(<) : $(*) : VAR=$(VAR) :'
		OUTPUT - $'+ : b : a : VAR=1 :
+ : c : b : VAR=2 :'

	EXEC	-n b c
