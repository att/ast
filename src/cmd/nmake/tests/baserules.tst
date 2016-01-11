# ast nmake base rules tests

INCLUDE test.def

TEST 01 'alternate base rules'

	EXEC	-bcf myrules.mk
		INPUT myrules.mk $'set --regress
COPY = cp
COPYFLAGS =
%.to : %.from (COPY) (COPYFLAGS)
	$(COPY) $(COPYFLAGS) $(>) $(<)'

	EXEC	-f u1.mk
		INPUT u1.mk $'rules "myrules"
all : x.to'
		INPUT x.from
		ERROR - $'+ cp x.from x.to'

	EXEC	-f u1.mk
		ERROR -

	EXEC	-n -f u1.mk MAKERULES=norules
		ERROR -

	EXEC	-f u2.mk
		INPUT u2.mk $'rules myrules
all : x.to'
		ERROR - $'+ cp x.from x.to'

	EXEC	-f u2.mk
		ERROR -

	EXEC	-f u3.mk
		INPUT u3.mk $'rules myrules
all : x.o'
		INPUT x.c
		ERROR - $'make: don\'t know how to make all : x.o'
		EXIT 1

	EXEC	-g g1.mk -f u1.mk
		INPUT g1.mk $'rules myrules
COPYFLAGS = -f'
		ERROR - $'+ cp -f x.from x.to'
		EXIT 0

	EXEC	-g g1.mk -f u1.mk
		ERROR -

	EXEC	-g g2.mk -f u1.mk
		INPUT g2.mk $'COPYFLAGS = -p'
		ERROR - $'+ cp -p x.from x.to'

	EXEC	-g g2.mk -f u1.mk
		ERROR -

	EXEC	-g g3.mk -f u1.mk
		INPUT g3.mk $'rules makerules
COPYFLAGS = -p'
		ERROR - $'make: "g3.mk", line 1: makerules: incompatible with current base rules myrules'
		EXIT 1

	EXEC	-c -f u4.mk
		INPUT u4.mk $'all : x.to'
		ERROR -
		EXIT 0

	EXEC	-n -f u4.mk MAKERULES=myrules
		OUTPUT - $'+ cp  x.from x.to'
		ERROR - $'make: warning: makerules: base rules changed to myrules
make: warning: u4.mo: recompiling'

	EXEC	-n -f u4.mk MAKERULES=myrules.mo

	EXEC	-n -f u4.mk MAKERULES=$TWD/myrules.mo

	EXPORT	MAKERULES=myrules

	EXEC	-n -f u4.mk

	EXPORT	MAKERULES=myrules.mo

	EXEC	-n -f u4.mk

	EXPORT	MAKERULES=$TWD/myrules.mo

	EXEC	-n -f u4.mk
