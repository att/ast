# ast nmake 2d viewpath and makepath tests

INCLUDE cc.def

TEST 01 'no view'

	EXEC
		INPUT Makefile $'all : .MAKE
	print $(*.VIEW)
	print $(...:N=\\(*\\)$(MAKEFILE)*:H)
	print $(...:N=$(MAKEFILE)*:H)'
		OUTPUT - $'.\n\nMakefile Makefile.mo'

	EXEC
		OUTPUT - $'.\n()Makefile\nMakefile Makefile.mo Makefile.ms'

TEST 02 'basic MAKEPATH'

	EXPORT MAKEPATH=../bot

	CD top

	EXEC	-n
		INPUT ../bot/Makefile $'main :: main.c'
		INPUT ../bot/main.c $'#include "hdr.h"
int main(){return 0;}'
		INPUT ../bot/hdr.h $'extern int n;'
		OUTPUT - $'+ cc -O -I../bot  -c ../bot/main.c
+ cc  -O   -o main main.o'

TEST 03 'basic VPATH'

	EXPORT VPATH=$TWD/top:$TWD/bot

	CD	top/src

	EXEC	-n
		INPUT $TWD/bot/src/Makefile $'main :: main.c'
		INPUT $TWD/bot/src/main.c $'#include "hdr.h"
int main(){return 0;}'
		INPUT $TWD/bot/src/hdr.h $'extern int n;'
		OUTPUT - $'+ cc -O -I'$TWD$'/bot/src  -c '$TWD$'/bot/src/main.c
+ cc  -O   -o main main.o'

TEST 04 'VPATH vs -I-'

	EXPORT VPATH=$TWD/dev:$TWD/ofc

	CD	dev

	EXEC	-n -s -g debug.mk s.i CC.DIALECT='-I-'
		INPUT $TWD/ofc/Makefile $'.INIT : .ppinit
.ppinit : .MAKE .VIRTUAL .FORCE
	CC = $(MAKEPP) -I-D'$TWD$'/pp.probe
s :: s.c'
		INPUT $TWD/ofc/s.c $'#include "h1.h"'
		INPUT $TWD/ofc/h1.h $'#include "h2.h"
int ofc_h1;'
		INPUT $TWD/ofc/h2.h $'int ofc_h2;'
		OUTPUT s.i $'# 1 "'$TWD$'/ofc/s.c"

# 1 "'$TWD$'/ofc/h1.h" 1

# 1 "'$TWD$'/ofc/h2.h" 1
int ofc_h2;
# 2 "'$TWD$'/ofc/h1.h" 2
int ofc_h1;
# 2 "'$TWD$'/ofc/s.c" 2'

	EXEC	-n -s -g debug.mk s.i CC.DIALECT='-I-'
		INPUT $TWD/dev/h2.h $'int dev_h2;'
		OUTPUT s.i $'# 1 "'$TWD$'/ofc/s.c"

# 1 "'$TWD$'/ofc/h1.h" 1

# 1 "h2.h" 1
int dev_h2;
# 2 "'$TWD$'/ofc/h1.h" 2
int ofc_h1;
# 2 "'$TWD$'/ofc/s.c" 2'

TEST 05 ':MAKE: with one node VPATH'

	EXPORT VPATH=$TWD

	EXEC	-n
		INPUT Makefile $':MAKE:'
		INPUT lmf/Makefile $':MAKE:'
		INPUT lmf/src/Makefile $':MAKE:'
		INPUT lmf/src/api/Makefile $':MAKE:'
		INPUT lmf/src/api/mmomc/Makefile $':MAKE:'
		INPUT lmf/src/api/mmomc/tools/Makefile $':MAKE:'
		INPUT lmf/src/api/mmomc/tools/src/Makefile $':MAKE:'
		INPUT lmf/src/api/mmomc/tools/src/item/Makefile $':MAKE:'
		INPUT lmf/src/api/mmomc/tools/src/item/libsrc/Makefile $':MAKE:'
		INPUT lmf/src/api/mmomc/tools/src/item/libsrc/item/Makefile $'
NODE1		= $(VPATH:/:/ /G:O=1)
MMOMCTOP1	= $(PWD:P=R=$(NODE1))
MMOMCTOP	= $(MMOMCTOP1)/lmf/src/api/mmomc
ARCH_MAKEFILE	= Item.arch.mk
include $(MMOMCTOP)/include/nmake/desc.mk
all :
	: NODE1 : $(NODE1) :
	: MMOMCTOP1 : $(MMOMCTOP1) :
	: MMOMCTOP : $(MMOMCTOP) :
	: ARCH_MAKEFILE : $(ARCH_MAKEFILE) :
	: $(MMOMCTOP)/include/nmake/desc.mk :'
		INPUT lmf/src/api/mmomc/include/nmake/desc.mk $'error 1 hit desc.mk'
		OUTPUT - $'+ : NODE1 : '$TWD$' :
+ : MMOMCTOP1 : ../../../../../../../../.. :
+ : MMOMCTOP : ../../../../../../../../../lmf/src/api/mmomc :
+ : ARCH_MAKEFILE : Item.arch.mk :
+ : ../../../../../../../../../lmf/src/api/mmomc/include/nmake/desc.mk :'
		ERROR - $'lmf/src/api/mmomc/tools/src/item/libsrc/item:
make [lmf/src/api/mmomc/tools/src/item/libsrc/item]: warning: hit desc.mk
lmf/src/api/mmomc/tools/src/item/libsrc:
lmf/src/api/mmomc/tools/src/item:
lmf/src/api/mmomc/tools/src:
lmf/src/api/mmomc/tools:
lmf/src/api/mmomc:
lmf/src/api:
lmf/src:
lmf:'

TEST 06 'VROOT'

	EXPORT VPATH=$TWD/ofc/proj

	CD ofc/proj

	EXEC	-n
		INPUT $TWD/ofc/proj/Makefile $'all :
	: $(PWD) : $(VROOT) :'
		OUTPUT - $'+ : '$TWD$'/ofc/proj : . :'

	CD src

	EXEC	-n
		INPUT $TWD/ofc/proj/src/Makefile $'all :
	: $(PWD) : $(VROOT) :'
		OUTPUT - $'+ : '$TWD$'/ofc/proj/src : .. :'

	CD util

	EXEC	-n
		INPUT $TWD/ofc/proj/src/util/Makefile $'all :
	: $(PWD) : $(VROOT) :'
		OUTPUT - $'+ : '$TWD$'/ofc/proj/src/util : ../.. :'

	EXPORT VPATH=$TWD/dev/proj:$TWD/ofc/proj

	CD $TWD/dev/proj

	EXEC	-n
		OUTPUT - $'+ : '$TWD$'/dev/proj : . :'

	CD src

	EXEC	-n
		OUTPUT - $'+ : '$TWD$'/dev/proj/src : .. :'

	CD util

	EXEC	-n
		OUTPUT - $'+ : '$TWD$'/dev/proj/src/util : ../.. :'

TEST 07 ':MAKE: with 2 node VPATH'

	EXPORT	VPATH=$TWD/top:$TWD/bot

	CD top

	EXEC	-n
		INPUT $TWD/top/Makefile $':MAKE:'
		INPUT $TWD/top/a/Makefile $'all : a.x\n\t: $(PWD) : $(*)'
		INPUT $TWD/bot/a/a.x
		INPUT $TWD/bot/b/Makefile $'all : b.x\n\t: $(PWD) : $(*)'
		INPUT $TWD/top/b/b.x
		OUTPUT - $'+ : '$TWD$'/top/a : '$TWD$'/bot/a/a.x
+ : '$TWD$'/top/b : b.x'
		ERROR - $'a:\nb:'

TEST 08 ':INSTALLDIR: with 2 node VPATH'

	EXPORT	VPATH=$TWD/dev:$TWD/ofc

	CD	ofc

	EXEC	install
		INPUT Makefile $'INSTALLROOT = .
.SOURCE : src
$(ETCDIR) :INSTALLDIR: a.c b.c c.c'
		INPUT src/a.c $'ofc'
		INPUT src/b.c
		INPUT src/c.c
		ERROR - $'+ mkdir -p etc
+ ignore cp src/a.c etc/a.c
+ ignore cp src/b.c etc/b.c
+ ignore cp src/c.c etc/c.c'

	CD	../dev

	EXEC	--regress=sync install
		ERROR - $'+ mkdir -p etc'

	EXEC	install
		INPUT src/a.c $'dev'
		ERROR - $'+ ignore cp src/a.c etc/a.c'

TEST 09 '3 node contained VPATH with .SOURCE dups'

	EXPORT	VPATH=$TWD:$TWD/inca:$TWD/incb

	EXEC	-n
		INPUT Makefile $'t :: t.c'
		INPUT t.c $'#include "a.h"
#include "b.h"
int main() { return 0; }'
		INPUT inca/a.h $'#define a 1'
		INPUT incb/b.h $'#define b 1'
		OUTPUT - $'+ cc -O -I'$TWD$'/inca -I'$TWD$'/incb  -c t.c
+ cc  -O   -o t t.o'

	EXEC	-n
		INPUT Makefile $'.SOURCE.h : inca incb\nt :: t.c'

TEST 10 'VROOT after VPATH change'

	EXPORT	VPATH=$TWD/n1

	CD	n1/a

	EXEC
		INPUT $TWD/n1/Localrules.mk $'LOCAL = local'
		INPUT Makefile $'include "$(VROOT)/Localrules.mk"
all :
	: LOCAL : $(LOCAL) :
	: VROOT : $(VROOT) :
	: VOFFSET : $(VOFFSET) :'
		ERROR - $'+ : LOCAL : local :
+ : VROOT : .. :
+ : VOFFSET : a :'

	EXPORT	VPATH=$TWD/n2:$TWD/n1

	CD	../../n2/a

	EXEC

TEST 11 'VROOT/VOFFSET across levels and nodes'

	EXPORT	VPATH=$TWD/ofc

	CD	ofc/proj

	EXEC
		INPUT $TWD/ofc/proj/Makefile $'all :
	: VROOT : $(VROOT) :
	: VOFFSET : $(VOFFSET) :'
		INPUT $TWD/ofc/proj/src/Makefile $'all :
	: VROOT : $(VROOT) :
	: VOFFSET : $(VOFFSET) :'
		INPUT $TWD/ofc/proj/src/util/Makefile $'all :
	: VROOT : $(VROOT) :
	: VOFFSET : $(VOFFSET) :'
		ERROR - $'+ : VROOT : .. :\n+ : VOFFSET : proj :'

	CD	src

	EXEC
		ERROR - $'+ : VROOT : ../.. :\n+ : VOFFSET : proj/src :'

	CD	util

	EXEC
		ERROR - $'+ : VROOT : ../../.. :\n+ : VOFFSET : proj/src/util :'

	EXPORT	VPATH=$TWD/dev:$TWD/ofc

	CD	../../../../dev/proj

	EXEC
		ERROR - $'+ : VROOT : .. :\n+ : VOFFSET : proj :'

	CD	src

	EXEC
		ERROR - $'+ : VROOT : ../.. :\n+ : VOFFSET : proj/src :'

	CD	util

	EXEC
		ERROR - $'+ : VROOT : ../../.. :\n+ : VOFFSET : proj/src/util :'

TEST 12 'header overlay'

	EXPORT	VPATH=$TWD/top:$TWD/bot

	CD	bot

	EXEC	--regress=sync
		INPUT Makefile $'t :LIBRARY: t.c'
		INPUT t.c $'#include "t.h"\nint t(){return 0;}'
		INPUT t.h $'#define T 1'
		ERROR - $'+ echo \'\' -lt
+ 1> t.req
+ cc -O -I. -c t.c
+ ar cr libt.a t.o
+ ignore ranlib libt.a
+ rm -f t.o'

	EXEC	--regress=sync
		ERROR -

	CD	../top

	EXEC	--regress=sync
		INPUT t.h $'#define T 2'
		ERROR - $'+ cc -O -I. -c '$TWD$'/bot/t.c
+ cp '$TWD$'/bot/libt.a libt.a
+ ar cr libt.a t.o
+ ignore ranlib libt.a
+ rm -f t.o'

	EXEC	--regress=sync
		ERROR -

	EXEC	--regress=sync clobber
		ERROR - $'+ ignore rm -f -r libt.a Makefile.ms'

	CD	../bot

	EXEC	--regress=sync clobber
		ERROR - $'+ ignore rm -f -r libt.a Makefile.mo Makefile.ms t.req'

TEST 13 'joint metarule overlay'

	EXPORT	VPATH=$TWD/bot

	CD	bot

	EXEC	target
		INPUT Makefile $'%.c %.h : %.j
	echo "#define PWD \\"this file is from $(PWD)\\"" > $(<:N=*.h)
	echo "int i;" >$(<:N=*.c)
target :: file.j main.c'
		INPUT file.j
		INPUT main.c $'#include "file.h"
int main() { return PWD != 0; }'
		ERROR - $'+ echo \'#define PWD "this file is from '$TWD$'/bot"\'
+ 1> file.h
+ echo \'int i;\'
+ 1> file.c
+ cc -O -c file.c
+ cc -O -I. -c main.c
+ cc -O -o target file.o main.o'

	EXPORT	VPATH=$TWD/top:$TWD/bot

	CD	../top

	EXEC	--regress=sync target
		ERROR -

	EXEC	target
		INPUT file.j $'TOP'
		ERROR - $'+ echo \'#define PWD "this file is from '$TWD$'/top"\'
+ 1> file.h
+ echo \'int i;\'
+ 1> file.c
+ cc -O -c file.c
+ cc -O -I. -c '$TWD$'/bot/main.c
+ cc -O -o target file.o main.o'

	EXEC	clobber
		ERROR - $'+ ignore rm -f -r target main.o file.c file.h file.o Makefile.ms'

	CD	../bot

	EXEC	clobber
		ERROR - $'+ ignore rm -f -r target main.o file.c file.h file.o Makefile.mo Makefile.ms'

TEST 14 'view over multiple subdirs'

	EXPORT	VPATH=$TWD/bot

	CD	bot

	EXEC	-n
		INPUT Makefile $':MAKE: s1 s1.0 s2
s2 :
	cd $(<)
	nmake $(-) $(=)
	silent echo $(<) done'
		INPUT s1/makefile $'s1 :\n\techo make $(<)'
		INPUT s1.0/nmakefile $'s1.0 :\n\techo make $(<)'
		INPUT s2/Nmakefile $'s2 :\n\techo make $(<)'
		OUTPUT - $'+ echo make s1
+ echo make s1.0
+ echo make s2
s2 done'
		ERROR - $'s1:
s1.0:
+ cd s2
+ nmake --noexec \'--regress=message\' \'--native-pp=-1\' --noprefix-include --novirtual'

	EXPORT	VPATH=$TWD/top:$TWD/bot

	CD	../top

	EXEC	-n --novirtual
		INPUT s2/
		OUTPUT - $'+ echo make s2
s2 done'
		ERROR - $'s1: warning: cannot recurse on virtual directory
s1.0: warning: cannot recurse on virtual directory
+ cd s2
+ nmake --noexec \'--regress=message\' \'--native-pp=-1\' --noprefix-include --novirtual'

	EXEC	-n
		OUTPUT - $'+ echo make s1
+ echo make s1.0
+ echo make s2
s2 done'
		ERROR - $'s1:
s1.0:
+ cd s2
+ nmake --noexec \'--regress=message\' \'--native-pp=-1\' --noprefix-include'

	EXEC	-n --novirtual
		ERROR - $'s1:
s1.0:
+ cd s2
+ nmake --noexec \'--regress=message\' \'--native-pp=-1\' --noprefix-include --novirtual'

TEST 15 '3 levels'

	EXPORT	VPATH=$TWD/bot

	CD	bot/esql

	EXEC
		INPUT $TWD/bot/esql/Makefile $'.SOURCE.eh : ../hdr

ESQL      = print
ESQLFLAGS =

(ESQL) (ESQLFLAGS) : .PARAMETER

.SOURCE.%.SCAN.esqlc : .FORCE $$(*.SOURCE.ec) $$(*.SOURCE) $$(*.SOURCE.eh) $$(*.SOURCE.h)

/* refer to standard scan rule for SQL files (.SCAN.sql)--though all 
 * embedded SQL statements should conform to ANSI syntax, we allow for
 * Informix proprietary extensions
 */
.SCAN.esqlc : .SCAN
	Q|/*|*/||C|
	Q|//||\\\\|LC|
	Q|"|"|\\\\|LQ|
	Q|\'|\'|\\\\|LQ|
	Q|\\\\|||CS|
	I| EXEC SQL include "%";|
	I| EXEC SQL include %;|
	I| $ include "%";|
	I| $ include <%>;|
	I| $ include %;|

.ATTRIBUTE.%.ec : .SCAN.esqlc

.sqlc.SEMAPHORE : .SEMAPHORE

%.cc : %.ec .sqlc.SEMAPHORE (ESQL) (ESQLFLAGS)
	$(CP) $(>) $(<)

all : DBcreateTables.cc DBdropTables.cc'
		INPUT $TWD/bot/esql/DBcreateTables.ec
		INPUT $TWD/bot/esql/DBdropTables.ec
		INPUT $TWD/bot/hdr/DBschema.eh
		ERROR - $'+ cp DBcreateTables.ec DBcreateTables.cc
+ cp DBdropTables.ec DBdropTables.cc'

	EXPORT	VPATH=$TWD/mid:$TWD/bot

	CD	../../mid/esql

	EXEC
		INPUT $TWD/mid/esql/
		INPUT $TWD/mid/hdr/
		ERROR -

	EXPORT	VPATH=$TWD/top:$TWD/mid:$TWD/bot

	CD	../../top/esql

	EXEC
		INPUT $TWD/top/esql/
		INPUT $TWD/top/hdr/
