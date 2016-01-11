# ast nmake implicit prereq scan tests

INCLUDE cc.def

TEST 01 'static source'

	EXEC -n
		INPUT Makefile $'.SOURCE.h : include
t :: t.c'
		INPUT t.c $'#include "t.h"'
		INPUT include/t.h 
		OUTPUT - $'+ cc -O -Iinclude  -c t.c
+ cc  -O   -o t t.o'

TEST 02 'generated source'

	EXEC	-n
		INPUT Makefile $'.SOURCE.h : include
t :: t.c
t.c : gen .ALWAYS
	echo \'#include "t.h"\' > $(<)'
		INPUT gen
		ERROR - $'+ echo \'#include "t.h"\'
+ 1> t.c
make: don\'t know how to make t : t.o : t.c : t.h'
		EXIT 1

	EXEC	-n
		INPUT include/t.h 
		OUTPUT - $'+ cc -O -Iinclude  -c t.c
+ cc  -O   -o t t.o'
		ERROR - $'+ echo \'#include "t.h"\'
+ 1> t.c'
		EXIT 0

	EXEC -n
		INPUT include/t.h $'#if DEBUG
#include "u.h"
#endif'

TEST 03 'ignored implicits'

	EXEC
		INPUT Makefile $'.SOURCE.%.SCAN.ec : .FORCE $$(*.SOURCE.ec) $$(*.SOURCE) $$(*.SOURCE.eh)
.SCAN.ec : .SCAN
	I| $ include "%" ; |
.ATTRIBUTE.%.ec : .SCAN.ec
ECFLAGS &= $$(.INCLUDE. ec -I)
%.c : %.ec
	: $(!) : $(!$(>)) :
	: $(ECFLAGS) :
	sed -e '/^\$include/d' $(*) > $(<)
%.o : %.c
	: $(!) > $(<)
all : x.o
x.eh : .IGNORE'
		INPUT x.ec $'$ include "x.eh" ;
#include "x.h"
main(){}'
		INPUT x.eh
		INPUT x.h
		ERROR - $'+ : x.ec : :
+ : :
+ sed -e /^/d x.ec
+ 1> x.c
+ : x.c
+ 1> x.o'

	EXEC
		ERROR -

	DO	touch x.eh

	EXEC

TEST 04 'strategy exercise'

	EXEC	-n
		INPUT Makefile $'.IMPORT : .FUNCTION
	if import
		import = 0
		return $(%%)
	end
.SCAN.x : .SCAN
	OS
	Q|/*|*/||C|
	Q|//||\\\\|LC|
	Q|"|"|\\\\|LQ|
	Q|\'|\'|\\\\|LQ|
	Q|\\\\|||CS|
	I| import: @ "%" , |Rimport=1|
	I| "%" ; |M$$(.IMPORT)|
.ATTRIBUTE.%.x : .SCAN.x
x : t.x
	: $(<) : $(*) : $(!) :'
		INPUT t.x $'"p.x",
"q.x";
import:
"inc/a.x",
"b.x",
"c.x",
"d.x";
"y.x",
"z.x";'
		INPUT inc/a.x $'q.x
#import
m.x
#endimport
z.x'
		INPUT b.x
		INPUT c.x
		INPUT z.x
		OUTPUT - $'+ : x : t.x : t.x z.x c.x b.x inc/a.x :'

TEST 05 'm4 scan'

	EXEC	-n
		INPUT Makefile $'.M4.INCLUDE : .ATTRIBUTE
.SCAN.x : .SCAN
	OM
	I|%|A.M4.INCLUDE|
.ATTRIBUTE.%.m4 : .SCAN.x
x : x.m4
	: $(!)'
		INPUT x.m4 $'include(y.m4)
xxx'
		INPUT y.m4 $'yyy'
		OUTPUT - $'+ : x.m4 y.m4'

TEST 06 'nroff scan'

	EXEC	-n
		INPUT Makefile $'.SCAN.nroff.INDIRECT : .FUNCTION
	if "$(%)" == "$[A-Za-z_]*([A-Za-z_0-9])"
		$(<<) : ($(%:/.//))
		return $($(%:/.//))
	end
	return $(%)
.SCAN.nroff : .SCAN
	I|.so % |M$$(.SCAN.nroff.INDIRECT $$(%))|A.DONTCARE|
	I|,so % |M$$(.SCAN.nroff.INDIRECT $$(%))|A.DONTCARE|
	I|.BP % |M$$(%:/\([\-,0-9]*\)$//)|A.DONTCARE|
	I|,BP % |M$$(%:/\([\-,0-9]*\)$//)|A.DONTCARE|
.ATTRIBUTE.%.mm : .SCAN.nroff
%.ps : %.mm
	$(&:T=E) troff -Tps $(>) | dpost > $(<)
FIGURE = t.fig
all : t.ps'
		INPUT t.mm $'.so $FIGURE'
		OUTPUT - $'+ FIGURE=t.fig troff -Tps t.mm | dpost > t.ps'

TEST 07 'USAGE state variable scan'

	EXEC	-n
		INPUT Makefile $'LICENSEFILE = regress.lic
USAGE == "$(LICENSEFILE:T=F:P=W=type=usage,$(LICENSE):/1999-..../1999-2004/)"
t :: t.c LICENSE=since=1999,author=bu'
		INPUT regress.lic $'message_set=3
contributor=(
	[bd]="Ben Dover <bd@regress.com>"
	[bu]="Bob Ushka <bu@regress.com>"
)
license=(
	type=open
	package=regress
	since=1986
	author=bd+bu
	organization="Progressive Regressioneers Incorporated"
	domain=regress.com
	corporation="PRI"
	location="Washington DC"
	url=http://www.${license.domain}/sw/license/${license.package}-${license.type}.html
	bin=http://www.${license.domain}/sw/license/${license.package}-${license.type}-bin.html
	notice=\'
This is a test.
\'
)'
		INPUT t.c $'static char usage[] = "YADA" USAGE "YADA";'
		OUTPUT - $'+ cc -O  -DUSAGE=\\""[-author?Bob Ushka <bu@regress.com>][-copyright?Copyright (c) 1999-2004 PRI][-license?http://www.regress.com/sw/license/regress-open.html]"\\" -c t.c
+ cc  -O   -o t t.o'

TEST 08 '.PARAMETER'

	EXEC	-n
		INPUT Makefile $'x.G : .SCAN.c .PARAMETER .SPECIAL
.ATTRIBUTE.%.G : .TERMINAL
all : x.G
	: $(*) : "$(?$(*))" :'
		INPUT x.G $'#define a 1
#define z 2'
		OUTPUT - $'+ : x.G : "(z) (a)" :'

TEST 09 'ar scan'

	EXEC	--regress=sync
		INPUT Makefile $'
tst :LIBRARY: \\
	12345678901.c \\
	123456789012.c \\
	1234567890123.c \\
	12345678901234.c \\
	123456789012345.c \\
	1234567890123456.c \\
	12345678901234567.c \\
	123456789012345678.c \\
	1234567890123456789.c'
		INPUT 12345678901.c $'int f_12345678901(){return 0;}'
		INPUT 123456789012.c $'int f_123456789012(){return 0;}'
		INPUT 1234567890123.c $'int f_1234567890123(){return 0;}'
		INPUT 12345678901234.c $'int f_12345678901234(){return 0;}'
		INPUT 123456789012345.c $'int f_123456789012345(){return 0;}'
		INPUT 1234567890123456.c $'int f_1234567890123456(){return 0;}'
		INPUT 12345678901234567.c $'int f_12345678901234567(){return 0;}'
		INPUT 123456789012345678.c $'int f_123456789012345678(){return 0;}'
		INPUT 1234567890123456789.c $'int f_1234567890123456789(){return 0;}'
		ERROR - $'+ echo \'\' -ltst
+ 1> tst.req
+ cc -O -c 12345678901.c
+ cc -O -c 123456789012.c
+ cc -O -c 1234567890123.c
+ cc -O -c 12345678901234.c
+ cc -O -c 123456789012345.c
+ cc -O -c 1234567890123456.c
+ cc -O -c 12345678901234567.c
+ cc -O -c 123456789012345678.c
+ cc -O -c 1234567890123456789.c
+ ar cr libtst.a 12345678901.o 123456789012.o 1234567890123.o 12345678901234.o 123456789012345.o 1234567890123456.o 12345678901234567.o 123456789012345678.o 1234567890123456789.o
+ ignore ranlib libtst.a
+ rm -f 12345678901.o 123456789012.o 1234567890123.o 12345678901234.o 123456789012345.o 1234567890123456.o 12345678901234567.o 123456789012345678.o 1234567890123456789.o'

	EXEC	--regress=sync
		ERROR -

TEST 10 'f77/c crossover'

	EXEC	-n t.o
		INPUT Makefile $'.SOURCE.%.SCAN.f : $$(*.SOURCE.h)
.SOURCE.h : hdr
F77FLAGS += $$(.INCLUDE. f -i)
t :: t.f'
		INPUT t.f $'include \'t.h\''
		INPUT hdr/t.h
		OUTPUT - $'+ f77 -ihdr -c t.f'

	EXEC	-n t.o
		INPUT Makefile $'.SOURCE.h : hdr
.ATTRIBUTE.%.f : .SCAN.c
F77FLAGS += $$(CCFLAGS:N=-I*)
t :: u.f'
		INPUT t.f $'#include "t.h"'
		OUTPUT - $'+ f77 -Ihdr -c t.f'

TEST 11 'start and finish actions'

	EXEC	-n
		INPUT Makefile $'.M4.INTERFACE : .ATTRIBUTE
.SDL.INTERFACE : .MAKE  .VIRTUAL .FORCE .REPEAT
	error 0 SDL INTERFACE
.ACCEPT .IGNORE .RETAIN : .M4.INTERFACE
.SCAN.G : .SCAN
	S|Rerror 0 start $$(<) scan : $$(?) :|
	F|Rerror 0 finish $$(<) scan : $$(?) :|
	T| \\# M4|M.M4.INTERFACE|
	T| \\# SDL|M.SDL.INTERFACE|
.ATTRIBUTE.%.G : .SCAN.G
f.h : f.G
	cp $(*) $(<)'
		INPUT f.G $'#M4
xxx
#end

#SDL
xxx
#end'
		OUTPUT - $'+ cp f.G f.h'
		ERROR - $'start f.G scan :  :
finish f.G scan : .SDL.INTERFACE .M4.INTERFACE :
SDL INTERFACE'

TEST 12 'multi-line implicit prereq include'

	EXEC	--regress=sync --noexec --force --mam=static
		INPUT Makefile $'.SCAN.x : .SCAN
	OS
	Q|/*|*/||C|
	Q|//||\\\\|LC|
	Q|"|"|\\\\|LQ|
	Q|\'|\'|\\\\|LQ|
	Q|\\\\|||CS|
	I| \\# import @ % @ \\# endimport |M$$(.PREFIX.INCLUDE.)
.ATTRIBUTE.%.x : .SCAN.x
x : t.x'
		INPUT b.x
		INPUT c.x
		INPUT inc/a.x $'q.x
#import
m.x
#endimport
z.x'
		INPUT inc/m.x
		INPUT t.x $'q.x
#import
inc/a.x
b.x
c.x
#endimport
z.x'
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
make x
make t.x
make c.x implicit
done c.x
make b.x implicit
done b.x
make inc/a.x implicit
make inc/m.x implicit
done inc/m.x
done inc/a.x
done t.x
done x virtual'

TEST 13 'scan trace and side effects'

	EXEC	-n
		INPUT Makefile $'.SCAN.tom : .SCAN
	I|* name %|Rprint found $$(%)
.ATTRIBUTE.%.tom : .SCAN.tom
x : x.tom
	$(*) : $(!)
p1 p2 p3 p4 p5 p6 p7 p8 p9 :
	prereq $(<)'
		INPUT x.tom $'1234n 567 name p1
1234x 567 name p2
name p3
Name name p4'
		OUTPUT - $'found p1
found p2
found p3
found p4
+ prereq p4
+ prereq p3
+ prereq p2
+ prereq p1
+ x.tom : x.tom p4 p3 p2 p1'

TEST 14 'include "messages"'

	EXEC	-n
		INPUT Makefile $'cmd :: cmd.c'
		INPUT cmd.c $'#ifdef xxx
#include "error -- this is a test"
#endif
main(){}'
		INPUT error.c
		OUTPUT - $'+ cc -O   -c cmd.c
+ cc  -O   -o cmd cmd.o'

TEST 15 '.SCAN.nroff'

	EXEC	-n
		INPUT Makefile $'t : t.nroff
	: $(!:H) :
.ATTRIBUTE.%.nroff : .SCAN.nroff'
		INPUT t.nroff $'.BP a.nroff
.BP b.nroff(3) 1 2 3 4
,BP c.nroff 5 6 7 8
,BP d.nroff(4) 9 8 7 6'
		INPUT a.nroff
		INPUT b.nroff
		INPUT c.nroff
		INPUT d.nroff
		OUTPUT - $'+ : a.nroff b.nroff c.nroff d.nroff t.nroff :'

TEST 16 '.SCAN.sql'

	EXEC	-n
		INPUT Makefile $'.BIND.%.SCAN.sql : .FUNCTION
	if "$(%:N!=*.h)"
		return $(%).h
	end
.SCAN.sql : .SCAN
	Q|/*|*/||C|
	Q|//||\\\\|LC|
	Q|"|"|\\\\|LQ|
	Q|\'|\'|\\\\|LQ|
	Q|\\\\|||CS|
	I| EXEC SQL include "%"|
	I| EXEC SQL include % |M$$(%:/;$//)|
	I| \\## include "%"|
	I| \\## include % |M$$(%:/;$//)|
	I| $ include <%> |
	I| $ include "%" |
	I| $ include \'%\' |
	I| $ include % |M$$(%:/;$//)|
.ATTRIBUTE.%.sql : .SCAN.sql 
all : test.sql
	: $(!:H) :'
		INPUT test.sql $'$include x;'
		INPUT x.h $'test;'
		OUTPUT - $'+ : test.sql x.h :'

TEST 17 '.ATTRIBUTE propagation'

	EXEC	-n
		INPUT Makefile $'.MESSAGE.ID : .ATTRIBUTE
.SCAN.G : .SCAN
	O|S|
	Q|/*|*/||C|
	Q|//||\\\\|LC|
	Q|"|"|\\\\|LQ|
	Q|\'|\'|\\\\|LQ|
	Q|\\\\|||CS|
	B| \\# if|
	E| \\# endif|
	T|message |A.MESSAGE.ID|
	T| *||
.ATTRIBUTE.%.G : .SCAN.G
all : .MAKE n.G y.G
	local X
	for X $(*)
		print : $(X) : $(X:A) : $(!$(X)) :
	end'
		INPUT n.G $'message_t  statements = {
        "first message",
        "second message"
};'
		INPUT y.G $'message "it\'s a hit"'
		OUTPUT - $': n.G : .SCAN.G :  :
: y.G : .MESSAGE.ID .SCAN.G :  :'

TEST 18 '.SCAN today gone tomorrow'

	EXEC	--regress=sync ASCAN=0
		INPUT Makefile $'if ASCAN
.SCAN.aaa : .SCAN
	OS
.ATTRIBUTE.%.aaa : .SCAN.aaa
end
.SCAN.bbb : .SCAN
	OS
.ATTRIBUTE.%.bbb : .SCAN.bbb
AAA == 1
BBB == 2
all : .MAKE a b
	query $(!$(*))
gen : .USE
	$(CP) $(*) $(<)
a : gen a.aaa
b : gen b.bbb'
		INPUT a.aaa $'AAA'
		INPUT b.bbb $'BBB'
		ERROR - $'+ cp a.aaa a
+ cp b.bbb b

a.aaa : [current] must=1 compiled regular EXISTS

()a.aaa : [recent] event=[current] force compiled state

b.bbb : [current] .SCAN.bbb must=1 compiled regular scanned EXISTS

()b.bbb : [recent] .SCAN.bbb event=[current] compiled scanned state
 prerequisites: (BBB) 
'

	EXEC	--regress=sync ASCAN=1
		ERROR - $'+ cp a.aaa a

a.aaa : [current] .SCAN.aaa compiled regular scanned EXISTS

()a.aaa : [recent] .SCAN.aaa event=[current] compiled scanned state
 prerequisites: (AAA) 

b.bbb : [recent] .SCAN.bbb compiled regular scanned EXISTS

()b.bbb : [recent] .SCAN.bbb event=[recent] compiled scanned state
 prerequisites: (BBB) 
'

	EXEC	--regress=sync ASCAN=0
		ERROR - $'
a.aaa : [recent] compiled regular EXISTS

()a.aaa : [recent] event=[recent] compiled state

b.bbb : [recent] .SCAN.bbb compiled regular scanned EXISTS

()b.bbb : [recent] .SCAN.bbb event=[recent] compiled scanned state
 prerequisites: (BBB) 
'

	EXEC	--regress=sync ASCAN=0

	EXEC	--regress=sync ASCAN=1
		ERROR - $'+ cp a.aaa a

a.aaa : [current] .SCAN.aaa compiled regular scanned EXISTS

()a.aaa : [recent] .SCAN.aaa event=[current] compiled scanned state
 prerequisites: (AAA) 

b.bbb : [recent] .SCAN.bbb compiled regular scanned EXISTS

()b.bbb : [recent] .SCAN.bbb event=[recent] compiled scanned state
 prerequisites: (BBB) 
'

	EXEC	--regress=sync ASCAN=1
		ERROR - $'
a.aaa : [recent] .SCAN.aaa compiled regular scanned EXISTS

()a.aaa : [recent] .SCAN.aaa event=[recent] compiled scanned state
 prerequisites: (AAA) 

b.bbb : [recent] .SCAN.bbb compiled regular scanned EXISTS

()b.bbb : [recent] .SCAN.bbb event=[recent] compiled scanned state
 prerequisites: (BBB) 
'

TEST 19 '.SCAN.sh'

	EXEC	-n
		INPUT Makefile $'STATE == 1
x :: x.sh'
		INPUT x.sh $': include a.sh b.sh
echo $STATE
: include c.sh'
		INPUT a.sh
		INPUT b.sh
		INPUT c.sh
		OUTPUT - $'+ case message:$OPTIND:$RANDOM in
+ ?*:*:*|*::*|*:*:$RANDOM)
+ 	;;
+ *)	if	ENV= LC_ALL=C x= $SHELL -nc \'[[ a || b ]] && : ${list[level]} $(( 1 + $x )) !(pattern)\' 2>/dev/null
+ 	then	if	grep \'### .*archaic.* ###\' >/dev/null
+ 		then	: x contains archaic constructs :
+ 		else	ENV= LC_ALL=C $SHELL -n x.sh
+ 		fi
+ 	fi
+ 	;;
+ esac
+ case \'\' in
+ "")	case 7 in
+ 	0)	cp x.sh x
+ 		;;
+ 	*)	{
+ 		i=`(read x; echo $x) < x.sh`
+ 		case $i in
+ 		\'#!\'*|*\'||\'*|\':\'*|\'":"\'*|"\':\'"*)	echo "$i" ;;
+ 		esac
+ 		cat - x.sh <<\'!\'
+ STATE=1
+ !
+ 		} > x
+ 		;;
+ 	esac
+ 	;;
+ *)	cat - x.sh > x <<\'!\'
+ 
+ STATE=1
+ !
+ 	;;
+ esac
+ silent test -w x -a -x x || chmod u+w,+x x'

	EXEC	--silent
		OUTPUT -
		OUTPUT x $': include a.sh b.sh
STATE=1
: include a.sh b.sh
echo $STATE
: include c.sh'

	EXEC	--silent STATE=2
		OUTPUT x $': include a.sh b.sh
STATE=2
: include a.sh b.sh
echo $STATE
: include c.sh'

	EXEC	--silent STATE=2

	EXEC	--silent
		OUTPUT x $': include a.sh b.sh
STATE=1
: include a.sh b.sh
echo $STATE
: include c.sh'

TEST 20 '2 level makepath with header criss-cross'

	EXEC	-n
		INPUT Makeargs $'MAKEPATH = src'
		INPUT include/direct.h $'#define STATUS 0'
		INPUT src/Makefile $'.SOURCE.h : include
cmd :: cmd.c'
		INPUT src/cmd.c $'#include "indirect.h"
main() { return STATUS; }'
		INPUT src/include/direct.h $'#define STATUS 1'
		INPUT src/include/indirect.h $'#include "direct.h"'
		OUTPUT - $'+ cc -O -Iinclude -Isrc/include  -c src/cmd.c
+ cc  -O   -o cmd cmd.o'

	EXEC	-n
		INPUT Makeargs $'MAKEPATH = .:src'

	EXEC	-n
		INPUT Makeargs $'MAKEPATH = $(PWD):src'

TEST 21 '.SCAN.tst'

	EXEC	-n
		INPUT Makefile $'.SCAN.tst : .SCAN
	I|\\T COPY % |M$$(.TST.SUFFIX. tst)|
include "scan.mk"'
		INPUT scan.mk $'.SUFFIX.HEADER.tst = .hdr .tst
.TST.SUFFIX. : .FUNCTION
	local F L S T
	L := $(%:O=1)
	F := $(%%:/ .*//:/\\.$//:/\'\\(.*\\)\'/\\1/)
	if ! "$(F:S)"
		for S $(.SUFFIX.HEADER.$(L)) $(<<:S)
			T := $(F)$(S)
			$(T) : .SCAN.$(L)
			if "$(T:T=F)"
				F := $(T)
				break
			end
		end
	end
	return $(F)
.ATTRIBUTE.%.tst : .SCAN.tst 
all : a.tst
	: $(!:H) :'
		INPUT a.tst $'123456 COPY aaa.
       COPY bbb.
ABCDEF COPY ccc REPLACING
      *COPY ddd REPLACING
ABCDE *COPY ddd REPLACING
ABCDEF*COPY ddd REPLACING'
		INPUT aaa.hdr -
		INPUT bbb.tst -
		INPUT ccc
		OUTPUT - $'+ : a.tst aaa.hdr bbb.tst ccc :'

	EXEC	-n
		INPUT Makefile $'.SCAN.tst : .SCAN
	I|\\D COPY % |M$$(.TST.SUFFIX. tst)|
include "scan.mk"'
		OUTPUT - $'+ : a.tst aaa.hdr bbb.tst :'

	EXEC	-n
		INPUT Makefile $'.SCAN.tst : .SCAN
	I|\\V COPY % |M$$(.TST.SUFFIX. tst)|
include "scan.mk"'
		OUTPUT - $'+ : a.tst bbb.tst ccc :'

	EXEC	-n
		INPUT Makefile $'.SCAN.tst : .SCAN
	I| COPY % |M$$(.TST.SUFFIX. tst)|
include "scan.mk"'
		OUTPUT - $'+ : a.tst bbb.tst :'
