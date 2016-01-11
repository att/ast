# ast nmake internal interface and engine flow tests

INCLUDE test.def

TEST 01 'nametype'

	EXEC	-n -f - . 'query - nametype ABC .ABC ABC. A.C .ABC. /A/B/C ()ABC (ABC) (+)ABC A$$(B)C A|B (A|B) *(ABC) @(ABC) (A/B/C) A*B*C A?B?C'
		OUTPUT - $'             ABC identifier
            .ABC variable
            ABC. variable
             A.C variable
           .ABC. intvar
          /A/B/C path
           ()ABC staterule
           (ABC) statevar
          (+)ABC altstate
          A$(B)C dynamic
             A|B glob
           (A|B) statevar
          *(ABC) glob
          @(ABC) glob
         (A/B/C) statevar
           A*B*C glob
           A?B?C glob'

TEST 02 'on the fly makefile conversion'

	DO	export MAKECONVERT=$'buildfile "tr A-Z a-z < $(>)"'
	EXEC	all
		INPUT buildfile $'ALL :\n\tECHO $(<)'
		OUTPUT - $'all'
		ERROR - $'+ tr A-Z a-z
+ 0< buildfile
+ echo all'

	DO	export MAKECONVERT=$'myfile:buildfile "tr A-Z a-z < $(>)"'
	EXEC	all
		ERROR - $'+ echo all'

	DO	export MAKECONVERT=$'$(CONVERT:B) "tr A-Z a-z < $(>)"'
	EXEC	all CONVERT=junk/buildfile.junk
		ERROR - $'+ echo all'

	DO	export MAKECONVERT=$'foofile:barfile "tr A-Z a-z < $(>)"'
	EXEC	all
		OUTPUT -
		ERROR - $'make: a makefile must be specified when foofile,barfile,Nmakefile,nmakefile,Makefile,makefile omitted'
		EXIT 1

	DO	unset MAKECONVERT

TEST 03 '.MAKEINIT vs. .INIT'

	# .MAKEINIT << candidate state var freeze << .INIT

	EXEC	-n INIT=.MAKEINIT
		INPUT Makefile $'STATEVAR == 1
$(INIT) : .init
.init : .MAKE init.c
	print $(?init.c)
	exit 0'
		INPUT init.c $'#include "init.h"
int statevar = STATEVAR;'
		INPUT init.h
		OUTPUT - $'init.h'

	EXEC	-n INIT=.INIT
		OUTPUT - $'(STATEVAR) init.h'

	# .MAKEINIT << command line scripts << .INIT

	EXEC	-n INIT=.MAKEINIT '.init : notfound.c'
		OUTPUT - $'init.h'

	EXEC	-n INIT=.INIT '.init : notfound.c'
		OUTPUT -
		ERROR - $'make: warning: don\'t know how to make .INIT : .init : notfound.c
make: Makefile: a main target must be specified'
		EXIT 1

TEST 04 ':MAKE: + MAKEFILES'

	EXEC	-n
		INPUT Makefile $'MAKEFILES = $(PWD:B).mk
.SCRIPT : MAKEFILES
:MAKE: maintenance'
		INPUT maintenance/maintenance.mk $'all :
	: $(PWD:B) : $(MAKEFILES) :'
		OUTPUT - $'+ : maintenance : maintenance.mk :'
		ERROR - $'maintenance:'

	EXEC	-n
		INPUT Makefile $'MAKEFILES = $(PWD:B).mk
script MAKEFILES
:MAKE: maintenance'

	EXEC	-n
		INPUT Makefile $'MAKEFILES = $(PWD:B).mk
script MAKEFILES
:MAKE:'
		OUTPUT -
		ERROR -

	EXEC	-n
		INPUT Makefile $'MAKEFILES = $(PWD:B).mk
:MAKE: maintenance'
		ERROR - $'maintenance:
make [maintenance]: warning: a makefile must be specified when Nmakefile,nmakefile,Makefile,makefile omitted
make [maintenance]: null: a main target must be specified
make: *** exit code 1 making maintenance'
		EXIT 1

TEST 05 ':MAKE: + MAKEFILES'

	EXEC	-s
		INPUT Makefile $'.JOBDONE : .FUNCTION
	local ( NAME CODE USR SYS ) $(%)
	if CODE
		error 2 $(NAME) failed with exit code $(CODE)
		exit $(CODE)
	else
		error 2 $(NAME) ok
	end
all : good bad ugly
good :
	exit 0
bad :
	exit 12
ugly :
	exit 34'
		ERROR - $'make: good ok
make: bad failed with exit code 12'
		EXIT 12
