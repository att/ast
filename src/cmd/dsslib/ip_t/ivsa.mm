.xx title="astsa"
.MT 4
.TL

.H 1 "ivsa"
.B ivsa
is a standalone library for ipv6 address longest prefix match using an
interval dictionary.
Use this until the
.BR lpm (1)
.B retrie
implementation handles ipv6 addresses.
.B ivsa
requires the standalone
.B astsa
source package. 
.P
To build libivsa.a read this package and
.B astsa
in the same directory, read README-ast, and then run:
.EX
 make -f ivsa.omk
.EE
to build the test harness and test the library run:
.EX
 make -f ivsa.omk test
.EE
You may have to do some /bin/make plumbing on *.omk to get it to work
on your system.
If you already have the non-standalone libast installed
then you should be able to compile and link just the standalone ivsa
against it.
.P
The library interface is implemented in libivsa.a; include iv.h in your
source and link your a.out with libivsa.a.  ivsa.omk (for old make) pulls
in standalone headers and source.
.L 'testiv --man'
lists the test harness man page on the standard error.
iv.3 is the api man page.  See testiv.c for example api usage.
.P
.B ivsa
is a subset of the {
.B "ast-base ast-dss"
} packages at
.EX
.xx link="http://www.research.att.com/sw/download/"
.EE
