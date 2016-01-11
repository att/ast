:PACKAGE: ast

LICENSE = zlib

PROTOFLAGS = -s

bzip2 :: bzip2.1 bzip2.c -lbz

bzip :LINK: bzip2

bunzip :LINK: bzip2

bunzip2 :LINK: bzip2

bzcat :LINK: bzip2

:INSTALLDIR: bzip2recover

bzip2recover :: bzip2recover.c

:TEST: bzip2

:: README RELEASE LICENSE LICENSE.txt CHANGES \
	manual.texi manual.ps manual_toc.html \
	manual_1.html manual_2.html manual_3.html manual_4.html \
	data/sample1.ref data/sample2.ref data/sample1.bz2 data/sample2.bz2
