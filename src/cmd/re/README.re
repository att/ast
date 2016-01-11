This directory contains AT&T test harnesses and data for the X/Open pattern
matching routines:

	HARNESS			HEADER		FUNCTIONS
	-------			------		---------
	testfnmatch.c		<fnmatch.h>	fnmatch()
	testglob.c		<glob.h>	glob()
	testregex.c		<regex.h>	regcomp(),regexec()

Each test*.c file is a main program driven by the *.dat test data files.
testglob.dat is only for testglob; all other test data files work with
the remaining harnesses.

The Makefile has targets for building harnesses and running tests:

	all		build all harnesses
	test		run all tests for all harnesses
	test.foo	build harness foo and run the foo tests

To run a single test, e.g. the standards.dat tests for testregex,

	testregex < standards.dat

If the local implementation hangs or dumps on some tests then run with
the -c option.

The -h option lists the test data format details. The test data files
exercise all features; the harnesses detect and ignore features not
supported by the local implementation.

Extensions to the standard terminology are derived from the AT&T RE
implementation, unified under <regex.h> with these modes:

	MODE	FLAGS			
	----	-----
	BRE	0			basic RE
	ERE	REG_EXTENDED		egrep RE with perl (...) extensions
	ARE	REG_AUGMENTED		ERE with ! negation, <> word boundaries
	SRE	REG_SHELL		sh patterns
	KRE	REG_SHELL|REG_AUGMENTED	ksh93 patterns: ! @ ( | & ) { }

and some additional flags to handle fnmatch():

	REG_SHELL_ESCAPED	FNM_NOESCAPE
	REG_SHELL_PATH		FNM_PATHNAME
	REG_SHELL_DOT		FNM_PERIOD

The original testregex.c was done by Doug McIlroy at Bell Labs.
The current implementation is maintained by

	Glenn Fowler <gsf@research.att.com>

I'd like to make the regression tests as comprehensive as possible.
Send any new tests to me and I'll roll them into the open source
distribution at http://www.research.att.com/sw/download/ with proper
attribution.

Please note that some regression tests nail down unspecified standard behavior.
These should be noted in the test data with 'u' but currently are not.
Experience with other implementations will help clean this up.
