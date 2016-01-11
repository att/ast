# ast nmake assignment tests

INCLUDE test.def

TEST 01 'delayed eval'

	EXEC	-n
		INPUT Makefile $'VAR1 = 1
VAR2 = $(VAR1)
VAR1 = 2
tst:
	: VAR1 : $(VAR1) : VAR2 : $(VAR2) :'
		OUTPUT - $'+ : VAR1 : 2 : VAR2 : 2 :'

	EXEC	-n VAR1=5
		OUTPUT - $'+ : VAR1 : 5 : VAR2 : 5 :'

	EXEC	-n VAR1+=5
		OUTPUT - $'+ : VAR1 : 2 5 : VAR2 : 2 5 :'

	EXEC	-n VAR2=5
		OUTPUT - $'+ : VAR1 : 2 : VAR2 : 5 :'

	EXEC	-n VAR2+=5
		OUTPUT - $'+ : VAR1 : 2 : VAR2 : 2 5 :'

TEST 02 'immediate eval'

	EXEC	-n
		INPUT Makefile $'VAR1 = 1
VAR2 := $(VAR1)
VAR1 = 2
tst:
	: VAR1 : $(VAR1) : VAR2 : $(VAR2) :'
		OUTPUT - $'+ : VAR1 : 2 : VAR2 : 1 :'

	EXEC	-n VAR1=5
		OUTPUT - $'+ : VAR1 : 5 : VAR2 : 5 :'

	EXEC	-n VAR1+=5
		OUTPUT - $'+ : VAR1 : 2 5 : VAR2 : 1 5 :'

	EXEC	-n VAR2=5
		OUTPUT - $'+ : VAR1 : 2 : VAR2 : 5 :'

	EXEC	-n VAR2+=5
		OUTPUT - $'+ : VAR1 : 2 : VAR2 : 1 5 :'

TEST 03 'append'

	EXEC	-n
		INPUT Makefile $'VAR1 = 1
VAR2 += $(VAR1)
VAR2 += 2
VAR1 += 3
tst:
	: VAR1 : $(VAR1) : VAR2 : $(VAR2) :'
		OUTPUT - $'+ : VAR1 : 1 3 : VAR2 : 1 2 :'

	EXEC	-n VAR1=5
		OUTPUT - $'+ : VAR1 : 5 : VAR2 : 5 2 :'

	EXEC	-n VAR1+=5
		OUTPUT - $'+ : VAR1 : 1 3 5 : VAR2 : 1 5 2 :'

	EXEC	-n VAR2=5
		OUTPUT - $'+ : VAR1 : 1 3 : VAR2 : 5 :'

	EXEC	-n VAR2+=5
		OUTPUT - $'+ : VAR1 : 1 3 : VAR2 : 1 2 5 :'

TEST 04 'auxilliary'

	EXEC	-n
		INPUT Makefile $'VAR1 = 1
VAR2 = 2
VAR1 &= $(VAR2)
tst:
	: VAR1 : $(VAR1) : VAR2 : $(VAR2) :'
		OUTPUT - $'+ : VAR1 : 1 2 : VAR2 : 2 :'

	EXEC	-n VAR1=5
		OUTPUT - $'+ : VAR1 : 5 2 : VAR2 : 2 :'

	EXEC	-n VAR1+=5
		OUTPUT - $'+ : VAR1 : 1 5 2 : VAR2 : 2 :'

	EXEC	-n VAR2=5
		OUTPUT - $'+ : VAR1 : 1 5 : VAR2 : 5 :'

	EXEC	-n VAR2+=5
		OUTPUT - $'+ : VAR1 : 1 2 5 : VAR2 : 2 5 :'

TEST 05 'delayed eval combinations'

	EXEC	-n
		INPUT src/file.t $'t'
		INPUT Makefile $'FILE = file.t
.SOURCE$(FILE:S) : src
.INIT : .MAKE
	if "$(DELAY)"
	LIB = $(FILE:T=F)
	else
	LIB := $(FILE:T=F)
	end
	DIR = $(LIB:D)
	error 0 LIB is $(LIB)
	error 0 LIB dir is $(LIB:D)
$(FILE) : .FORCE
	: $(LIB) $(LIB:T=F) $(<)
	: $(LIB:D) $(LIB:T=F:D) $(<:D) $(DIR)'
		OUTPUT - $'+ : src/file.t src/file.t file.t
+ : src src . src'
		ERROR - $'LIB is src/file.t
LIB dir is src'

	EXEC	-n DELAY=1
		OUTPUT - $'+ : file.t file.t file.t
+ : . . . .'

TEST 06 'defined in .mo'

	EXEC	--silent WHO=there
		INPUT Makefile $'WHO = world
all :
	echo hello $(WHO)'
		OUTPUT - $'hello there'

	EXEC	--silent WHO=there

	EXEC	--silent
		OUTPUT - $'hello world'

	EXEC	--silent

TEST 07 'frozen in .mo'

	EXEC	--silent WHO=there
		INPUT Makefile $'WHO = world
who := $(WHO)
all :
	echo hello $(who)'
		OUTPUT - $'hello there'

	EXEC	--silent WHO=there

	EXEC	--silent
		OUTPUT - $'hello world'

	EXEC	--silent

TEST 08 'variable and option scope'

	EXEC	-bcf base.mk
		INPUT base.mk $'
CLOBBER := 1
set option=";clobber;b;-;The install action removes targets instead of renaming to \atarget\a\b.old\b."
set clobber:=1
set jobs:=9'

	EXEC	MAKERULES=base -n
		INPUT Makefile $'all :
	echo CLOBBER=$(CLOBBER) --clobber=$(-clobber) --jobs=$(-jobs)'
		OUTPUT - $'+ echo CLOBBER=1 --clobber=1 --jobs=9'

	EXEC	MAKERULES=base -n CLOBBER=0 --clobber=0 --jobs=0
		INPUT Makefile $'all :
	echo CLOBBER=$(CLOBBER) --clobber=$(-clobber) --jobs=$(-jobs)'
		OUTPUT - $'+ echo CLOBBER=0 --clobber= --jobs='

	EXEC	MAKERULES=base -n CLOBBER= --noclobber --nojobs
		INPUT Makefile $'all :
	echo CLOBBER=$(CLOBBER) --clobber=$(-clobber) --jobs=$(-jobs)'
		OUTPUT - $'+ echo CLOBBER= --clobber= --jobs='

	EXEC	MAKERULES=base -n --'?clobber'
		OUTPUT -
		ERROR - $'make: [ options ] [ script ... ] [ target ... ]
OPTIONS
  --clobber       (base) The install action removes targets instead of renaming
                  to target.old.'
		EXIT 2

TEST 09 '+= vs space'

	EXEC	-n
		INPUT Makefile $'all : .MAKE
	a = a
	b =
	c = c
	A =
	A += $(a)
	B =
	B += $(a)
	B += $(b)
	C =
	C += $(a)
	C += $(b)
	C += $(c)
	D =
	D += $(a)
	D += $(c)
	E =
	E += $(b)
	E += $(c)
	F =
	F += $(c)
	G =
	G += $(b)
	for x A B C D E F G
		print $(x):$($(x)):
	end'
		OUTPUT - $'A:a:
B:a:
C:a c:
D:a c:
E:c:
F:c:
G::'
