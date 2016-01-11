# ast nmake base rule assertion operator tests

INCLUDE cc.def

TEST 01 ':LIBRARY: with .ARUPDATE'

	EXEC	--regress=sync
		INPUT Makefile $'cmd :: cmd.c -llib
lib :LIBRARY: lib.c
.ARUPDATE : .ARENHANCE
.ARENHANCE : .BEFORE $$(<<<:B:S)$(CC.SUFFIX.OBJECT)
%$(CC.SUFFIX.ARCHIVE)$(CC.SUFFIX.SOURCE) : %$(CC.SUFFIX.ARCHIVE) .FORCE
	echo \'char bye[] = "goodbye $(*)";\' > $(<)'
		INPUT cmd.c $'int main() { return 0; }'
		INPUT lib.c $'int lib() { return 0; }'
		ERROR - $'+ cc -O -c cmd.c
+ echo \'\' -llib
+ 1> lib.req
+ cc -O -c lib.c
+ ar cr liblib.a lib.o
make: warning: liblib.a.c: prerequisite liblib.a is active
+ echo \'char bye[] = "goodbye liblib.a";\'
+ 1> liblib.a.c
+ cc -O -c liblib.a.c
+ ignore ranlib liblib.a
+ rm -f lib.o
+ cc -O -o cmd cmd.o liblib.a'

	EXEC
		ERROR -

TEST 02 ':: with scoped variables'

	EXEC	-n all
		INPUT Makefile $'DEBUG ==
a :: DEBUG=1 a.c x.c
b :: b.c x.c'
		INPUT a.c $'int debug_a = DEBUG;'
		INPUT b.c $'int debug_b = DEBUG;'
		INPUT x.c
		OUTPUT - $'+ cc -O  -DDEBUG -c a.c
+ cc -O   -c x.c
+ cc  -O   -o a a.o x.o
+ cc -O   -c b.c
+ cc  -O   -o b b.o x.o'

	EXEC	-n all DEBUG=2
		OUTPUT - $'+ cc -O  -DDEBUG -c a.c
+ cc -O   -c x.c
+ cc  -O   -o a a.o x.o
+ cc -O  -DDEBUG=2 -c b.c
+ cc  -O   -o b b.o x.o'

TEST 03 ':: vs metarules'

	EXEC	-n
		INPUT Makefile 'x :: y.c'
		INPUT x.c
		INPUT y.c
		OUTPUT - $'+ cc -O   -c y.c
+ cc  -O   -o x y.o'

TEST 04 ':LIBRARY:'

	EXEC	--regress=sync
		INPUT Makefile $'cmd :: cmd.c -ltst
tst :LIBRARY: tst.c'
		INPUT cmd.c $'int main(){return 0;}'
		INPUT tst.c $'int tst(){return 0;}'
		OUTPUT tst.req $' -ltst'
		ERROR - $'+ cc -O -c cmd.c
+ echo \'\' -ltst
+ 1> tst.req
+ cc -O -c tst.c
+ ar cr libtst.a tst.o
+ ignore ranlib libtst.a
+ rm -f tst.o
+ cc -O -o cmd cmd.o libtst.a'

TEST 05 ':NOTHING:'

	EXEC
		INPUT Makeargs $':NOTHING: K&R coding violation'
		INPUT Makefile $'a :: a.c'

	EXEC

TEST 06 ':TABLE:'

	EXEC	-n .
		INPUT Makefile $'TABLE.list = 1
aaa {
	bbb	{
		ccc
	}
	ddd	{
		eee
	}
	fff
}
aaa :TABLE: $(OPTIONS)'
		OUTPUT - $'bbb = 1
bbb_ccc = 1
ddd = 1
ddd_eee = 1
fff = 1'

	EXEC	-n OPTIONS='class=huh format=upper' .
		OUTPUT - $'HUH_BBB = 1
HUH_BBB_CCC = 1
HUH_DDD = 1
HUH_DDD_EEE = 1
HUH_FFF = 1'

	EXEC	-n OPTIONS='class=HUH state=1' .
		OUTPUT - $'HUH_bbb == 1
HUH_bbb_ccc == 1
HUH_ddd == 1
HUH_ddd_eee == 1
HUH_fff == 1'

TEST 07 ':INSTALLDIR:'

	EXPORT INSTALLROOT=.

	EXEC	-n install
		INPUT Makefile $'$(FUNDIR) :INSTALLDIR: test\ntest :: test.sh'
		INPUT test.sh
		OUTPUT - $'+ case message:$OPTIND:$RANDOM in
+ ?*:*:*|*::*|*:*:$RANDOM)
+ 	;;
+ *)	if	ENV= LC_ALL=C x= $SHELL -nc \'[[ a || b ]] && : ${list[level]} $(( 1 + $x )) !(pattern)\' 2>/dev/null
+ 	then	if	grep \'### .*archaic.* ###\' >/dev/null
+ 		then	: test contains archaic constructs :
+ 		else	ENV= LC_ALL=C $SHELL -n test.sh
+ 		fi
+ 	fi
+ 	;;
+ esac
+ case \'\' in
+ "")	case 0 in
+ 	0)	cp test.sh test
+ 		;;
+ 	*)	{
+ 		i=`(read x; echo $x) < test.sh`
+ 		case $i in
+ 		\'#!\'*|*\'||\'*|\':\'*|\'":"\'*|"\':\'"*)	echo "$i" ;;
+ 		esac
+ 		cat - test.sh <<\'!\'
+ 
+ !
+ 		} > test
+ 		;;
+ 	esac
+ 	;;
+ *)	cat - test.sh > test <<\'!\'
+ 
+ 
+ !
+ 	;;
+ esac
+ silent test -w test -a -x test || chmod u+w,+x test
+ if	silent test ! -d fun
+ then	mkdir -p fun 		    		   
+ fi
+ if	silent test \'\' != "test"
+ then	if	silent test \'\' != ""
+ 	then	: fun/test linked to test
+ 	elif	silent test -d "test"
+ 	then	cp -pr test fun
+ 	else	silent cmp -s test fun/test ||
+ 		{
+ 		if	silent test -f "fun/test"
+ 		then	{ mv -f fun/test fun/test.old || { test -f fun/test && ignore rm -f fun/test.old* && mv -f fun/test `echo fun/test.old* | sed -e \'s/.* //\' -e \'s/old\\(z*\\)$/old\\1z/\' -e \'s/\\*$//\'`; }; }
+ 		fi
+ 		ignore cp test fun/test  		    		   
+ 		}
+ 	fi
+ fi'

	EXEC	-n install
		INPUT Makefile $':INSTALLDIR: a
a :: a.c'
		INPUT a.c
		OUTPUT -

TEST 08 ':MAKE: with .mk rhs'

	EXEC	-n
		INPUT Makefile $':MAKE: a.mk b.mk'
		INPUT a.mk $'a :\n\t: making $(<)'
		INPUT b.mk $'b :\n\t: making $(<)'
		OUTPUT - $'+ : making a
+ : making b'
		ERROR - $'a.mk:
b.mk:'

	EXEC	--
		OUTPUT -
		ERROR - $'a.mk:
+ : making a
b.mk:
+ : making b'

TEST 09 ':MAKE: error handling'

	EXEC
		INPUT Makefile $':MAKE:'
		INPUT a/Makefile $'tst :\n\ttrue'
		INPUT b/Makefile $'tst :\n\tfalse'
		INPUT c/Makefile $'tst :\n\ttrue'
		INPUT d/Makefile $'tst :\n\ttrue'
		ERROR - $'a:
+ true
b:
+ false
make [b]: *** exit code 1 making tst
make: *** exit code 1 making b'
		EXIT 1

	EXEC	-k
		ERROR - $'a:
+ true
b:
+ false
make [b]: *** exit code 1 making tst
make [b]: *** 1 action failed
make: *** exit code 1 making b
c:
+ true
d:
+ true
make: *** 1 action failed'

TEST 10 ':MAKE: with dir rhs'

	EXEC	-n
		INPUT Makefile $':MAKE: sub\nall :\n\t: parent'
		INPUT sub/Makefile $'all :\n\t: child'
		OUTPUT - $'+ : child'
		ERROR - $'sub:'

	EXEC	-n all
		OUTPUT - $'+ : child\n+ : parent'
		ERROR - $'sub:'

TEST 11 ':LIBRARY: and :: target clash'

	EXEC	-n all
		INPUT Makefile $'CCFLAGS = $(CC.DLL)
foo :: foo.c -lfoo
foo :LIBRARY: lib.c'
		INPUT foo.c $'extern int foo();
main(){return foo();}'
		INPUT lib.c $'#ifdef __EXPORT__
__EXPORT__
#else
extern
#endif
int foo() { return 123; }'
		INPUT foo.req $' -lfoo
 -lbar'
		INPUT libbar.a
		OUTPUT - $'+ cc -D_BLD_DLL -D_BLD_PIC   -c foo.c
+ echo "" -lfoo > foo.req
+ cc -D_BLD_DLL -D_BLD_PIC   -c lib.c
+ ar  cr libfoo.a lib.o
+ ignore ranlib libfoo.a
+ rm -f lib.o
+ cc     -o foo foo.o libfoo.a libbar.a
+ cc  -shared  -o libfoo.so.1.0 -all libfoo.a -notall '

TEST 12 'multi-level :MAKE:'

	EXEC	-n
		INPUT Makefile $':MAKE:'
		INPUT bar/Makefile $'all :\n\t: $(PWD:B)'
		INPUT foo/fum/Makefile $'all :\n\t: $(PWD:B)'
		OUTPUT - $'+ : bar'
		ERROR - $'bar:'

	EXEC	-n
		INPUT foo/Makefile $':MAKE:'
		OUTPUT - $'+ : bar\n+ : fum'
		ERROR - $'bar:\nfoo/fum:\nfoo:'

TEST 13 ':JOINT: inference'

	# can .JOINT be inferred for targets that share
	# the same prereqs and action? no, since the
	# action may determine the target name via $(<)

	EXEC	-n
		INPUT Makefile $'all : main.o init.o builtins.o
builtins.h : mkbuiltins builtins
	sh $(*:P=E)
builtins.c : mkbuiltins builtins
	sh $(*:P=E)
init.c : mkinit builtins.c
	sh $(*:P=E)'
		INPUT builtins
		INPUT main.c $'#include "builtins.h"'
		INPUT mkbuiltins
		INPUT mkinit
		OUTPUT - $'+ sh ./mkbuiltins ./builtins
+ cc -O    -O   -o builtins builtins.c
+ sh ./mkbuiltins ./builtins
+ cc -O -I.  -c main.c
+ sh ./mkinit ./builtins.c
+ cc -O   -c init.c
+ cc -O   -c builtins.c'

	EXEC	-n
		INPUT Makefile $'all : main.o init.o builtins.o
builtins.h builtins.c : mkbuiltins builtins
	sh $(*:P=E)
init.c : mkinit builtins.c
	sh $(*:P=E)'
		OUTPUT - $'+ sh ./mkbuiltins ./builtins
+ cc -O    -O   -o builtins builtins.c
+ sh ./mkbuiltins ./builtins
+ cc -O -I.  -c main.c
+ sh ./mkinit ./builtins.c
+ cc -O   -c init.c
+ cc -O   -c builtins.c'

	EXEC	-n
		INPUT Makefile $'all : main.o init.o builtins.o
builtins.h builtins.c :JOINT: mkbuiltins builtins
	sh $(*:P=E)
init.c : mkinit builtins.c
	sh $(*:P=E)'
		OUTPUT - $'+ sh ./mkbuiltins ./builtins
+ cc -O    -O   -o builtins builtins.c
+ cc -O -I.  -c main.c
+ sh ./mkinit ./builtins.c
+ cc -O   -c init.c
+ cc -O   -c builtins.c'

TEST 14 ':JOINT:'

	EXEC
		INPUT Makefile $'all : a b c
a b c :JOINT: i
	touch $(<)'
		INPUT i
		ERROR - $'+ touch a b c'

	EXEC	--regress=sync
		ERROR -

	DO	touch b

	EXEC
		ERROR - $'+ touch a b c'

	EXEC
		ERROR -

TEST 15 ':: .sh rhs mismatch with lhs'

	# :: relies on metarules so lhs base must match rhs base for .sh

	EXEC	-n
		INPUT Makefile $'bin/foo :: foo.sh\nbar :: huh.sh'
		INPUT foo.sh
		INPUT huh.sh
		OUTPUT - $'+ case message:$OPTIND:$RANDOM in
+ ?*:*:*|*::*|*:*:$RANDOM)
+ 	;;
+ *)	if	ENV= LC_ALL=C x= $SHELL -nc \'[[ a || b ]] && : ${list[level]} $(( 1 + $x )) !(pattern)\' 2>/dev/null
+ 	then	if	grep \'### .*archaic.* ###\' >/dev/null
+ 		then	: bin/foo contains archaic constructs :
+ 		else	ENV= LC_ALL=C $SHELL -n foo.sh
+ 		fi
+ 	fi
+ 	;;
+ esac
+ case \'\' in
+ "")	case 0 in
+ 	0)	cp foo.sh bin/foo
+ 		;;
+ 	*)	{
+ 		i=`(read x; echo $x) < foo.sh`
+ 		case $i in
+ 		\'#!\'*|*\'||\'*|\':\'*|\'":"\'*|"\':\'"*)	echo "$i" ;;
+ 		esac
+ 		cat - foo.sh <<\'!\'
+ 
+ !
+ 		} > bin/foo
+ 		;;
+ 	esac
+ 	;;
+ *)	cat - foo.sh > bin/foo <<\'!\'
+ 
+ 
+ !
+ 	;;
+ esac
+ silent test -w bin/foo -a -x bin/foo || chmod u+w,+x bin/foo'

TEST 16 ':INSTALLDIR: with dir prerequisite'

	EXEC	install
		INPUT Makefile $'usr :INSTALLDIR: include x.h'
		INPUT include/a.h $'a.tst'
		INPUT include/z.h $'z.tst'
		INPUT x.h $'x.tst'
		ERROR - $'+ mkdir -p usr
+ cp -pr include usr
+ ignore cp x.h usr/x.h'

TEST 17 ':MAKE: explicit order'

	EXEC	-n
		INPUT Makefile $'DIRS  = first
DIRS += middle
DIRS += last
:MAKE: $(DIRS)'
		INPUT first/Makefile $'all:\n\t: $(PWD:B)'
		INPUT middle/Makefile $'all:\n\t: $(PWD:B)'
		INPUT last/Makefile $'all:\n\t: $(PWD:B)'
		OUTPUT - $'+ : first
+ : middle
+ : last'
		ERROR - $'first:
middle:
last:'

TEST 18 ':: and :LIBRARY: with :LINK:'

	EXEC	-n
		INPUT Makefile $':ALL:
test :LIBRARY: lib.c
libtestlib.a :LINK: $(.LIB.NAME. test)'
		INPUT lib.c $'lib(){}'
		OUTPUT - $'+ echo "" -ltest > test.req
+ cc -O   -c lib.c
+ ar  cr libtest.a lib.o
+ ignore ranlib libtest.a
+ rm -f lib.o
+ if	silent test -f "libtestlib.a"
+ then	cp libtestlib.a libtestlib.a.old
+ rm -f libtestlib.a
+ fi
+ ln libtest.a libtestlib.a'

	EXEC	-n
		INPUT Makefile $':ALL:
libtest.a :: lib.c
libtestlib.a :LINK: libtest.a'
		OUTPUT - $'+ cc -O   -c lib.c
+ ar  cr libtest.a lib.o
+ ignore ranlib libtest.a
+ rm -f lib.o
+ if	silent test -f "libtestlib.a"
+ then	cp libtestlib.a libtestlib.a.old
+ rm -f libtestlib.a
+ fi
+ ln libtest.a libtestlib.a'

	EXEC	-n
		INPUT Makefile $':ALL:
testlib.a :: lib.c
libtestlib.a :LINK: testlib.a'
		OUTPUT - $'+ cc -O   -c lib.c
+ ar  cr testlib.a lib.o
+ ignore ranlib testlib.a
+ rm -f lib.o
+ if	silent test -f "libtestlib.a"
+ then	cp libtestlib.a libtestlib.a.old
+ rm -f libtestlib.a
+ fi
+ ln testlib.a libtestlib.a'

TEST 19 ':LINK: + install'

	EXEC	-n install
		INPUT Makefile $'BINDIR = bin
gg :: gg.sh
ug gu :LINK: gg'
		INPUT gg.sh
		OUTPUT - $'+ case message:$OPTIND:$RANDOM in
+ ?*:*:*|*::*|*:*:$RANDOM)
+ 	;;
+ *)	if	ENV= LC_ALL=C x= $SHELL -nc \'[[ a || b ]] && : ${list[level]} $(( 1 + $x )) !(pattern)\' 2>/dev/null
+ 	then	if	grep \'### .*archaic.* ###\' >/dev/null
+ 		then	: gg contains archaic constructs :
+ 		else	ENV= LC_ALL=C $SHELL -n gg.sh
+ 		fi
+ 	fi
+ 	;;
+ esac
+ case \'\' in
+ "")	case 0 in
+ 	0)	cp gg.sh gg
+ 		;;
+ 	*)	{
+ 		i=`(read x; echo $x) < gg.sh`
+ 		case $i in
+ 		\'#!\'*|*\'||\'*|\':\'*|\'":"\'*|"\':\'"*)	echo "$i" ;;
+ 		esac
+ 		cat - gg.sh <<\'!\'
+ 
+ !
+ 		} > gg
+ 		;;
+ 	esac
+ 	;;
+ *)	cat - gg.sh > gg <<\'!\'
+ 
+ 
+ !
+ 	;;
+ esac
+ silent test -w gg -a -x gg || chmod u+w,+x gg
+ if	silent test -f "ug"
+ then	cp ug ug.old
+ rm -f ug
+ fi
+ ln gg ug
+ if	silent test -f "gu"
+ then	cp gu gu.old
+ rm -f gu
+ fi
+ ln gg gu
+ if	silent test ! -d bin
+ then	mkdir -p bin 		    		   
+ fi
+ if	silent test \'\' != "gg"
+ then	if	silent test \'\' != ""
+ 	then	: bin/gg linked to gg
+ 	elif	silent test -d "gg"
+ 	then	cp -pr gg bin
+ 	else	silent cmp -s gg bin/gg ||
+ 		{
+ 		if	silent test -f "bin/gg"
+ 		then	{ mv -f bin/gg bin/gg.old || { test -f bin/gg && ignore rm -f bin/gg.old* && mv -f bin/gg `echo bin/gg.old* | sed -e \'s/.* //\' -e \'s/old\\(z*\\)$/old\\1z/\' -e \'s/\\*$//\'`; }; }
+ 		fi
+ 		ignore cp gg bin/gg  		    		   
+ 		}
+ 	fi
+ fi
+ if	silent test -f "bin/ug"
+ then	cp bin/ug bin/ug.old
+ rm -f bin/ug
+ fi
+ ln bin/gg bin/ug
+ if	silent test -f "bin/gu"
+ then	cp bin/gu bin/gu.old
+ rm -f bin/gu
+ fi
+ ln bin/gg bin/gu'

	EXEC	--regress=sync install
		INPUT Makefile $'INSTALLROOT = .
.COMMAND.o : .USE
	touch $(<)
cmd :: cmd.c
aaa zzz :LINK: cmd'
		INPUT cmd.c $'int main(){return 0;}'
		OUTPUT -
		ERROR - $'+ cc -O -c cmd.c
+ touch cmd
+ ln cmd aaa
+ ln cmd zzz
+ mkdir -p bin
+ ignore cp cmd bin/cmd
+ ln bin/cmd bin/aaa
+ ln bin/cmd bin/zzz'

	EXEC	--regress=sync install
		ERROR -

	EXEC	--regress=sync install
		INPUT aaa zzz
		ERROR - $'+ touch cmd
+ cp aaa aaa.old
+ rm -f aaa
+ ln cmd aaa
+ cp zzz zzz.old
+ rm -f zzz
+ ln cmd zzz
+ mv -f bin/cmd bin/cmd.old
+ ignore cp cmd bin/cmd
+ cp bin/aaa bin/aaa.old
+ rm -f bin/aaa
+ ln bin/cmd bin/aaa
+ cp bin/zzz bin/zzz.old
+ rm -f bin/zzz
+ ln bin/cmd bin/zzz'

	EXEC	--regress=sync install
		ERROR -

	EXEC	--regress=sync install
		INPUT bin/aaa aaa
		ERROR - $'+ mv -f bin/cmd bin/cmd.old
+ ignore cp cmd bin/cmd
+ cp bin/aaa bin/aaa.old
+ rm -f bin/aaa
+ ln bin/cmd bin/aaa
+ cp bin/zzz bin/zzz.old
+ rm -f bin/zzz
+ ln bin/cmd bin/zzz'

	EXEC	install
		ERROR -

	DO	rm aaa

	EXEC	install
		ERROR - $'+ ln cmd aaa'

	EXEC	install
		ERROR -

TEST 20 ':: with dir qualified target'

	EXEC	-n all
		INPUT Makefile $'bin/AAA :: AAA.sh
BBB :: BBB.sh'
		INPUT AAA.sh
		INPUT BBB.sh
		INPUT bin/
		OUTPUT - $'+ case message:$OPTIND:$RANDOM in
+ ?*:*:*|*::*|*:*:$RANDOM)
+ 	;;
+ *)	if	ENV= LC_ALL=C x= $SHELL -nc \'[[ a || b ]] && : ${list[level]} $(( 1 + $x )) !(pattern)\' 2>/dev/null
+ 	then	if	grep \'### .*archaic.* ###\' >/dev/null
+ 		then	: bin/AAA contains archaic constructs :
+ 		else	ENV= LC_ALL=C $SHELL -n AAA.sh
+ 		fi
+ 	fi
+ 	;;
+ esac
+ case \'\' in
+ "")	case 0 in
+ 	0)	cp AAA.sh bin/AAA
+ 		;;
+ 	*)	{
+ 		i=`(read x; echo $x) < AAA.sh`
+ 		case $i in
+ 		\'#!\'*|*\'||\'*|\':\'*|\'":"\'*|"\':\'"*)	echo "$i" ;;
+ 		esac
+ 		cat - AAA.sh <<\'!\'
+ 
+ !
+ 		} > bin/AAA
+ 		;;
+ 	esac
+ 	;;
+ *)	cat - AAA.sh > bin/AAA <<\'!\'
+ 
+ 
+ !
+ 	;;
+ esac
+ silent test -w bin/AAA -a -x bin/AAA || chmod u+w,+x bin/AAA
+ case message:$OPTIND:$RANDOM in
+ ?*:*:*|*::*|*:*:$RANDOM)
+ 	;;
+ *)	if	ENV= LC_ALL=C x= $SHELL -nc \'[[ a || b ]] && : ${list[level]} $(( 1 + $x )) !(pattern)\' 2>/dev/null
+ 	then	if	grep \'### .*archaic.* ###\' >/dev/null
+ 		then	: BBB contains archaic constructs :
+ 		else	ENV= LC_ALL=C $SHELL -n BBB.sh
+ 		fi
+ 	fi
+ 	;;
+ esac
+ case \'\' in
+ "")	case 0 in
+ 	0)	cp BBB.sh BBB
+ 		;;
+ 	*)	{
+ 		i=`(read x; echo $x) < BBB.sh`
+ 		case $i in
+ 		\'#!\'*|*\'||\'*|\':\'*|\'":"\'*|"\':\'"*)	echo "$i" ;;
+ 		esac
+ 		cat - BBB.sh <<\'!\'
+ 
+ !
+ 		} > BBB
+ 		;;
+ 	esac
+ 	;;
+ *)	cat - BBB.sh > BBB <<\'!\'
+ 
+ 
+ !
+ 	;;
+ esac
+ silent test -w BBB -a -x BBB || chmod u+w,+x BBB'

TEST 21 ':JOINT: + update'

	EXEC
		INPUT Makefile $'SMGEN = smgen
SMGEN_FLAGS = -k -s printf
FILES = color.sm traffic.c
traffic : $(FILES:B:S=.o)
	cc -o $(<) $(*)
color.c color.h ColorSM.h :JOINT: color.sm (SMGEN) (SMGEN_FLAGS)
	if [ "$(>)" != "" ]
	then
	    $(SMGEN) $(SMGEN_FLAGS) $(>)
	else
	    silent echo $(<) are up to date with $(*)
	fi'
		INPUT -x smgen $'if [ -f color.c ]
then
echo color.c is up-to-date
else
echo making color.
cp color.sm color.c
fi
if [ -f color.h ]
then
echo color.h is up-to-date
else
echo making color.h
echo "#define ONE 1" > color.h
fi
echo making ColorSM.h
echo "#define TWO 2" > ColorSM.h'
		INPUT traffic.c $'#include "Kolor.h"
int main()
{
	hello();
	return 0;
}'
		INPUT Kolor.h $'#include "ColorSM.h"
#include "color.h"'
		INPUT color.sm $'#include "Kolor.h"
hello()
{}'
		OUTPUT - $'making color.
making color.h
making ColorSM.h'
		ERROR - $'+ [ color.sm \'!=\' \'\' ]
+ smgen -k -s printf color.sm
+ cc -O -I. -c color.c
+ cc -O -I. -c traffic.c
+ cc -o traffic color.o traffic.o'

	EXEC	--regress=sync
		OUTPUT -
		ERROR -

	DO	touch color.sm

	EXEC
		OUTPUT - $'color.c is up-to-date
color.h is up-to-date
making ColorSM.h'
		ERROR - $'+ [ color.sm \'!=\' \'\' ]
+ smgen -k -s printf color.sm
+ cc -O -I. -c color.c
+ cc -O -I. -c traffic.c
+ cc -o traffic color.o traffic.o'

TEST 22 ':JOINT: + update + views'

	EXPORT	INSTALLROOT=..
	EXPORT	VPATH=$TWD/dev:$TWD/ofc

	CD	ofc/src

	EXEC	install
		INPUT Makefile $'DBHDRS = DB1.h DB2.h
CONV = conv
$(DBHDRS) :JOINT: dbsrc (CONV)
	$(CONV:T=F) $(*)
$(INSTALLROOT)/lib/db :INSTALLDIR: $(DBHDRS)'
		INPUT -x conv $'cp $1 DB1.h
cp $1 DB2.h'
		INPUT dbsrc $'db stuff'
		ERROR - $'+ mkdir -p ../lib/db
+ conv dbsrc
+ ignore cp DB1.h ../lib/db/DB1.h
+ ignore cp DB2.h ../lib/db/DB2.h'

	CD	../../dev/src

	EXEC	install
		ERROR - $'+ mkdir -p ../lib/db'

	EXEC	-n clobber
		OUTPUT - $'+ ignore rm -f -r  Makefile.ms'
		ERROR -

	EXEC	-n clobber.install
		OUTPUT - $'+ ignore rm -f -r '

	CD	../../ofc/src

	EXEC	-n clobber
		OUTPUT - $'+ ignore rm -f -r  DB2.h DB1.h Makefile.mo Makefile.ms'
		ERROR -

	EXEC	-n clobber.install
		OUTPUT - $'+ ignore rm -f -r ../lib/db/DB1.h ../lib/db/DB2.h'

TEST 23 ':MAKE: + clobber'

	EXEC
		INPUT Makefile $':MAKE:'
		INPUT a/Makefile $'a : a.src\n\t$(CP) $(*) $(<)'
		INPUT a/a.src
		INPUT b/Makefile $'b : b.src\n\t$(CP) $(*) $(<)'
		INPUT b/b.src
		ERROR - $'a:
+ cp a.src a
b:
+ cp b.src b'

	EXEC
		ERROR - $'a:
b:'

	EXEC	-n clobber
		OUTPUT - $'+ ignore rm -f -r  a Makefile.mo Makefile.ms
+ ignore rm -f -r  b Makefile.mo Makefile.ms
+ ignore rm -f -r  Makefile.mo Makefile.ms'

	EXEC	-n recurse clobber

TEST 24 ':MAKE: with lhs and rhs'

	EXEC	--novirtual
		INPUT Makefile $'all : d1 d2
d1 :MAKE: a
d2 :MAKE: b'
		ERROR - $'a: warning: cannot recurse on virtual directory
b: warning: cannot recurse on virtual directory'

	EXEC	--
		ERROR - $'a: warning: cannot recurse on virtual directory
b: warning: cannot recurse on virtual directory'

	EXEC	--novirtual

	EXEC	--
		INPUT a/Makefile $'all :\n\t: $(PWD:B)'
		INPUT b/Makefile $'all :\n\t: $(PWD:B)'
		ERROR - $'a:
+ : a
b:
+ : b'
		EXIT 0

	EXEC	d1
		ERROR - $'a:
+ : a'

	EXEC	d2
		ERROR - $'b:
+ : b'

TEST 25 ':: with command/library name clash'

	EXEC	-n
		INPUT Makefile $'t :: t.c -lt'
		INPUT t.c $'int main(){return 0;}'
		INPUT libt.a
		OUTPUT - $'+ cc -O   -c t.c
+ cc  -O   -o t t.o libt.a'

TEST 26 ':LIBRARY: + version + options'

	EXPORT	INSTALLROOT=.

	EXEC	--regress=sync all VERSION=2.4
		INPUT Makefile $'VERSION = 1.0
CCFLAGS += $$(CC.DLL)
t $(VERSION) :LIBRARY: a.c z.c'
		INPUT a.c $'int a() { return 0; }'
		INPUT z.c $'int z() { return 0; }'
		ERROR - $'+ echo \'\' -lt
+ 1> t.req
+ cc -O -D_BLD_DLL -D_BLD_PIC -c a.c
+ cc -O -D_BLD_DLL -D_BLD_PIC -c z.c
+ ar cr libt.a a.o z.o
+ ignore ranlib libt.a
+ rm -f a.o z.o
+ cc -shared -o libt.so.2.4 -all libt.a -notall'

	EXEC	--regress=sync all VERSION=2.4
		ERROR -

	EXEC	--regress=sync all VERSION=5.0
		ERROR - $'+ cc -shared -o libt.so.5.0 -all libt.a -notall'

	EXEC	--regress=sync all VERSION=5.0
		ERROR -

	EXEC	--regress=sync clobber
		ERROR - $'+ ignore rm -f -r libt.so.2.4 libt.a Makefile.mo Makefile.ms t.req libt.so.5.0'

	EXEC	--regress=sync install VERSION='7.1 plugin=foo'
		ERROR - $'+ cc -O -D_BLD_DLL -D_BLD_PIC -c a.c
+ cc -O -D_BLD_DLL -D_BLD_PIC -c z.c
+ ar cr libfoot.a a.o z.o
+ ignore ranlib libfoot.a
+ rm -f a.o z.o
+ mkdir -p lib/foo
+ cc -shared -o libt.so.7.1 -all libfoot.a -notall
+ /bin/cp libt.so.7.1 lib/foo/libt.so.7.1
+ /bin/ln lib/foo/libt.so.7.1 lib/foo/libt.so
+ chmod -w lib/foo/libt.so.7.1'

	EXEC	--regress=sync install VERSION='7.1 plugin=foo'
		ERROR -

TEST 27 ':MAKE: + install + file name clash'

	EXEC	-n install
		INPUT Makefile $':MAKE: xinstall'
		INPUT xinstall/Makefile $':MAKE: grand'
		INPUT xinstall/grand/Makefile $'bin :INSTALLDIR: t.exe'
		INPUT xinstall/grand/t.exe
		OUTPUT - $'+ if	silent test ! -d bin
+ then	mkdir -p bin 		    		   
+ fi
+ if	silent test \'\' != "t.exe"
+ then	if	silent test \'\' != ""
+ 	then	: bin/t.exe linked to t.exe
+ 	elif	silent test -d "t.exe"
+ 	then	cp -pr t.exe bin
+ 	else	silent cmp -s t.exe bin/t.exe ||
+ 		{
+ 		if	silent test -f "bin/t.exe"
+ 		then	{ mv -f bin/t.exe bin/t.exe.old || { test -f bin/t.exe && ignore rm -f bin/t.exe.old* && mv -f bin/t.exe `echo bin/t.exe.old* | sed -e \'s/.* //\' -e \'s/old\\(z*\\)$/old\\1z/\' -e \'s/\\*$//\'`; }; }
+ 		fi
+ 		ignore cp t.exe bin/t.exe  		    		   
+ 		}
+ 	fi
+ fi'
		ERROR - $'xinstall:
xinstall/grand:'

	EXEC	-n recurse install

	EXEC	-n .INSTALL
		INPUT Makefile $':MAKE: install'
		INPUT install/Makefile $':MAKE: grand'
		INPUT install/grand/Makefile $'bin :INSTALLDIR: t.exe'
		INPUT install/grand/t.exe
		ERROR - $'install:
install/grand:'

	EXEC	-n install
		OUTPUT - $'+ if	silent test ! -d bin
+ then	mkdir -p bin 		    		   
+ fi
+ if	silent test \'\' != "t.exe"
+ then	if	silent test \'\' != ""
+ 	then	: bin/t.exe linked to t.exe
+ 	elif	silent test -d "t.exe"
+ 	then	cp -pr t.exe bin
+ 	else	silent cmp -s t.exe bin/t.exe ||
+ 		{
+ 		if	silent test -f "bin/t.exe"
+ 		then	{ mv -f bin/t.exe bin/t.exe.old || { test -f bin/t.exe && ignore rm -f bin/t.exe.old* && mv -f bin/t.exe `echo bin/t.exe.old* | sed -e \'s/.* //\' -e \'s/old\\(z*\\)$/old\\1z/\' -e \'s/\\*$//\'`; }; }
+ 		fi
+ 		ignore cp t.exe bin/t.exe  		    		   
+ 		}
+ 	fi
+ fi'

TEST 28 ':INSTALLDIR: + mode'

	EXEC	-n install
		INPUT Makefile $':ALL:
FILES = a
BINDIR = bin
$$(BINDIR) :INSTALLDIR: mode=0644 $(FILES)'
		INPUT a
		OUTPUT - $'+ if	silent test ! -d bin
+ then	mkdir -p bin 		    		   
+ fi
+ if	silent test \'\' != "a"
+ then	if	silent test \'\' != ""
+ 	then	: bin/a linked to a
+ 	elif	silent test -d "a"
+ 	then	cp -pr a bin
+ 	else	silent cmp -s a bin/a ||
+ 		{
+ 		if	silent test -f "bin/a"
+ 		then	{ mv -f bin/a bin/a.old || { test -f bin/a && ignore rm -f bin/a.old* && mv -f bin/a `echo bin/a.old* | sed -e \'s/.* //\' -e \'s/old\\(z*\\)$/old\\1z/\' -e \'s/\\*$//\'`; }; }
+ 		fi
+ 		ignore cp a bin/a  		    		   && chmod 0644 bin/a
+ 		}
+ 	fi
+ fi'

TEST 29 ':INSTALLDIR: with a view'

	EXPORT	VPATH=$TWD/dev:$TWD/ofc

	CD	ofc

	EXEC	install
		INPUT Makefile $'.SOURCE : src
INSTALLROOT=.
$(BINDIR) :INSTALLDIR: prog1
$(LIBDIR) :INSTALLDIR: prog2
$(INCLUDEDIR) :INSTALLDIR: prog3'
		INPUT src/prog1
		INPUT src/prog2
		INPUT src/prog3
		ERROR - $'+ mkdir -p bin
+ ignore cp src/prog1 bin/prog1
+ mkdir -p lib
+ ignore cp src/prog2 lib/prog2
+ mkdir -p include
+ ignore cp src/prog3 include/prog3'

	CD	../dev

	EXEC	install
		ERROR - $'+ mkdir -p bin
+ mkdir -p lib
+ mkdir -p include'

TEST 30 'install + .INSTALL.%.attribute + :INSTALLMAP:'

	EXEC	-n install
		INPUT Makefile $'INSTALLROOT = .
TBINDIR = $(INSTALLROOT)/tbin
.TBIN : .ATTRIBUTE
.INSTALL.%.TBIN : $$(TBINDIR)
t :: t.c .TBIN'
		INPUT t.c $'int main(){return 0;}'
		OUTPUT - $'+ cc -O   -c t.c
+ cc  -O   -o t t.o
+ if	silent test ! -d tbin
+ then	mkdir -p tbin 		    		   
+ fi
+ if	silent test \'\' != "t"
+ then	if	silent test \'\' != ""
+ 	then	: tbin/t linked to t
+ 	elif	silent test -d "t"
+ 	then	cp -pr t tbin
+ 	else	silent cmp -s t tbin/t ||
+ 		{
+ 		if	silent test -f "tbin/t"
+ 		then	{ mv -f tbin/t tbin/t.old || { test -f tbin/t && ignore rm -f tbin/t.old* && mv -f tbin/t `echo tbin/t.old* | sed -e \'s/.* //\' -e \'s/old\\(z*\\)$/old\\1z/\' -e \'s/\\*$//\'`; }; }
+ 		fi
+ 		ignore cp t tbin/t  		    		   
+ 		}
+ 	fi
+ fi'

	EXEC	-n install
		INPUT Makefile $'INSTALLROOT = .
TBINDIR = $(INSTALLROOT)/tbin
.TBIN : .ATTRIBUTE
$(TBINDIR) :INSTALLMAP: A=.TBIN
t :: t.c .TBIN'
		OUTPUT - $'+ cc -O   -c t.c
+ cc  -O   -o t t.o
+ if	silent test ! -d tbin
+ then	mkdir -p tbin 		    		   
+ fi
+ if	silent test \'\' != "t.o"
+ then	if	silent test \'\' != ""
+ 	then	: tbin/t.o linked to t.o
+ 	elif	silent test -d "t.o"
+ 	then	cp -pr t.o tbin
+ 	else	silent cmp -s t.o tbin/t.o ||
+ 		{
+ 		if	silent test -f "tbin/t.o"
+ 		then	{ mv -f tbin/t.o tbin/t.o.old || { test -f tbin/t.o && ignore rm -f tbin/t.o.old* && mv -f tbin/t.o `echo tbin/t.o.old* | sed -e \'s/.* //\' -e \'s/old\\(z*\\)$/old\\1z/\' -e \'s/\\*$//\'`; }; }
+ 		fi
+ 		ignore cp t.o tbin/t.o  		    		   
+ 		}
+ 	fi
+ fi
+ if	silent test \'\' != "t"
+ then	if	silent test \'\' != ""
+ 	then	: tbin/t linked to t
+ 	elif	silent test -d "t"
+ 	then	cp -pr t tbin
+ 	else	silent cmp -s t tbin/t ||
+ 		{
+ 		if	silent test -f "tbin/t"
+ 		then	{ mv -f tbin/t tbin/t.old || { test -f tbin/t && ignore rm -f tbin/t.old* && mv -f tbin/t `echo tbin/t.old* | sed -e \'s/.* //\' -e \'s/old\\(z*\\)$/old\\1z/\' -e \'s/\\*$//\'`; }; }
+ 		fi
+ 		ignore cp t tbin/t  		    		   
+ 		}
+ 	fi
+ fi'

TEST 31 'nested assertions'

	EXEC	-n all
		INPUT Makefile $'":GRANTPROC:" : .OPERATOR .MAKE
	$(<) : $(>) .VIRTUAL .IGNORE
		: grant exec on $(~)
":MAKESTOREDPROC:" : .OPERATOR .MAKE
	$(<) : $(>) .VIRTUAL
		: make the stored procedure $(<) from $(*)
":STOREDPROC:" : .OPERATOR .MAKE
	local I V
	V := $(>:N=*=*)
	for I $(>:N=*.sp)
		.ALL : $(I:D:B)
		$(I:D:B:S=.grant) :GRANTPROC: $(I:D:B) $(V)
		$(I:D:B) :MAKESTOREDPROC: $(I) $(I:D:B:S=.grant)
	end
:STOREDPROC: sp1.sp sp2.sp GRANT=public
:STOREDPROC: sp3.sp sp4.sp GRANT=lagerto'
		INPUT sp1.sp
		INPUT sp2.sp
		INPUT sp3.sp
		INPUT sp4.sp
		OUTPUT - $'+ : grant exec on sp1 GRANT=public
+ : make the stored procedure sp1 from sp1.sp
+ : grant exec on sp2 GRANT=public
+ : make the stored procedure sp2 from sp2.sp
+ : grant exec on sp3 GRANT=lagerto
+ : make the stored procedure sp3 from sp3.sp
+ : grant exec on sp4 GRANT=lagerto
+ : make the stored procedure sp4 from sp4.sp'

TEST 32 ':MAKE: + ..'

	CD	work

	EXEC	-n
		INPUT Makefile $':MAKE: \\\
	../src/web/jsp \\\
	../src/web/WEB-INF \\\
	../src/java/cr'
		INPUT ../src/web/jsp/Makefile $'all:\n\t: $(PWD)'
		INPUT ../src/web/WEB-INF/Makefile $'all:\n\t: $(PWD)'
		INPUT ../src/java/cr/Makefile $'all:\n\t: $(PWD)'
		OUTPUT - $'+ : '$TWD$'/src/web/jsp
+ : '$TWD$'/src/web/WEB-INF
+ : '$TWD$'/src/java/cr'
		ERROR - $'../src/web/jsp:
../src/web/WEB-INF:
../src/java/cr:'

TEST 33 ':JOINT: vs cc'

	EXEC
		INPUT Makefile $'all : b.o a.o
b.o a.o :JOINT: j.c
	$(CC) -c $(*)
	$(CP) $(*:B:S=.o) $(<:O=1)
	$(MV) $(*:B:S=.o) $(<:O=2)'
		INPUT j.c $'int a() { return 0; }'
		ERROR - $'+ cc -c j.c
+ cp j.o b.o
+ mv j.o a.o'

	EXEC
		ERROR -

TEST 34 ':JOINT: vs :LIBRARY:'

	EXEC	--regress=sync
		INPUT Makefile $'tst :LIBRARY: b.o a.o
b.o a.o :JOINT: fun.c
	$(CC) -DFUN=$(<:O=1:B) -c $(*)
	$(MV) $(*:B:S=.o) $(<:O=1)
	$(CC) -DFUN=$(<:O=2:B) -c $(*)
	$(MV) $(*:B:S=.o) $(<:O=2)'
		INPUT fun.c $'int FUN() { return 0; }'
		ERROR - $'+ echo \'\' -ltst
+ 1> tst.req
+ cc \'-DFUN=b\' -c fun.c
+ mv fun.o b.o
+ cc \'-DFUN=a\' -c fun.c
+ mv fun.o a.o
+ ar cr libtst.a b.o a.o
+ ignore ranlib libtst.a
+ rm -f b.o a.o'

	EXEC
		ERROR

TEST 35 ':LIBRARY: + :VARIANT: + cc-'

	EXEC	-n all cc-
		INPUT Makefile $'CCFLAGS = $(CC.OPTIMIZE) $(CC.DLL)
libraryC$(VARIANTID) :LIBRARY: a.c b.c c.c -lws2_32
++ :VARIANT:
	CC = CC
2 :VARIANT:
	CCFLAGS += -DFOO=bar'
			INPUT a.c
			INPUT b.c
			INPUT c.c
			OUTPUT - $'+ echo "" -llibraryC > libraryC.req
+ cc -O -D_BLD_DLL -D_BLD_PIC   -c a.c
+ cc -O -D_BLD_DLL -D_BLD_PIC   -c b.c
+ cc -O -D_BLD_DLL -D_BLD_PIC   -c c.c
+ ar  cr liblibraryC.a a.o b.o c.o
+ ignore ranlib liblibraryC.a
+ rm -f a.o b.o c.o
+ cc  -shared  -o liblibraryC.so.1.0 -all liblibraryC.a -notall 
+ echo "" -llibraryC++ > libraryC++.req
+ CC -O -D_BLD_DLL -D_BLD_PIC   -c ../a.c
+ CC -O -D_BLD_DLL -D_BLD_PIC   -c ../b.c
+ CC -O -D_BLD_DLL -D_BLD_PIC   -c ../c.c
+ ar  cr liblibraryC++.a a.o b.o c.o
+ ignore ranlib liblibraryC++.a
+ rm -f a.o b.o c.o
+ cc  -shared  -o liblibraryC++.so.1.0 -all liblibraryC++.a -notall 
+ echo "" -llibraryC2 > libraryC2.req
+ cc -O -D_BLD_DLL -D_BLD_PIC -DFOO=bar   -c ../a.c
+ cc -O -D_BLD_DLL -D_BLD_PIC -DFOO=bar   -c ../b.c
+ cc -O -D_BLD_DLL -D_BLD_PIC -DFOO=bar   -c ../c.c
+ ar  cr liblibraryC2.a a.o b.o c.o
+ ignore ranlib liblibraryC2.a
+ rm -f a.o b.o c.o
+ cc  -shared  -o liblibraryC2.so.1.0 -all liblibraryC2.a -notall '
			ERROR - $'cc-++:\ncc-2:'

TEST 36 ':MAKE: with *.mk rhs'

	PROG	mkdir dir

	EXEC	-n
		INPUT Makefile $':MAKE: foo.mk'
		INPUT foo.mk $'all :\n\t: foo :'
		INPUT dir/Makefile $'all :\n\t: dir :'
		OUTPUT - $'+ : foo :'
		ERROR - $'foo.mk:'
	EXEC	-n all

	EXEC	-n
		INPUT Makefile $':MAKE: dir'
		OUTPUT - $'+ : dir :'
		ERROR - $'dir:'
	EXEC	-n all

	EXEC	-n
		INPUT Makefile $':MAKE: foo.mk - dir'
		OUTPUT - $'+ : foo :\n+ : dir :'
		ERROR - $'foo.mk:\ndir:'
	EXEC	-n all
