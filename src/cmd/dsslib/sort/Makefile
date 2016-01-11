:PACKAGE: ast

LICENSE = since=2011,author=gsf

CCFLAGS = $(CC.OPTIMIZE) $(CC.DLL)

sort plugin=dss :LIBRARY: sort.c lib.c -lrecsort -ldss

:TEST: dss

:: RELEASE \
	data/pwd.dat data/pwd.dss
