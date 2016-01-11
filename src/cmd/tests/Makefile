/*
 * regression tests for library functions
 * `tests' runs all tests visible from .. with output in tests.out
 */

:PACKAGE: ast

LICENSE = since=1999,author=gsf

TESTLIBFLAGS = --proc=8 --thread=8 --timeout=1

:TEST: astconf ip6 opt strelapsed strtoi strtof

aso :TESTLIB: ast -ltaso \
	taso.c tlock.c tproc.c tthread.c

base64 :TEST: base64.c

cdt :TESTLIB: ast dttest.h -ltaso -ldll \
	tbags.c tdeque.c tevent.c tlist.c tobag.c tqueue.c trehash.c \
	trhbags.c tsafehash.c tsafetree.c tsearch.c tshare.c tstack.c \
	tstringset.c tview.c \
	tvsafehash.c tvsaferehash.c tvsafetree.c tvsharemem.c tvthread.c

date :TEST: date.c date.msk date.dat datey1k.dat datey2k.dat week.dat zone.dat
	for i in $(*:N=*.dat)
	do	DATEMSK=$(*:N=*.msk:/ /:/G) TZ=EST5EDT $(*:O=1) $(TESTDATEFLAGS) < $i
	done

sfio :TESTLIB: ast sftest.h -ltaso \
	talarm.c talign.c tappend.c tatexit.c tbadargs.c tclose.c \
	tdisc.c terrno.c texcept.c tflags.c tfmt.c tgetr.c thole.c \
	tleak.c tlocale.c tlongdouble.c tmmap2read.c tmode.c tmove.c \
	tmprdwr.c tmpread.c tmprocess.c tmtsafe.c tmultiple.c tmwrite.c \
	tnoseek.c tnotify.c topen.c tpipe.c tpipemove.c tpkrd.c tpoll.c \
	tpool.c tpopen.c tpopenrw.c tprintf.c tpublic.c tputgetc.c \
	tputgetd.c tputgetl.c tputgetm.c tputgetr.c tputgetu.c trcrv.c \
	treserve.c tresize.c tscanf.c tscanf1.c tseek.c tsetbuf.c \
	tsetfd.c tsfstr.c tshare.c tsize.c tstack.c tstatus.c tstdio.c \
	tstkpk.c tstring.c tswap.c tsync.c ttell.c ttmp.c ttmpfile.c \
	tungetc.c twalk.c twc.c twchar.c twhole.c twrrd.c

vmalloc :TESTLIB: ast vmtest.h -ltaso \
	talign.c tcompact.c tek.c tlast.c tmalloc.c tmmopen.c tpool.c \
	trandom.c tregion.c tresize.c tsafemalloc.c tsharemem.c \
	tsmall.c tstat.c twalk.c

:: RELEASE NOTES testdate.sh y2k.dat \
	strtof-6.37.38-15.307.308-15.307.308.rt   strtof-6.37.38-15.307.308-15.307.308.tst \
	strtof-6.37.38-15.307.308-18.4931.4932.rt strtof-6.37.38-15.307.308-18.4931.4932.tst \
	strtof-6.37.38-15.307.308-31.291.308.rt   strtof-6.37.38-15.307.308-31.291.308.tst \
	strtof-6.37.38-15.307.308-31.307.308.rt   strtof-6.37.38-15.307.308-31.307.308.tst \
	strtof-6.37.38-15.307.308-33.4931.4932.rt strtof-6.37.38-15.307.308-33.4931.4932.tst

install :
	: only tests in this directory : nothing to install :
