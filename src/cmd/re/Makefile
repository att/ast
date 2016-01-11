:PACKAGE: ast

LICENSE = since=1995,author=gsf

:ALL:

ed :: ed.c

red :LINK: ed

sed :: sed.h sed0.c sed1.c sed2.c sed3.c LICENSE=since=1995,author=gsf+doug

:: README RELEASE ed.tst sed.tst

TESTREFLAGS = -c

:TEST: ed sed

glob :TEST: testglob.c testglob.dat
	for i in $(*:O>1)
	do	$(*:O=1) $(TESTREFLAGS) < $i
	done

fmt :TEST: testfmt.c testfmt.dat
	$(*:O=1) $(TESTREFLAGS) $(*:O>1)

fnmatch :TEST: testfnmatch.c testfnmatch.dat testregex.dat \
		locale.dat testmatch.dat
	$(*:O=1) $(TESTREFLAGS) $(*:O>1)

match :TEST: testmatch.c testregex.dat locale.dat testmatch.dat
	$(*:O=1) $(TESTREFLAGS) $(*:O>1)

oldmatch :TEST: testoldmatch.c testregex.dat locale.dat testmatch.dat
	$(*:O=1) $(TESTREFLAGS) $(*:O>1)

regex :TEST: testregex.c testregex.dat locale.dat testmatch.dat testsub.dat \
		testdecomp.dat rxposix.dat zero.dat regex++.dat iso8859-1.dat \
		perl.dat minimal.dat escape.dat group.dat haskell.dat \
		nullsubexpr.dat subexpr.dat nested.dat austin.dat xopen.dat \
		pcre-1.dat pcre-2.dat pcre-3.dat pcre-4.dat pcre-5.dat \
		cut.dat libtre.dat reg.dat callout.dat type.dat ksh.dat \
		repetition.dat rightassoc.dat forcedassoc.dat noop.dat \
		conjunction.dat
	$(*:O=1) $(TESTREFLAGS) $(*:O>1)

test.re : test.glob - test.fnmatch - test.match - test.regex

:: testregex.mm basic.dat categorize.dat nullsubexpr-A.dat \
	leftassoc.dat rightassoc.dat forcedassoc.dat unknownassoc.dat \
	repetition.dat re-assoc.mm re-definitions.mm re-nullsubexpr.mm \
	testregex.mm re-categorize.mm re-interpretation.mm re-repetition.mm


testregex.tgz : README.re Makefile.re \
		testfnmatch.c testglob.c testregex.c \
		testfnmatch.dat testglob.dat testmatch.dat testregex.dat \
		locale.dat rxposix.dat
	for f in $(*)
	do	case $f in
		*READ*)	echo "^^^$f^README" ;;
		*Make*)	echo "^^^$f^Makefile" ;;
		*.c)	echo "^^$(PROTO) $(PROTOFLAGS) -dp $f^$f^${f##*/}" ;;
		*.dat)	echo "^^$(PROTO) $(PROTOFLAGS) -dp -c'#' $f^$f^${f##*/}" ;;
		*)	echo "^^^$f^${f##*/}" ;;
		esac
	done |
	$(PAX) -x tgz -wvf $(<) --filter=-
