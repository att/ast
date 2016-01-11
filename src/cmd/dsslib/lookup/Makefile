:PACKAGE: ast

LICENSE = since=2010,author=gsf

CCFLAGS = $(CC.OPTIMIZE) $(CC.DLL)

lookup plugin=dss :LIBRARY: lookup.c lib.c -ldss

:TEST: dss

:: RELEASE \
	data/delete.dat data/test.json
