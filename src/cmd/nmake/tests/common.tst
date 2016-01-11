# ast nmake base rule common action tests

INCLUDE cc.def

TEST 01 'install with --compare and --clobber'

	EXPORT INSTALLROOT=.
	EXPORT SHELL=/bin/sh

	EXEC	--regress=sync install
		INPUT Makefile $'t :: t.sh'
		INPUT t.sh aaa
		ERROR - $'+ cp t.sh t
+ chmod u+w,+x t
+ mkdir -p bin
+ ignore cp t bin/t'

	EXEC	--regress=sync install
		INPUT t.sh aaa
		ERROR - $'+ cp t.sh t'

	EXEC	--regress=sync --nocompare install
		INPUT t.sh aaa
		ERROR - $'+ cp t.sh t
+ mv -f bin/t bin/t.old
+ ignore cp t bin/t'

	EXEC	--regress=sync install compare=0 # obsolete compatibility
		INPUT t.sh aaa
		ERROR - $'make: warning: compare=0: obsolete: use --nocompare
+ cp t.sh t
+ mv -f bin/t bin/t.old
+ ignore cp t bin/t'

	EXEC	--regress=sync --nocompare --clobber install
		INPUT t.sh aaa
		ERROR - $'+ cp t.sh t
+ rm -f bin/t
+ ignore cp t bin/t'

	EXEC	--regress=sync install compare=0 clobber=1 # obsolete compatibility
		INPUT t.sh aaa
		ERROR - $'make: warning: clobber=1 compare=0: obsolete: use --clobber --nocompare
+ cp t.sh t
+ rm -f bin/t
+ ignore cp t bin/t'

TEST 02 'install+clean+clobber' && {

	CD	root/package

	EXPORT INSTALLROOT=../..

	EXEC	install
		INPUT Makefile $'Hdrs = a.h b.h c.h
a :: a.c
a.h :
	touch $(<)
$(INCLUDEDIR) :INSTALLDIR: $(Hdrs)'
		INPUT a.c $'#include "a.h"
main(){return 0;}'
		INPUT b.h
		INPUT c.h
		ERROR - $'+ touch a.h
+ cc -O -I. -c a.c
+ cc -O -o a a.o
+ mkdir -p ../../bin
+ ignore cp a ../../bin/a
+ mkdir -p ../../include
+ ignore cp a.h ../../include/a.h
+ ignore cp b.h ../../include/b.h
+ ignore cp c.h ../../include/c.h'

	EXEC	install
		ERROR -

	EXEC	clean
		ERROR - $'+ ignore rm -f a.h a.o'

	EXEC	clobber
		ERROR - $'+ ignore rm -f -r a Makefile.mo Makefile.ms'

	EXEC	clobber
		ERROR - $'+ ignore rm -f -r Makefile.mo'

	EXEC	--nocompare install '$(BINDIR)/a'
		ERROR - $'+ touch a.h
+ cc -O -I. -c a.c
+ cc -O -o a a.o
+ mv -f ../../bin/a ../../bin/a.old
+ ignore cp a ../../bin/a
+ mv -f ../../include/a.h ../../include/a.h.old
+ ignore cp a.h ../../include/a.h
+ mv -f ../../include/b.h ../../include/b.h.old
+ ignore cp b.h ../../include/b.h
+ mv -f ../../include/c.h ../../include/c.h.old
+ ignore cp c.h ../../include/c.h'

	EXEC	clobber
		ERROR - $'+ ignore rm -f -r a.h a.o a Makefile.mo Makefile.ms'

}

TEST 03 '*FLAGS'

	EXEC	-n
		INPUT Makefile $'CCFLAGS += --CC.WARN
CCLDFLAGS += --CC.EXPORT.DYNAMIC
tst :: tst.c'
		INPUT tst.c
		OUTPUT - $'+ cc -O --CC.WARN   -c tst.c
+ cc --CC.EXPORT.DYNAMIC  -O --CC.WARN   -o tst tst.o'

TEST 04 'cc-'

	EXEC	-n all cc-
		INPUT Makefile $'CCFLAGS += -Ddummy
all :
	echo CCFLAGS=$(CCFLAGS)'
		INPUT cc-pg/
		OUTPUT - $'+ echo CCFLAGS=-O -Ddummy  
+ echo CCFLAGS=-pg  '
		ERROR - $'cc-pg:'

TEST 05 'command arg target vs common action => .ACTION'

	EXEC	-n query
		INPUT Makefile $'query :
	: $(<) :'
		OUTPUT - $'+ : query :'

	EXEC	-n query
		INPUT Makefile $'query :: query.c'
		INPUT query.c $'int main(){return 0;}'
		OUTPUT - $'+ cc -O   -c query.c
+ cc  -O   -o query query.o'

	EXEC	-n query .
		INPUT Makefile $'all{ls}'
		INPUT - $'all'
		OUTPUT -
		ERROR - $'
all : [not found] target unbound
 action:
	 ls
'

TEST 06 ':: with .o lhs'

	EXEC	-n
		INPUT Makefile $'if HIT
.OBJECT.o : .USE .COMMAND
	: HIT -o $(<) $(*)
end
o.o :: a.c z.c'
		INPUT a.c
		INPUT z.c
		OUTPUT - $'+ cc -O   -c a.c
+ cc -O   -c z.c
+ ld -r  -o o.o a.o z.o'

	EXEC	-n HIT=1
		OUTPUT - $'+ cc -O   -c a.c
+ cc -O   -c z.c
+ : HIT -o o.o a.o z.o'

TEST 07 'install with link on'

	EXEC	-n install
		INPUT Makefile $'INSTALLROOT = root
tst :LIBRARY: tst.c -laha
aha :LIBRARY: aha.c'
		INPUT tst.c
		INPUT tst.req ' -ltst\n -lm'
		INPUT aha.c
		INPUT aha.req ' -laha'
		OUTPUT - $'+ echo "" -ltst -laha > tst.req
+ cc -O   -c tst.c
+ ar  cr libtst.a tst.o
+ ignore ranlib libtst.a
+ rm -f tst.o
+ echo "" -laha > aha.req
+ cc -O   -c aha.c
+ ar  cr libaha.a aha.o
+ ignore ranlib libaha.a
+ rm -f aha.o
+ if	silent test ! -d root/lib
+ then	mkdir -p root/lib 		    		   
+ fi
+ if	silent test \'\' != "libtst.a"
+ then	if	silent test \'\' != ""
+ 	then	: root/lib/libtst.a linked to libtst.a
+ 	elif	silent test -d "libtst.a"
+ 	then	cp -pr libtst.a root/lib
+ 	else	silent cmp -s libtst.a root/lib/libtst.a ||
+ 		{
+ 		if	silent test -f "root/lib/libtst.a"
+ 		then	{ mv -f root/lib/libtst.a root/lib/libtst.a.old || { test -f root/lib/libtst.a && ignore rm -f root/lib/libtst.a.old* && mv -f root/lib/libtst.a `echo root/lib/libtst.a.old* | sed -e \'s/.* //\' -e \'s/old\\(z*\\)$/old\\1z/\' -e \'s/\\*$//\'`; }; }
+ 		fi
+ 		ignore cp libtst.a root/lib/libtst.a  		    		   
+ 		}
+ 	fi
+ fi
+ ignore ranlib root/lib/libtst.a
+ if	silent test ! -d root/lib/lib
+ then	mkdir -p root/lib/lib 		    		   
+ fi
+ if	silent test \'\' != "tst.req"
+ then	if	silent test \'\' != ""
+ 	then	: root/lib/lib/tst linked to tst.req
+ 	elif	silent test -d "tst.req"
+ 	then	cp -pr tst.req root/lib/lib
+ 	else	silent cmp -s tst.req root/lib/lib/tst ||
+ 		{
+ 		if	silent test -f "root/lib/lib/tst"
+ 		then	{ mv -f root/lib/lib/tst root/lib/lib/tst.old || { test -f root/lib/lib/tst && ignore rm -f root/lib/lib/tst.old* && mv -f root/lib/lib/tst `echo root/lib/lib/tst.old* | sed -e \'s/.* //\' -e \'s/old\\(z*\\)$/old\\1z/\' -e \'s/\\*$//\'`; }; }
+ 		fi
+ 		ignore cp tst.req root/lib/lib/tst  		    		   
+ 		}
+ 	fi
+ fi
+ if	silent test \'\' != "libaha.a"
+ then	if	silent test \'\' != ""
+ 	then	: root/lib/libaha.a linked to libaha.a
+ 	elif	silent test -d "libaha.a"
+ 	then	cp -pr libaha.a root/lib
+ 	else	silent cmp -s libaha.a root/lib/libaha.a ||
+ 		{
+ 		if	silent test -f "root/lib/libaha.a"
+ 		then	{ mv -f root/lib/libaha.a root/lib/libaha.a.old || { test -f root/lib/libaha.a && ignore rm -f root/lib/libaha.a.old* && mv -f root/lib/libaha.a `echo root/lib/libaha.a.old* | sed -e \'s/.* //\' -e \'s/old\\(z*\\)$/old\\1z/\' -e \'s/\\*$//\'`; }; }
+ 		fi
+ 		ignore cp libaha.a root/lib/libaha.a  		    		   
+ 		}
+ 	fi
+ fi
+ ignore ranlib root/lib/libaha.a
+ if	silent test \'\' != "aha.req"
+ then	if	silent test \'\' != ""
+ 	then	: root/lib/lib/aha linked to aha.req
+ 	elif	silent test -d "aha.req"
+ 	then	cp -pr aha.req root/lib/lib
+ 	else	silent cmp -s aha.req root/lib/lib/aha ||
+ 		{
+ 		if	silent test -f "root/lib/lib/aha"
+ 		then	{ mv -f root/lib/lib/aha root/lib/lib/aha.old || { test -f root/lib/lib/aha && ignore rm -f root/lib/lib/aha.old* && mv -f root/lib/lib/aha `echo root/lib/lib/aha.old* | sed -e \'s/.* //\' -e \'s/old\\(z*\\)$/old\\1z/\' -e \'s/\\*$//\'`; }; }
+ 		fi
+ 		ignore cp aha.req root/lib/lib/aha  		    		   
+ 		}
+ 	fi
+ fi'

	EXEC	-n --link="*" install
		OUTPUT - $'+ echo "" -ltst -laha > tst.req
+ cc -O   -c tst.c
+ ar  cr libtst.a tst.o
+ ignore ranlib libtst.a
+ rm -f tst.o
+ echo "" -laha > aha.req
+ cc -O   -c aha.c
+ ar  cr libaha.a aha.o
+ ignore ranlib libaha.a
+ rm -f aha.o
+ if	silent test ! -d root/lib
+ then	mkdir -p root/lib 		    		   
+ fi
+ if	silent test \'\' != "libtst.a"
+ then	if	silent test \'\' != ""
+ 	then	: root/lib/libtst.a linked to libtst.a
+ 	elif	silent test -d "libtst.a"
+ 	then	ln -s ../../libtst.a root/lib/libtst.a || cp -pr libtst.a root/lib
+ 	else	silent cmp -s libtst.a root/lib/libtst.a ||
+ 		{
+ 		if	silent test -f "root/lib/libtst.a"
+ 		then	{ mv -f root/lib/libtst.a root/lib/libtst.a.old || { test -f root/lib/libtst.a && ignore rm -f root/lib/libtst.a.old* && mv -f root/lib/libtst.a `echo root/lib/libtst.a.old* | sed -e \'s/.* //\' -e \'s/old\\(z*\\)$/old\\1z/\' -e \'s/\\*$//\'`; }; }
+ 		fi
+ 		ln -s ../../libtst.a root/lib/libtst.a || ignore cp libtst.a root/lib/libtst.a  		    		   
+ 		}
+ 	fi
+ fi
+ ignore ranlib root/lib/libtst.a
+ if	silent test ! -d root/lib/lib
+ then	mkdir -p root/lib/lib 		    		   
+ fi
+ if	silent test \'\' != "tst.req"
+ then	if	silent test \'\' != ""
+ 	then	: root/lib/lib/tst linked to tst.req
+ 	elif	silent test -d "tst.req"
+ 	then	ln -s ../../../tst.req root/lib/lib/tst || cp -pr tst.req root/lib/lib
+ 	else	silent cmp -s tst.req root/lib/lib/tst ||
+ 		{
+ 		if	silent test -f "root/lib/lib/tst"
+ 		then	{ mv -f root/lib/lib/tst root/lib/lib/tst.old || { test -f root/lib/lib/tst && ignore rm -f root/lib/lib/tst.old* && mv -f root/lib/lib/tst `echo root/lib/lib/tst.old* | sed -e \'s/.* //\' -e \'s/old\\(z*\\)$/old\\1z/\' -e \'s/\\*$//\'`; }; }
+ 		fi
+ 		ln -s ../../../tst.req root/lib/lib/tst || ignore cp tst.req root/lib/lib/tst  		    		   
+ 		}
+ 	fi
+ fi
+ if	silent test \'\' != "libaha.a"
+ then	if	silent test \'\' != ""
+ 	then	: root/lib/libaha.a linked to libaha.a
+ 	elif	silent test -d "libaha.a"
+ 	then	ln -s ../../libaha.a root/lib/libaha.a || cp -pr libaha.a root/lib
+ 	else	silent cmp -s libaha.a root/lib/libaha.a ||
+ 		{
+ 		if	silent test -f "root/lib/libaha.a"
+ 		then	{ mv -f root/lib/libaha.a root/lib/libaha.a.old || { test -f root/lib/libaha.a && ignore rm -f root/lib/libaha.a.old* && mv -f root/lib/libaha.a `echo root/lib/libaha.a.old* | sed -e \'s/.* //\' -e \'s/old\\(z*\\)$/old\\1z/\' -e \'s/\\*$//\'`; }; }
+ 		fi
+ 		ln -s ../../libaha.a root/lib/libaha.a || ignore cp libaha.a root/lib/libaha.a  		    		   
+ 		}
+ 	fi
+ fi
+ ignore ranlib root/lib/libaha.a
+ if	silent test \'\' != "aha.req"
+ then	if	silent test \'\' != ""
+ 	then	: root/lib/lib/aha linked to aha.req
+ 	elif	silent test -d "aha.req"
+ 	then	ln -s ../../../aha.req root/lib/lib/aha || cp -pr aha.req root/lib/lib
+ 	else	silent cmp -s aha.req root/lib/lib/aha ||
+ 		{
+ 		if	silent test -f "root/lib/lib/aha"
+ 		then	{ mv -f root/lib/lib/aha root/lib/lib/aha.old || { test -f root/lib/lib/aha && ignore rm -f root/lib/lib/aha.old* && mv -f root/lib/lib/aha `echo root/lib/lib/aha.old* | sed -e \'s/.* //\' -e \'s/old\\(z*\\)$/old\\1z/\' -e \'s/\\*$//\'`; }; }
+ 		fi
+ 		ln -s ../../../aha.req root/lib/lib/aha || ignore cp aha.req root/lib/lib/aha  		    		   
+ 		}
+ 	fi
+ fi'

	EXEC	-n install link="*" # obsolete compatibility
		ERROR - $'make: warning: link=*: obsolete: use --link=*'

TEST 08 'instrumented compilation'

	DO	mkdir bin
	DO	touch bin/transmorgrify
	DO	chmod +x bin/transmorgrify

	EXPORT	PATH=$PATH:$PWD/bin

	EXEC	-n instrument=transmorgrify INSTRUMENT_transmorgrify='command=CC root=TRANSMORGRIFY_HOME'
		INPUT Makefile $'cmd :: cmd.c -llib
lib :LIBRARY: lib.c'
		INPUT cmd.c $'extern int lib();\nmain() { return lib(); }'
		INPUT lib.c $'extern int lib() { return 0; }'
		INPUT lib.req $' -llib'
		INPUT transmorgrify
		OUTPUT - $'+ TRANSMORGRIFY_HOME='$PWD$'/bin transmorgrify cc -O   -c cmd.c
+ echo "" -llib > lib.req
+ TRANSMORGRIFY_HOME='$PWD$'/bin transmorgrify cc -O   -c lib.c
+ ar  cr liblib-tra.a lib.o
+ ignore ranlib liblib-tra.a
+ rm -f lib.o
+ TRANSMORGRIFY_HOME='$PWD$'/bin transmorgrify cc  -O   -o cmd cmd.o liblib-tra.a'
		ERROR - $'make: warning: instrument=transmorgrify: obsolete: use --instrument=transmorgrify'

TEST 09 ':MAKE: + pax source manifest'

	EXPORT	VPATH=$TWD

	EXEC	--silent -n list.package.source
		INPUT Makefile $':MAKE:'
		INPUT cmd/Makefile $':MAKE:'
		INPUT cmd/c/Makefile $'c :: c.c'
		INPUT cmd/c/c.c $'c(){}'
		INPUT cmd/tcl/Makefile $':PACKAGE: tcl
foo :: foo.c
:: bar
stdio.h : .SCAN.IGNORE'
		INPUT cmd/tcl/foo.c $'#include <stdio.h>\nmain(){}'
		INPUT cmd/tcl/bar/a
		INPUT cmd/tcl/bar/b
		INPUT cmd/tcl/bar/c
		INPUT cmd/tcl/bar/d
		INPUT lib/Makefile $':MAKE:'
		INPUT lib/libl/Makefile $'l :LIBRARY: l.c'
		INPUT lib/libl/l.c $'l(){}'
		OUTPUT - $';;;'$TWD$'/cmd/c/Makefile;cmd/c/Makefile
;;;'$TWD$'/cmd/c/c.c;cmd/c/c.c
;;;'$TWD$'/cmd/tcl/Makefile;cmd/tcl/Makefile
;;;'$TWD$'/cmd/tcl/bar/a;cmd/tcl/bar/a
;;;'$TWD$'/cmd/tcl/bar/b;cmd/tcl/bar/b
;;;'$TWD$'/cmd/tcl/bar/c;cmd/tcl/bar/c
;;;'$TWD$'/cmd/tcl/bar/d;cmd/tcl/bar/d
;;;'$TWD$'/cmd/tcl/foo.c;cmd/tcl/foo.c
;;;'$TWD$'/lib/libl/Makefile;lib/libl/Makefile
;;;'$TWD$'/lib/libl/l.c;lib/libl/l.c
;;;'$TWD$'/lib/Makefile;lib/Makefile
;;;'$TWD$'/cmd/Makefile;cmd/Makefile
;;;'$TWD$'/cc.probe;cc.probe
;;;'$TWD$'/Makefile;Makefile
;;;'$TWD$'/cmd/Makefile;cmd/Makefile
;;;'$TWD$'/cmd/c/Makefile;cmd/c/Makefile
;;;'$TWD$'/cmd/tcl/Makefile;cmd/tcl/Makefile
;;;'$TWD$'/lib/Makefile;lib/Makefile
;;;'$TWD$'/lib/libl/Makefile;lib/libl/Makefile'

TEST 10 '--arclean=edit-op'

	EXEC	-n all
		INPUT Makefile $'tst :LIBRARY: explicit.o implicit.c'
		INPUT explicit.c
		INPUT implicit.c
		OUTPUT - $'+ echo "" -ltst > tst.req
+ cc -O   -c explicit.c
+ cc -O   -c implicit.c
+ ar  cr libtst.a explicit.o implicit.o
+ ignore ranlib libtst.a
+ rm -f explicit.o implicit.o'

	EXEC	-n --arclean=A=.IMPLICIT all
		OUTPUT - $'+ echo "" -ltst > tst.req
+ cc -O   -c explicit.c
+ cc -O   -c implicit.c
+ ar  cr libtst.a explicit.o implicit.o
+ ignore ranlib libtst.a
+ rm -f implicit.o'

TEST 11 'ar member deletion'

	EXEC	--regress=sync
		INPUT Makefile $'tst :LIBRARY: a.c b.c'
		INPUT a.c 'int a(){return 0;}'
		INPUT b.c 'int b(){return 0;}'
		ERROR - $'+ echo \'\' -ltst
+ 1> tst.req
+ cc -O -c a.c
+ cc -O -c b.c
+ ar cr libtst.a a.o b.o
+ ignore ranlib libtst.a
+ rm -f a.o b.o'

	EXEC	--regress=sync
		INPUT Makefile $'tst :LIBRARY: b.c'
		ERROR - $'+ ignore ar d libtst.a a.o
+ ignore ranlib libtst.a'

TEST 12 'install + clean + clobber'

	CD	work

	EXPORT	INSTALLROOT=..

	EXEC	-n
		INPUT $TWD/work/Makefile $'.SOURCE : src inc
CCFLAGS += $$(CC.DLL)
t1 :: main.c
t2 :: main.c a.c
t3.a :: a.c b.c
t4 :LIBRARY: c.c'
		INPUT $TWD/work/main.c $'int main(){return 0;}'
		INPUT $TWD/work/a.c $'int a(){return 0;}'
		INPUT $TWD/work/b.c $'int b(){return 0;}'
		INPUT $TWD/work/c.c $'int c(){return 0;}'
		OUTPUT - $'+ cc -O -D_BLD_DLL -D_BLD_PIC   -c main.c
+ cc  -O   -o t1 main.o'

	EXEC	-n all
		OUTPUT - $'+ cc -O -D_BLD_DLL -D_BLD_PIC   -c main.c
+ cc  -O   -o t1 main.o
+ cc -O -D_BLD_DLL -D_BLD_PIC   -c a.c
+ cc  -O   -o t2 main.o a.o
+ cc -O -D_BLD_DLL -D_BLD_PIC   -c b.c
+ ar  cr t3.a a.o b.o
+ ignore ranlib t3.a
+ rm -f a.o b.o
+ echo "" -lt4 > t4.req
+ cc -O -D_BLD_DLL -D_BLD_PIC   -c c.c
+ ar  cr libt4.a c.o
+ ignore ranlib libt4.a
+ rm -f c.o
+ cc  -shared  -o libt4.so.1.0 -all libt4.a -notall '

	EXEC	-n install
		OUTPUT - $'+ cc -O -D_BLD_DLL -D_BLD_PIC   -c main.c
+ cc  -O   -o t1 main.o
+ cc -O -D_BLD_DLL -D_BLD_PIC   -c a.c
+ cc  -O   -o t2 main.o a.o
+ cc -O -D_BLD_DLL -D_BLD_PIC   -c b.c
+ ar  cr t3.a a.o b.o
+ ignore ranlib t3.a
+ rm -f a.o b.o
+ echo "" -lt4 > t4.req
+ cc -O -D_BLD_DLL -D_BLD_PIC   -c c.c
+ ar  cr libt4.a c.o
+ ignore ranlib libt4.a
+ rm -f c.o
+ cc  -shared  -o libt4.so.1.0 -all libt4.a -notall 
+ if	silent test ! -d ../bin
+ then	mkdir -p ../bin 		    		   
+ fi
+ if	silent test \'\' != "t1"
+ then	if	silent test \'\' != ""
+ 	then	: ../bin/t1 linked to t1
+ 	elif	silent test -d "t1"
+ 	then	cp -pr t1 ../bin
+ 	else	silent cmp -s t1 ../bin/t1 ||
+ 		{
+ 		if	silent test -f "../bin/t1"
+ 		then	{ mv -f ../bin/t1 ../bin/t1.old || { test -f ../bin/t1 && ignore rm -f ../bin/t1.old* && mv -f ../bin/t1 `echo ../bin/t1.old* | sed -e \'s/.* //\' -e \'s/old\\(z*\\)$/old\\1z/\' -e \'s/\\*$//\'`; }; }
+ 		fi
+ 		ignore cp t1 ../bin/t1  		    		   
+ 		}
+ 	fi
+ fi
+ if	silent test \'\' != "t2"
+ then	if	silent test \'\' != ""
+ 	then	: ../bin/t2 linked to t2
+ 	elif	silent test -d "t2"
+ 	then	cp -pr t2 ../bin
+ 	else	silent cmp -s t2 ../bin/t2 ||
+ 		{
+ 		if	silent test -f "../bin/t2"
+ 		then	{ mv -f ../bin/t2 ../bin/t2.old || { test -f ../bin/t2 && ignore rm -f ../bin/t2.old* && mv -f ../bin/t2 `echo ../bin/t2.old* | sed -e \'s/.* //\' -e \'s/old\\(z*\\)$/old\\1z/\' -e \'s/\\*$//\'`; }; }
+ 		fi
+ 		ignore cp t2 ../bin/t2  		    		   
+ 		}
+ 	fi
+ fi
+ if	silent test ! -d ../lib
+ then	mkdir -p ../lib 		    		   
+ fi
+ if	silent test \'\' != "t3.a"
+ then	if	silent test \'\' != ""
+ 	then	: ../lib/t3.a linked to t3.a
+ 	elif	silent test -d "t3.a"
+ 	then	cp -pr t3.a ../lib
+ 	else	silent cmp -s t3.a ../lib/t3.a ||
+ 		{
+ 		if	silent test -f "../lib/t3.a"
+ 		then	{ mv -f ../lib/t3.a ../lib/t3.a.old || { test -f ../lib/t3.a && ignore rm -f ../lib/t3.a.old* && mv -f ../lib/t3.a `echo ../lib/t3.a.old* | sed -e \'s/.* //\' -e \'s/old\\(z*\\)$/old\\1z/\' -e \'s/\\*$//\'`; }; }
+ 		fi
+ 		ignore cp t3.a ../lib/t3.a  		    		   
+ 		}
+ 	fi
+ fi
+ ignore ranlib ../lib/t3.a
+ if	silent test \'\' != "libt4.a"
+ then	if	silent test \'\' != ""
+ 	then	: ../lib/libt4.a linked to libt4.a
+ 	elif	silent test -d "libt4.a"
+ 	then	cp -pr libt4.a ../lib
+ 	else	silent cmp -s libt4.a ../lib/libt4.a ||
+ 		{
+ 		if	silent test -f "../lib/libt4.a"
+ 		then	{ mv -f ../lib/libt4.a ../lib/libt4.a.old || { test -f ../lib/libt4.a && ignore rm -f ../lib/libt4.a.old* && mv -f ../lib/libt4.a `echo ../lib/libt4.a.old* | sed -e \'s/.* //\' -e \'s/old\\(z*\\)$/old\\1z/\' -e \'s/\\*$//\'`; }; }
+ 		fi
+ 		ignore cp libt4.a ../lib/libt4.a  		    		   
+ 		}
+ 	fi
+ fi
+ ignore ranlib ../lib/libt4.a
+ if	silent test ! -d ../lib/lib
+ then	mkdir -p ../lib/lib 		    		   
+ fi
+ if	silent test \'\' != "t4.req"
+ then	if	silent test \'\' != ""
+ 	then	: ../lib/lib/t4 linked to t4.req
+ 	elif	silent test -d "t4.req"
+ 	then	cp -pr t4.req ../lib/lib
+ 	else	silent cmp -s t4.req ../lib/lib/t4 ||
+ 		{
+ 		if	silent test -f "../lib/lib/t4"
+ 		then	{ mv -f ../lib/lib/t4 ../lib/lib/t4.old || { test -f ../lib/lib/t4 && ignore rm -f ../lib/lib/t4.old* && mv -f ../lib/lib/t4 `echo ../lib/lib/t4.old* | sed -e \'s/.* //\' -e \'s/old\\(z*\\)$/old\\1z/\' -e \'s/\\*$//\'`; }; }
+ 		fi
+ 		ignore cp t4.req ../lib/lib/t4  		    		   
+ 		}
+ 	fi
+ fi
+ if	silent test -f ../lib/libt4.oo.1.0
+ then	/bin/rm -f ../lib/libt4.oo.1.0
+ fi
+ if	silent test -f ../lib/libt4.so.1.0
+ then	/bin/mv ../lib/libt4.so.1.0 ../lib/libt4.oo.1.0
+ fi
+ /bin/cp libt4.so.1.0 ../lib/libt4.so.1.0
+ if	silent test "../lib/libt4.so.1.0" != "../lib/libt4.so"
+ then	if	silent test -f ../lib/libt4.so
+ 	then	/bin/rm -f ../lib/libt4.so
+ 	fi
+ 	/bin/ln ../lib/libt4.so.1.0 ../lib/libt4.so
+ fi
+ chmod -w ../lib/libt4.so.1.0'

	EXEC	--regress=sync install
		OUTPUT -
		ERROR - $'+ cc -O -D_BLD_DLL -D_BLD_PIC -c main.c
+ cc -O -o t1 main.o
+ cc -O -D_BLD_DLL -D_BLD_PIC -c a.c
+ cc -O -o t2 main.o a.o
+ cc -O -D_BLD_DLL -D_BLD_PIC -c b.c
+ ar cr t3.a a.o b.o
+ ignore ranlib t3.a
+ rm -f a.o b.o
+ echo \'\' -lt4
+ 1> t4.req
+ cc -O -D_BLD_DLL -D_BLD_PIC -c c.c
+ ar cr libt4.a c.o
+ ignore ranlib libt4.a
+ rm -f c.o
+ cc -shared -o libt4.so.1.0 -all libt4.a -notall
+ mkdir -p ../bin
+ ignore cp t1 ../bin/t1
+ ignore cp t2 ../bin/t2
+ mkdir -p ../lib
+ ignore cp t3.a ../lib/t3.a
+ ignore ranlib ../lib/t3.a
+ ignore cp libt4.a ../lib/libt4.a
+ ignore ranlib ../lib/libt4.a
+ mkdir -p ../lib/lib
+ ignore cp t4.req ../lib/lib/t4
+ /bin/cp libt4.so.1.0 ../lib/libt4.so.1.0
+ /bin/ln ../lib/libt4.so.1.0 ../lib/libt4.so
+ chmod -w ../lib/libt4.so.1.0'

	EXEC	-n list.install
		OUTPUT - $'bin
bin/t1
bin/t2
lib
lib/t3.a
lib/libt4.a
lib/lib
lib/lib/t4
lib/libt4.so.1.0
lib/libt4.so'
		ERROR -

	EXEC	-n clobber.install
		OUTPUT - $'+ ignore rm -f -r ../bin/t1 ../bin/t2 ../lib/t3.a ../lib/libt4.a ../lib/lib/t4 ../lib/libt4.so.1.0 ../lib/libt4.so'

	EXEC	-n clean
		OUTPUT - $'+ ignore rm -f main.o t4.req'

	EXEC	tar TARFLAGS=
		OUTPUT -
		ERROR - $'+ tar cf work.tar Makefile a.c b.c c.c main.c'

	EXEC	-n clean
		OUTPUT - $'+ ignore rm -f main.o t4.req'
		ERROR -

	EXEC	-n clobber
		OUTPUT - $'+ ignore rm -f -r  libt4.so.1.0 t3.a main.o t1 t2 t4.req libt4.a Makefile.mo Makefile.ms'

TEST 13 'clobber + joint metarule'

	EXEC	--regress=sync
		INPUT Makefile $'%.c c_%.c %.h : %.sch
	sed s/FUN/fun/g $(>) > $(>:B:S=.c)
	echo "#include \\"$(>:B).h\\"" > c_$(>:B:S=.c)
	sed s/FUN/nuf/g $(>) >> c_$(>:B:S=.c)
	echo "#define $(>:B)_x 1" > $(>:B:S=.h)
.INIT : .init
.init : .MAKE .VIRTUAL .FORCE
	local T
	.BUILT. := $(.BUILT.:V:/$(")")$/:H&)
a :LIBRARY: g.sch'
		INPUT g.sch 'int FUN() { return 0; }'
		INPUT g.h '#define G 1'
		ERROR - $'+ echo \'\' -la
+ 1> a.req
+ sed s/FUN/fun/g g.sch
+ 1> g.c
+ echo \'#include "g.h"\'
+ 1> c_g.c
+ sed s/FUN/nuf/g g.sch
+ 1>> c_g.c
+ echo \'#define g_x 1\'
+ 1> g.h
+ cc -O -c g.c
+ cc -O -I. -c c_g.c
+ ar cr liba.a g.o c_g.o
+ ignore ranlib liba.a
+ rm -f g.o c_g.o'

	EXEC	clean
		ERROR - $'+ ignore rm -f a.req c_g.c g.c g.h'

	EXEC	clobber
		ERROR - $'+ ignore rm -f -r Makefile.mo Makefile.ms liba.a'

TEST 14 'cc-'

	EXEC	-n all cc-
		INPUT Makefile $'t :LIBRARY: t.c'
		INPUT t.c
		INPUT cc-g/
		OUTPUT - $'+ echo "" -lt > t.req
+ cc -O   -c t.c
+ ar  cr libt.a t.o
+ ignore ranlib libt.a
+ rm -f t.o
+ echo "" -lt > t.req
+ cc -g   -c ../t.c
+ ar  cr libt-g.a t.o
+ ignore ranlib libt-g.a
+ rm -f t.o'
		ERROR - $'cc-g:'

TEST 15 'install + link=name'

	EXPORT	INSTALLROOT=.

	EXEC	-n --regress=sync install
		INPUT Makefile $'
.SOURCE : src
$(INCLUDEDIR) :INSTALLDIR: t.h
t :: main.c
t2 :: main.c
t :LIBRARY: a.c'
		INPUT t.h $'#define T 1'
		INPUT main.c $'int main(){return 0;}'
		INPUT a.c $'int a(){return 0;}'
		OUTPUT - $'+ cc -O   -c main.c
+ cc  -O   -o t main.o
+ cc  -O   -o t2 main.o
+ echo "" -lt > t.req
+ cc -O   -c a.c
+ ar  cr libt.a a.o
+ ignore ranlib libt.a
+ rm -f a.o
+ if	silent test ! -d include
+ then	mkdir -p include 		    		   
+ fi
+ if	silent test \'\' != "t.h"
+ then	if	silent test \'\' != ""
+ 	then	: include/t.h linked to t.h
+ 	elif	silent test -d "t.h"
+ 	then	cp -pr t.h include
+ 	else	silent cmp -s t.h include/t.h ||
+ 		{
+ 		if	silent test -f "include/t.h"
+ 		then	{ mv -f include/t.h include/t.h.old || { test -f include/t.h && ignore rm -f include/t.h.old* && mv -f include/t.h `echo include/t.h.old* | sed -e \'s/.* //\' -e \'s/old\\(z*\\)$/old\\1z/\' -e \'s/\\*$//\'`; }; }
+ 		fi
+ 		ignore cp t.h include/t.h  		    		   
+ 		}
+ 	fi
+ fi
+ if	silent test ! -d bin
+ then	mkdir -p bin 		    		   
+ fi
+ if	silent test \'\' != "t"
+ then	if	silent test \'\' != ""
+ 	then	: bin/t linked to t
+ 	elif	silent test -d "t"
+ 	then	cp -pr t bin
+ 	else	silent cmp -s t bin/t ||
+ 		{
+ 		if	silent test -f "bin/t"
+ 		then	{ mv -f bin/t bin/t.old || { test -f bin/t && ignore rm -f bin/t.old* && mv -f bin/t `echo bin/t.old* | sed -e \'s/.* //\' -e \'s/old\\(z*\\)$/old\\1z/\' -e \'s/\\*$//\'`; }; }
+ 		fi
+ 		ignore cp t bin/t  		    		   
+ 		}
+ 	fi
+ fi
+ if	silent test \'\' != "t2"
+ then	if	silent test \'\' != ""
+ 	then	: bin/t2 linked to t2
+ 	elif	silent test -d "t2"
+ 	then	cp -pr t2 bin
+ 	else	silent cmp -s t2 bin/t2 ||
+ 		{
+ 		if	silent test -f "bin/t2"
+ 		then	{ mv -f bin/t2 bin/t2.old || { test -f bin/t2 && ignore rm -f bin/t2.old* && mv -f bin/t2 `echo bin/t2.old* | sed -e \'s/.* //\' -e \'s/old\\(z*\\)$/old\\1z/\' -e \'s/\\*$//\'`; }; }
+ 		fi
+ 		ignore cp t2 bin/t2  		    		   
+ 		}
+ 	fi
+ fi
+ if	silent test ! -d lib
+ then	mkdir -p lib 		    		   
+ fi
+ if	silent test \'\' != "libt.a"
+ then	if	silent test \'\' != ""
+ 	then	: lib/libt.a linked to libt.a
+ 	elif	silent test -d "libt.a"
+ 	then	cp -pr libt.a lib
+ 	else	silent cmp -s libt.a lib/libt.a ||
+ 		{
+ 		if	silent test -f "lib/libt.a"
+ 		then	{ mv -f lib/libt.a lib/libt.a.old || { test -f lib/libt.a && ignore rm -f lib/libt.a.old* && mv -f lib/libt.a `echo lib/libt.a.old* | sed -e \'s/.* //\' -e \'s/old\\(z*\\)$/old\\1z/\' -e \'s/\\*$//\'`; }; }
+ 		fi
+ 		ignore cp libt.a lib/libt.a  		    		   
+ 		}
+ 	fi
+ fi
+ ignore ranlib lib/libt.a
+ if	silent test ! -d lib/lib
+ then	mkdir -p lib/lib 		    		   
+ fi
+ if	silent test \'\' != "t.req"
+ then	if	silent test \'\' != ""
+ 	then	: lib/lib/t linked to t.req
+ 	elif	silent test -d "t.req"
+ 	then	cp -pr t.req lib/lib
+ 	else	silent cmp -s t.req lib/lib/t ||
+ 		{
+ 		if	silent test -f "lib/lib/t"
+ 		then	{ mv -f lib/lib/t lib/lib/t.old || { test -f lib/lib/t && ignore rm -f lib/lib/t.old* && mv -f lib/lib/t `echo lib/lib/t.old* | sed -e \'s/.* //\' -e \'s/old\\(z*\\)$/old\\1z/\' -e \'s/\\*$//\'`; }; }
+ 		fi
+ 		ignore cp t.req lib/lib/t  		    		   
+ 		}
+ 	fi
+ fi'

	EXEC	-n --regress=sync install link=t
		OUTPUT - $'+ cc -O   -c main.c
+ cc  -O   -o t main.o
+ cc  -O   -o t2 main.o
+ echo "" -lt > t.req
+ cc -O   -c a.c
+ ar  cr libt.a a.o
+ ignore ranlib libt.a
+ rm -f a.o
+ if	silent test ! -d include
+ then	mkdir -p include 		    		   
+ fi
+ if	silent test \'\' != "t.h"
+ then	if	silent test \'\' != ""
+ 	then	: include/t.h linked to t.h
+ 	elif	silent test -d "t.h"
+ 	then	cp -pr t.h include
+ 	else	silent cmp -s t.h include/t.h ||
+ 		{
+ 		if	silent test -f "include/t.h"
+ 		then	{ mv -f include/t.h include/t.h.old || { test -f include/t.h && ignore rm -f include/t.h.old* && mv -f include/t.h `echo include/t.h.old* | sed -e \'s/.* //\' -e \'s/old\\(z*\\)$/old\\1z/\' -e \'s/\\*$//\'`; }; }
+ 		fi
+ 		ignore cp t.h include/t.h  		    		   
+ 		}
+ 	fi
+ fi
+ if	silent test ! -d bin
+ then	mkdir -p bin 		    		   
+ fi
+ if	silent test \'\' != "t"
+ then	if	silent test \'\' != ""
+ 	then	: bin/t linked to t
+ 	elif	silent test -d "t"
+ 	then	ln -s ../t bin/t || cp -pr t bin
+ 	else	silent cmp -s t bin/t ||
+ 		{
+ 		if	silent test -f "bin/t"
+ 		then	{ mv -f bin/t bin/t.old || { test -f bin/t && ignore rm -f bin/t.old* && mv -f bin/t `echo bin/t.old* | sed -e \'s/.* //\' -e \'s/old\\(z*\\)$/old\\1z/\' -e \'s/\\*$//\'`; }; }
+ 		fi
+ 		ln -s ../t bin/t || ignore cp t bin/t  		    		   
+ 		}
+ 	fi
+ fi
+ if	silent test \'\' != "t2"
+ then	if	silent test \'\' != ""
+ 	then	: bin/t2 linked to t2
+ 	elif	silent test -d "t2"
+ 	then	cp -pr t2 bin
+ 	else	silent cmp -s t2 bin/t2 ||
+ 		{
+ 		if	silent test -f "bin/t2"
+ 		then	{ mv -f bin/t2 bin/t2.old || { test -f bin/t2 && ignore rm -f bin/t2.old* && mv -f bin/t2 `echo bin/t2.old* | sed -e \'s/.* //\' -e \'s/old\\(z*\\)$/old\\1z/\' -e \'s/\\*$//\'`; }; }
+ 		fi
+ 		ignore cp t2 bin/t2  		    		   
+ 		}
+ 	fi
+ fi
+ if	silent test ! -d lib
+ then	mkdir -p lib 		    		   
+ fi
+ if	silent test \'\' != "libt.a"
+ then	if	silent test \'\' != ""
+ 	then	: lib/libt.a linked to libt.a
+ 	elif	silent test -d "libt.a"
+ 	then	cp -pr libt.a lib
+ 	else	silent cmp -s libt.a lib/libt.a ||
+ 		{
+ 		if	silent test -f "lib/libt.a"
+ 		then	{ mv -f lib/libt.a lib/libt.a.old || { test -f lib/libt.a && ignore rm -f lib/libt.a.old* && mv -f lib/libt.a `echo lib/libt.a.old* | sed -e \'s/.* //\' -e \'s/old\\(z*\\)$/old\\1z/\' -e \'s/\\*$//\'`; }; }
+ 		fi
+ 		ignore cp libt.a lib/libt.a  		    		   
+ 		}
+ 	fi
+ fi
+ ignore ranlib lib/libt.a
+ if	silent test ! -d lib/lib
+ then	mkdir -p lib/lib 		    		   
+ fi
+ if	silent test \'\' != "t.req"
+ then	if	silent test \'\' != ""
+ 	then	: lib/lib/t linked to t.req
+ 	elif	silent test -d "t.req"
+ 	then	ln -s ../../t.req lib/lib/t || cp -pr t.req lib/lib
+ 	else	silent cmp -s t.req lib/lib/t ||
+ 		{
+ 		if	silent test -f "lib/lib/t"
+ 		then	{ mv -f lib/lib/t lib/lib/t.old || { test -f lib/lib/t && ignore rm -f lib/lib/t.old* && mv -f lib/lib/t `echo lib/lib/t.old* | sed -e \'s/.* //\' -e \'s/old\\(z*\\)$/old\\1z/\' -e \'s/\\*$//\'`; }; }
+ 		fi
+ 		ln -s ../../t.req lib/lib/t || ignore cp t.req lib/lib/t  		    		   
+ 		}
+ 	fi
+ fi'
		ERROR - $'make: warning: link=t: obsolete: use --link=t'

TEST 16 'install + link=*'

	EXPORT	VPATH=$TWD/dev:$TWD/ofc

	CD	ofc/src

	EXEC	--regress=sync install
		INPUT Makefile $'INSTALLROOT=../..
t :LIBRARY: a.c b.c c.c'
		INPUT a.c $'int a(){return 0;}'
		INPUT b.c $'int b(){return 0;}'
		INPUT c.c $'int c(){return 0;}'
		ERROR - $'+ echo \'\' -lt
+ 1> t.req
+ cc -O -c a.c
+ cc -O -c b.c
+ cc -O -c c.c
+ ar cr libt.a a.o b.o c.o
+ ignore ranlib libt.a
+ rm -f a.o b.o c.o
+ mkdir -p ../../lib
+ ignore cp libt.a ../../lib/libt.a
+ ignore ranlib ../../lib/libt.a
+ mkdir -p ../../lib/lib
+ ignore cp t.req ../../lib/lib/t'

	EXEC	--regress=sync install
		ERROR -

	CD	../../dev/src

	EXEC	--regress=sync install
		ERROR -

	EXEC	--regress=sync install
		ERROR -

TEST 17 'install + list.install + clobber.install'

	CD	src

	EXEC	install
		INPUT Makefile $'INSTALLROOT = ..
t :: t.sh'
		INPUT t.sh
		ERROR - $'+ cp t.sh t
+ chmod u+w,+x t
+ mkdir -p ../bin
+ ignore cp t ../bin/t'

	EXEC	list.install
		OUTPUT - $'bin\nbin/t'
		ERROR -

	EXEC	clobber.install
		OUTPUT -
		ERROR - $'+ ignore rm -f -r ../bin/t'

TEST 18 'install with dups'

	EXEC	-n install
		INPUT Makefile $'ETCDIR = etc
$(ETCDIR) :INSTALLDIR: NSTMTC.fld
$(ETCDIR)/EIT5E.fld :INSTALL: NSTMTC.fld
$(ETCDIR)/EIT4EP.fld :INSTALL: NSTMTC.fld
$(ETCDIR)/NSTUI.fld :INSTALL: NSTMTC.fld'
		INPUT NSTMTC.fld
		OUTPUT - $'+ if	silent test ! -d etc
+ then	mkdir -p etc 		    		   
+ fi
+ if	silent test \'\' != "NSTMTC.fld"
+ then	if	silent test \'\' != ""
+ 	then	: etc/NSTMTC.fld linked to NSTMTC.fld
+ 	elif	silent test -d "NSTMTC.fld"
+ 	then	cp -pr NSTMTC.fld etc
+ 	else	silent cmp -s NSTMTC.fld etc/NSTMTC.fld ||
+ 		{
+ 		if	silent test -f "etc/NSTMTC.fld"
+ 		then	{ mv -f etc/NSTMTC.fld etc/NSTMTC.fld.old || { test -f etc/NSTMTC.fld && ignore rm -f etc/NSTMTC.fld.old* && mv -f etc/NSTMTC.fld `echo etc/NSTMTC.fld.old* | sed -e \'s/.* //\' -e \'s/old\\(z*\\)$/old\\1z/\' -e \'s/\\*$//\'`; }; }
+ 		fi
+ 		ignore cp NSTMTC.fld etc/NSTMTC.fld  		    		   
+ 		}
+ 	fi
+ fi
+ if	silent test \'\' != "NSTMTC.fld"
+ then	if	silent test \'\' != ""
+ 	then	: etc/EIT5E.fld linked to NSTMTC.fld
+ 	elif	silent test -d "NSTMTC.fld"
+ 	then	cp -pr NSTMTC.fld etc
+ 	else	silent cmp -s NSTMTC.fld etc/EIT5E.fld ||
+ 		{
+ 		if	silent test -f "etc/EIT5E.fld"
+ 		then	{ mv -f etc/EIT5E.fld etc/EIT5E.fld.old || { test -f etc/EIT5E.fld && ignore rm -f etc/EIT5E.fld.old* && mv -f etc/EIT5E.fld `echo etc/EIT5E.fld.old* | sed -e \'s/.* //\' -e \'s/old\\(z*\\)$/old\\1z/\' -e \'s/\\*$//\'`; }; }
+ 		fi
+ 		ignore cp NSTMTC.fld etc/EIT5E.fld  		    		   
+ 		}
+ 	fi
+ fi
+ if	silent test \'\' != "NSTMTC.fld"
+ then	if	silent test \'\' != ""
+ 	then	: etc/EIT4EP.fld linked to NSTMTC.fld
+ 	elif	silent test -d "NSTMTC.fld"
+ 	then	cp -pr NSTMTC.fld etc
+ 	else	silent cmp -s NSTMTC.fld etc/EIT4EP.fld ||
+ 		{
+ 		if	silent test -f "etc/EIT4EP.fld"
+ 		then	{ mv -f etc/EIT4EP.fld etc/EIT4EP.fld.old || { test -f etc/EIT4EP.fld && ignore rm -f etc/EIT4EP.fld.old* && mv -f etc/EIT4EP.fld `echo etc/EIT4EP.fld.old* | sed -e \'s/.* //\' -e \'s/old\\(z*\\)$/old\\1z/\' -e \'s/\\*$//\'`; }; }
+ 		fi
+ 		ignore cp NSTMTC.fld etc/EIT4EP.fld  		    		   
+ 		}
+ 	fi
+ fi
+ if	silent test \'\' != "NSTMTC.fld"
+ then	if	silent test \'\' != ""
+ 	then	: etc/NSTUI.fld linked to NSTMTC.fld
+ 	elif	silent test -d "NSTMTC.fld"
+ 	then	cp -pr NSTMTC.fld etc
+ 	else	silent cmp -s NSTMTC.fld etc/NSTUI.fld ||
+ 		{
+ 		if	silent test -f "etc/NSTUI.fld"
+ 		then	{ mv -f etc/NSTUI.fld etc/NSTUI.fld.old || { test -f etc/NSTUI.fld && ignore rm -f etc/NSTUI.fld.old* && mv -f etc/NSTUI.fld `echo etc/NSTUI.fld.old* | sed -e \'s/.* //\' -e \'s/old\\(z*\\)$/old\\1z/\' -e \'s/\\*$//\'`; }; }
+ 		fi
+ 		ignore cp NSTMTC.fld etc/NSTUI.fld  		    		   
+ 		}
+ 	fi
+ fi'

TEST 19 'synthesized rule name clashes'

	EXEC	-n install
		INPUT Makefile $'file :
	: > $(<)
../../install/file :INSTALL: file'
		OUTPUT - $'+ if	silent test ! -d ../../install
+ then	mkdir -p ../../install 		    		   
+ fi
+ : > file
+ if	silent test \'\' != "file"
+ then	if	silent test \'\' != ""
+ 	then	: ../../install/file linked to file
+ 	elif	silent test -d "file"
+ 	then	cp -pr file ../../install
+ 	else	silent cmp -s file ../../install/file ||
+ 		{
+ 		if	silent test -f "../../install/file"
+ 		then	{ mv -f ../../install/file ../../install/file.old || { test -f ../../install/file && ignore rm -f ../../install/file.old* && mv -f ../../install/file `echo ../../install/file.old* | sed -e \'s/.* //\' -e \'s/old\\(z*\\)$/old\\1z/\' -e \'s/\\*$//\'`; }; }
+ 		fi
+ 		ignore cp file ../../install/file  		    		   
+ 		}
+ 	fi
+ fi'

TEST 20 'cc- permutations and perturbations'

	EXEC	-n all cc-
		INPUT Makefile $'CCFLAGS += -Ddummy
TEST = abc
DATA = DATA
all :
	echo [ $(-errorid) ] CCFLAGS=$(CCFLAGS) TEST=$(TEST) DATA=$(DATA)'
		INPUT cc-pg/
		INPUT cc-,TEST=123,DATA=data/
		INPUT cc-O,TEST=789
		OUTPUT - $'+ echo [  ] CCFLAGS=-O -Ddummy   TEST=abc DATA=DATA
+ echo [ cc-,TEST=123,DATA=data ] CCFLAGS=-O -Ddummy   TEST=123 DATA=data
+ echo [ cc-pg ] CCFLAGS=-pg   TEST=abc DATA=DATA'
		ERROR - $'cc-,TEST=123,DATA=data:\ncc-pg:'
