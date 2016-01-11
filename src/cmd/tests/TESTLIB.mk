/*
 * testlib library test harness operator
 *
 *	unit :TESTLIB: library [ testhdr*.h ] [ -ltestlib* ] test*.c
 *
 * *.[ch] assumed to be in ./unit
 */

.TEST.DEBUG.SYMBOLS : .MAKE .VIRTUAL .FORCE .REPEAT
	set --debug-symbols
	CCFLAGS += $(CC.DEBUG)
	CC.LIB.TYPE = -g

":TESTLIB:" : .MAKE .OPERATOR
	eval
	.SOURCE : $$(<)
	.SOURCE.h : ../../lib/libast/$$(<) ../../lib/libast
	$(<) :TEST: testlib terror.h $(>:O>1:C,.*\.[ch]$,$(<)/&,)
		set +x; $$(*:O=1) $$(TESTLIBFLAGS) $(>:O=1) $(<) $$(*:O>1) $$(CC) $$(CCFLAGS) $$(LDFLAGS) $$(TESTLIBCCFLAGS)
	test.$$(>:N=*.h:O=1:B:/\(..\).*/\1/:-$(<:B:/\(..\).*/\1/)) : .TEST.DEBUG.SYMBOLS testlib $$(>:V:O>1:N!=*.c) $$$(*.ARGS:D=$(<):B:S=.c) .CLEARARGS
		set +x; $$(*:O=1) $$(TESTLIBFLAGS) --verbose $(>:O=1) $(<) $$(*:O>1) $$(CC) $$(CCFLAGS) $$(LDFLAGS) $$(TESTLIBCCFLAGS)
	end
