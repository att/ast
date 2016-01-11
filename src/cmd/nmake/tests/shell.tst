# ast nmake shell tests

INCLUDE test.def

TEST 01 'here documents'

	EXEC
		INPUT Makefile $'tst :
	cat > $(<) <<XXX
	main(){}
	XXX'
		OUTPUT tst $'main(){}'
		ERROR - $'+ cat
+ 1> tst 0<< \\XXX
main(){}
XXX'

TEST 02 'shell exit'

	EXEC	CODE=0
		INPUT Makefile $'CODE = 0
TEST = exit $(CODE)
all :
	{
		$(TEST)
	}
	: done $(<) :'
		ERROR - $'+ exit 0'

	EXEC	CODE=1
		ERROR - $'+ exit 1
make: *** exit code 1 making all'
		EXIT 1

	EXEC	CODE=2
		ERROR - $'+ exit 2
make: *** exit code 2 making all'
		EXIT 1

TEST 03 'shell conditionals'

	EXEC	OK=false
		INPUT Makefile $'OK = error
out : in
	: $(>) :
	$(OK) && touch $(<)
	$(OK)'
		INPUT in
		ERROR - $'+ : in :
+ false
+ false
make: *** exit code 1 making out'
		EXIT 1

	EXEC	OK=true
		ERROR - $'+ : in :
+ true
+ touch out
+ true'
		EXIT 0

	EXEC	OK=true
		ERROR -

	EXEC	OK=false
