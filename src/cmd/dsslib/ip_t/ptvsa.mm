ipv6 address longest prefix match using an interval dictionary
use this until the lpm(1) retrie implementation handles ipv6 addresses

	Glenn Fowler
	Network Services Research
	gsf@research.att.com

This directory contains the standalone code for ipv6 address longest
prefix match using an interval dictionary. The code is currently AT&T
PROPRIETARY, so don't propagate this outside of AT&T.  ptvsa requires
the ast standalone astsa source package.  Read this package and astsa
in the same directory and run this to build libptvsa.a:

	make -f ptvsa.omk

and this to build the test harness and test the library:

	make -f ptvsa.omk test

You may have to do some /bin/make plumbing on *.omk to get it to work
on your system.

The library interface is implemented in libptvsa.a; include ptv.h in your
source and link your a.out with libptvsa.a.  ptvsa.omk (for old make) pulls
in standalone headers and source.

'testptv --man' lists the test harness man page on the standard error.
ptv.3 is the api man page. See testptv.c for example api usage.

ptvsa is a subset of the { ast-base ast-dss } packages at

	http://www.research.att.com/sw/download/
