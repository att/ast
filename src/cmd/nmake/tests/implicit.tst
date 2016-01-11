# ast nmake implicit prerequisite tests

INCLUDE cc.def

TEST 01 '.PFX.INCLUDE vs. .STD.INCLUDE --noprefix-include'

	EXEC	--regress=sync --noprefix-include
		INPUT Makefile $'.SOURCE.G : uhdr
.SOURCE.h : hdr
%.h : %.G
	cp $(*) $(<)
.MAIN : all
all : .MAKE .FORCE .REPEAT
	make foo.c
	query foo.c foo.h bar.G bar.h'
		INPUT uhdr/bar.G $'#include "foo.h"'
		INPUT hdr/foo.h $'#include "bar.h"'
		INPUT foo.c $'#include "foo.h"'
		ERROR - $'+ cp uhdr/bar.G bar.h

foo.c : [current] .SCAN.c must=1 regular scanned EXISTS

()foo.c : [recent] .SCAN.c event=[current] compiled scanned state
 prerequisites: hdr/foo.h==foo.h 

hdr/foo.h==foo.h : [current] .LCL.INCLUDE .PFX.INCLUDE .SCAN.c must=1 terminal regular scanned EXISTS

()foo.h : [recent] .LCL.INCLUDE .SCAN.c event=[current] implicit compiled scanned state
 prerequisites: bar.h 

uhdr/bar.G==bar.G : [current] must=1 regular EXISTS

()bar.G : [recent] event=[current] force compiled state

bar.h : [current] .LCL.INCLUDE .SCAN.c must=4 regular scanned triggered EXISTS

()bar.h : [current] .LCL.INCLUDE .SCAN.c event=[current] implicit built compiled state

(+)bar.h : cancel=[not found] .SCAN.c implicit compiled scanned state
 prerequisites: hdr/foo.h==foo.h 
'

	EXEC	--regress=sync --noprefix-include
		ERROR - $'
foo.c : [recent] .SCAN.c regular scanned EXISTS

()foo.c : [recent] .SCAN.c event=[recent] compiled scanned state
 prerequisites: hdr/foo.h==foo.h 

hdr/foo.h==foo.h : [recent] .LCL.INCLUDE .PFX.INCLUDE .SCAN.c terminal regular scanned EXISTS

()foo.h : [recent] .LCL.INCLUDE .PFX.INCLUDE .SCAN.c event=[recent] implicit compiled scanned state
 prerequisites: bar.h 

uhdr/bar.G==bar.G : [recent] regular EXISTS

()bar.G : [recent] event=[recent] force compiled state

bar.h : [recent] .LCL.INCLUDE .SCAN.c regular scanned EXISTS

()bar.h : [recent] .LCL.INCLUDE .SCAN.c event=[recent] implicit built compiled state

(+)bar.h : cancel=[not found] .SCAN.c implicit compiled scanned state
 prerequisites: hdr/foo.h==foo.h 
'

	EXEC	--regress=sync --noprefix-include

TEST 02 '.PFX.INCLUDE vs. .STD.INCLUDE subdir --prefix-include'

	EXEC	--regress=sync --prefix-include
		INPUT Makefile $'.SOURCE.G : uhdr
.SOURCE.h : hdr
%.h : %.G
	cp $(*) $(<)
.MAIN : all
all : .MAKE .FORCE .REPEAT
	make foo.c
	query foo.c foo.h bar.G bar.h'
		INPUT uhdr/bar.G $'#include "foo.h"'
		INPUT hdr/foo.h $'#include "bar.h"'
		INPUT foo.c $'#include "foo.h"'
		ERROR - $'+ cp uhdr/bar.G bar.h

foo.c : [current] .SCAN.c must=1 regular scanned EXISTS

()foo.c : [recent] .SCAN.c event=[current] compiled scanned state
 prerequisites: hdr/foo.h==foo.h 

hdr/foo.h==foo.h : [current] .LCL.INCLUDE .PFX.INCLUDE .SCAN.c must=1 terminal regular scanned EXISTS

()foo.h : [recent] .LCL.INCLUDE .SCAN.c event=[current] implicit compiled scanned state
 prerequisites: bar.h 

uhdr/bar.G==bar.G : [current] must=1 regular EXISTS

()bar.G : [recent] event=[current] force compiled state

bar.h : [current] .LCL.INCLUDE .SCAN.c must=4 regular scanned triggered EXISTS

()bar.h : [current] .LCL.INCLUDE .SCAN.c event=[current] implicit built compiled state

(+)bar.h : cancel=[not found] .SCAN.c implicit compiled scanned state
 prerequisites: hdr/foo.h==foo.h 
'

	EXEC	--regress=sync --prefix-include
		ERROR - $'
foo.c : [recent] .SCAN.c regular scanned EXISTS

()foo.c : [recent] .SCAN.c event=[recent] compiled scanned state
 prerequisites: hdr/foo.h==foo.h 

hdr/foo.h==foo.h : [recent] .LCL.INCLUDE .PFX.INCLUDE .SCAN.c terminal regular scanned EXISTS

()foo.h : [recent] .LCL.INCLUDE .PFX.INCLUDE .SCAN.c event=[recent] implicit compiled scanned state
 prerequisites: bar.h 

uhdr/bar.G==bar.G : [recent] regular EXISTS

()bar.G : [recent] event=[recent] force compiled state

bar.h : [recent] .LCL.INCLUDE .SCAN.c regular scanned EXISTS

()bar.h : [recent] .LCL.INCLUDE .SCAN.c event=[recent] implicit built compiled state

(+)bar.h : cancel=[not found] .SCAN.c implicit compiled scanned state
 prerequisites: hdr/foo.h==foo.h 
'

	EXEC	--regress=sync --prefix-include

TEST 03 '.PFX.INCLUDE vs. .PFX.INCLUDE . --noprefix-include'

	EXEC	--regress=sync --noprefix-include
		INPUT Makefile $'.SOURCE.G : uhdr
.SOURCE.h : hdr
%.h : %.G
	cp $(*) $(<)
.MAIN : all
all : .MAKE .FORCE .REPEAT
	make foo.c
	query foo.c foo.h bar.G bar.h'
		INPUT uhdr/bar.G $'#include "foo.h"'
		INPUT foo.h $'#include "bar.h"'
		INPUT foo.c $'#include "foo.h"'
		ERROR - $'+ cp uhdr/bar.G bar.h

foo.c : [current] .SCAN.c must=1 regular scanned EXISTS

()foo.c : [recent] .SCAN.c event=[current] compiled scanned state
 prerequisites: foo.h 

foo.h : [current] .LCL.INCLUDE .PFX.INCLUDE .SCAN.c must=1 terminal regular scanned EXISTS

()foo.h : [recent] .LCL.INCLUDE .SCAN.c event=[current] implicit compiled scanned state
 prerequisites: bar.h 

uhdr/bar.G==bar.G : [current] must=1 regular EXISTS

()bar.G : [recent] event=[current] force compiled state

bar.h : [current] .LCL.INCLUDE .SCAN.c must=4 regular scanned triggered EXISTS

()bar.h : [current] .LCL.INCLUDE .SCAN.c event=[current] implicit built compiled state

(+)bar.h : cancel=[not found] .SCAN.c implicit compiled scanned state
 prerequisites: foo.h 
'

	EXEC	--regress=sync --noprefix-include
		ERROR - $'
foo.c : [recent] .SCAN.c regular scanned EXISTS

()foo.c : [recent] .SCAN.c event=[recent] compiled scanned state
 prerequisites: foo.h 

foo.h : [recent] .LCL.INCLUDE .PFX.INCLUDE .SCAN.c terminal regular scanned EXISTS

()foo.h : [recent] .LCL.INCLUDE .PFX.INCLUDE .SCAN.c event=[recent] implicit compiled scanned state
 prerequisites: bar.h 

uhdr/bar.G==bar.G : [recent] regular EXISTS

()bar.G : [recent] event=[recent] force compiled state

bar.h : [recent] .LCL.INCLUDE .SCAN.c regular scanned EXISTS

()bar.h : [recent] .LCL.INCLUDE .SCAN.c event=[recent] implicit built compiled state

(+)bar.h : cancel=[not found] .SCAN.c implicit compiled scanned state
 prerequisites: foo.h 
'

	EXEC	--regress=sync --noprefix-include

TEST 04 '.PFX.INCLUDE vs. .PFX.INCLUDE . --prefix-include'

	EXEC	--regress=sync --prefix-include
		INPUT Makefile $'.SOURCE.G : uhdr
.SOURCE.h : hdr
%.h : %.G
	cp $(*) $(<)
.MAIN : all
all : .MAKE .FORCE .REPEAT
	make foo.c
	query foo.c foo.h bar.G bar.h'
		INPUT uhdr/bar.G $'#include "foo.h"'
		INPUT foo.h $'#include "bar.h"'
		INPUT foo.c $'#include "foo.h"'
		ERROR - $'+ cp uhdr/bar.G bar.h

foo.c : [current] .SCAN.c must=1 regular scanned EXISTS

()foo.c : [recent] .SCAN.c event=[current] compiled scanned state
 prerequisites: foo.h 

foo.h : [current] .LCL.INCLUDE .PFX.INCLUDE .SCAN.c must=1 terminal regular scanned EXISTS

()foo.h : [recent] .LCL.INCLUDE .SCAN.c event=[current] implicit compiled scanned state
 prerequisites: bar.h 

uhdr/bar.G==bar.G : [current] must=1 regular EXISTS

()bar.G : [recent] event=[current] force compiled state

bar.h : [current] .LCL.INCLUDE .SCAN.c must=4 regular scanned triggered EXISTS

()bar.h : [current] .LCL.INCLUDE .SCAN.c event=[current] implicit built compiled state

(+)bar.h : cancel=[not found] .SCAN.c implicit compiled scanned state
 prerequisites: foo.h 
'

	EXEC	--regress=sync --prefix-include
		ERROR - $'
foo.c : [recent] .SCAN.c regular scanned EXISTS

()foo.c : [recent] .SCAN.c event=[recent] compiled scanned state
 prerequisites: foo.h 

foo.h : [recent] .LCL.INCLUDE .PFX.INCLUDE .SCAN.c terminal regular scanned EXISTS

()foo.h : [recent] .LCL.INCLUDE .PFX.INCLUDE .SCAN.c event=[recent] implicit compiled scanned state
 prerequisites: bar.h 

uhdr/bar.G==bar.G : [recent] regular EXISTS

()bar.G : [recent] event=[recent] force compiled state

bar.h : [recent] .LCL.INCLUDE .SCAN.c regular scanned EXISTS

()bar.h : [recent] .LCL.INCLUDE .SCAN.c event=[recent] implicit built compiled state

(+)bar.h : cancel=[not found] .SCAN.c implicit compiled scanned state
 prerequisites: foo.h 
'

	EXEC	--regress=sync --prefix-include

TEST 05 'generate implicit include'

	EXEC
		INPUT Makefile $'t :: t.c
t.c :
	{
	echo \'#include "t.h"\'
	echo \'main() { return T; }\'
	} > $(<)
t.h : t.src
	cat $(>) > $(<)'
		INPUT t.src $'#define T 0'
		ERROR - $'+ 1> t.c
+ echo \'#include "t.h"\'
+ echo \'main() { return T; }\'
+ cat t.src
+ 1> t.h
+ cc -O -I. -c t.c
+ cc -O -o t t.o'
